#include "paging.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/debug.h>

extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;
extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;

static page_directory_t *kernel_page_directory;
static page_directory_t *current_page_directory;

#define TOTAL_MEMORY_MB 64
#define TOTAL_FRAMES (TOTAL_MEMORY_MB * 1024 * 1024 / PAGE_SIZE)
#define BITMAP_SIZE (TOTAL_FRAMES / 32)
static uint32_t frame_bitmap[BITMAP_SIZE];

/**
 * Mark a frame as used in the bitmap
 * @param frame_addr Physical address of the frame
 */
static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;

    if (idx >= BITMAP_SIZE) {
        debug_error("set_frame: Frame index %u out of bounds (max %u)", idx, BITMAP_SIZE-1);
        return;
    }

    frame_bitmap[idx] |= (1 << off);
}

/**
 * Mark a frame as free in the bitmap
 * @param frame_addr Physical address of the frame
 */
static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;

    if (idx >= BITMAP_SIZE) {
        debug_error("clear_frame: Frame index %u out of bounds (max %u)", idx, BITMAP_SIZE-1);
        return;
    }

    frame_bitmap[idx] &= ~(1 << off);
}

/**
 * Test if a frame is used
 * @param frame_addr Physical address of the frame
 * @return Non-zero if the frame is used, 0 otherwise
 */
static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;

    if (idx >= BITMAP_SIZE) {
        debug_error("test_frame: Frame index %u out of bounds (max %u)", idx, BITMAP_SIZE-1);
        return 0;
    }

    return (frame_bitmap[idx] & (1 << off));
}

/**
 * Find the first free frame
 * @return Frame number or (uint32_t)-1 if no frames are available
 */
static uint32_t first_free_frame() {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        if (frame_bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t bit = 1 << j;
                if (!(frame_bitmap[i] & bit)) {
                    return i * 32 + j;
                }
            }
        }
    }

    debug_error("No free frames available!");
    return (uint32_t)-1;
}

/**
 * Allocate a physical frame
 * @return Physical address of the allocated frame, or 0 on failure
 */
static uint32_t alloc_frame() {
    uint32_t frame = first_free_frame();
    if (frame == (uint32_t)-1) {
        return 0;
    }

    uint32_t frame_addr = frame * PAGE_SIZE;
    debug_debug("Allocated frame at physical address %x", frame_addr);
    set_frame(frame_addr);
    return frame_addr;
}

/**
 * Free a physical frame
 * @param frame_addr Physical address of the frame to free
 */
static void free_frame(uint32_t frame_addr) {
    debug_debug("Freeing frame at physical address %x", frame_addr);
    clear_frame(frame_addr);
}

/**
 * Get the page table for a virtual address
 * @param virt_addr Virtual address
 * @param create Whether to create the page table if it doesn't exist
 * @return Pointer to the page table, or NULL if it doesn't exist and create is false
 */
static page_table_t* get_page_table(uint32_t virt_addr, bool create) {
    uint32_t pdindex = virt_addr >> 22;
    uint32_t *page_table_addr;

    // Check if the page table already exists
    if ((*current_page_directory)[pdindex] & PAGE_PRESENT) {
        page_table_addr = (uint32_t*)((*current_page_directory)[pdindex] & PAGE_FRAME);
        debug_trace("Using existing page table at physical %x for address %x",
                   (unsigned)page_table_addr, virt_addr);
        return (page_table_t*)P2V(page_table_addr);
    }

    if (create) {
        page_table_addr = (uint32_t*)alloc_frame();
        if (!page_table_addr) {
            debug_error("Failed to allocate page table for address %x", virt_addr);
            return NULL;
        }

        // Clear the new page table
        memset(P2V(page_table_addr), 0, PAGE_SIZE);

        // Add the page table to the page directory
        (*current_page_directory)[pdindex] = (uint32_t)page_table_addr | PAGE_PRESENT | PAGE_WRITE;
        debug_trace("Created new page table at physical %x for address %x",
                   (unsigned)page_table_addr, virt_addr);
        return (page_table_t*)P2V(page_table_addr);
    }

    return NULL;
}

/**
 * Initialize the paging system
 */
void init_paging(void) {
    printf("Initializing paging system...\n");

    // Clear the frame bitmap
    memset(frame_bitmap, 0, sizeof(frame_bitmap));

    // Mark the first 1MB as used (reserved for BIOS, etc.)
    for (uint32_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        set_frame(addr);
    }

    // Mark kernel physical memory as used
    uint32_t kernel_start = (uint32_t)&kernel_physical_start;
    uint32_t kernel_end = (uint32_t)&kernel_physical_end;
    printf("Marking kernel physical memory as used: %x - %x\n", kernel_start, kernel_end);

    for (uint32_t addr = kernel_start; addr < kernel_end; addr += PAGE_SIZE) {
        set_frame(addr);
    }

    // Get the current page directory from CR3
    uint32_t cr3_value;
    __asm__ __volatile__("movl %%cr3, %0" : "=r"(cr3_value));

    // Convert to virtual address
    kernel_page_directory = (page_directory_t*)P2V((void*)cr3_value);
    current_page_directory = kernel_page_directory;

    printf("Current page directory at physical: %x, virtual: %x\n",
        cr3_value, (uint32_t)kernel_page_directory);

    // Set up recursive page directory entry - allows the page directory to map itself
    // at the highest 4MB of virtual memory
    (*kernel_page_directory)[1023] = (uint32_t)V2P(kernel_page_directory) | PAGE_PRESENT | PAGE_WRITE;

    // Update the page directory
    __asm__ __volatile__("movl %0, %%cr3" : : "r"(cr3_value));

    debug_info("Paging system initialized successfully");
    printf("Paging system initialized!\n");
}

/**
 * Allocate a physical page (4KB)
 * @return Physical address of the allocated page, or NULL on failure
 */
void* kmalloc_physical_page(void) {
    uint32_t frame = alloc_frame();
    if (!frame) {
        return NULL;
    }

    void* virt_addr = P2V((void*)frame);
    memset(virt_addr, 0, PAGE_SIZE);
    return (void*)frame;
}

/**
 * Free a physical page
 * @param addr Physical address of the page to free
 */
void kfree_physical_page(void* addr) {
    if (!addr) {
        return;
    }

    free_frame((uint32_t)addr);
}

/**
 * Map a virtual page to a physical frame
 * @param virtual_addr Virtual address to map
 * @param physical_addr Physical address to map to
 * @param flags Page flags (PAGE_PRESENT, PAGE_WRITE, etc.)
 */
void map_page_to_frame(void* virtual_addr, void* physical_addr, uint32_t flags) {
    uint32_t virt_addr = (uint32_t)virtual_addr;
    uint32_t phys_addr = (uint32_t)physical_addr;
    uint32_t ptindex = (virt_addr >> 12) & 0x3FF;

    debug_debug("Mapping virtual %x to physical %x with flags %x",
               virt_addr, phys_addr, flags);

    page_table_t *table = get_page_table(virt_addr, true);
    if (!table) {
        debug_error("Failed to get page table for virtual address %x", virt_addr);
        return;
    }

    (*table)[ptindex] = (phys_addr & PAGE_FRAME) | (flags & 0xFFF) | PAGE_PRESENT;
    flush_tlb_entry(virt_addr);

    debug_trace("Mapped virtual %x to physical %x (PD idx: %u, PT idx: %u)",
               virt_addr, phys_addr, virt_addr >> 22, ptindex);
}

/**
 * Unmap a virtual page
 * @param virtual_addr Virtual address to unmap
 */
void unmap_page(void* virtual_addr) {
    uint32_t virt_addr = (uint32_t)virtual_addr;
    uint32_t ptindex = (virt_addr >> 12) & 0x3FF;

    debug_debug("Unmapping virtual address %x", virt_addr);

    page_table_t *table = get_page_table(virt_addr, false);
    if (!table) {
        debug_warning("Page table not found for virtual address %x", virt_addr);
        return;
    }

    // Clear the page table entry
    (*table)[ptindex] = 0;
    flush_tlb_entry(virt_addr);

    debug_trace("Unmapped virtual address %x (PD idx: %u, PT idx: %u)",
               virt_addr, virt_addr >> 22, ptindex);
}

/**
 * Get the physical address for a virtual address
 * @param virtual_addr Virtual address
 * @return Physical address or NULL if not mapped
 */
void* get_physical_address(void* virtual_addr) {
    uint32_t virt_addr = (uint32_t)virtual_addr;
    uint32_t ptindex = (virt_addr >> 12) & 0x3FF;

    page_table_t *table = get_page_table(virt_addr, false);
    if (!table) {
        return NULL;
    }

    if (!((*table)[ptindex] & PAGE_PRESENT)) {
        return NULL;
    }

    uint32_t frame_addr = (*table)[ptindex] & PAGE_FRAME;
    uint32_t offset = virt_addr & 0xFFF;
    return (void*)(frame_addr + offset);
}

/**
 * Switch to a different page directory
 * @param dir Pointer to the new page directory
 */
void switch_page_directory(page_directory_t *dir) {
    current_page_directory = dir;
    __asm__ __volatile__("movl %0, %%cr3" : : "r"((uint32_t)V2P(dir)));
    debug_debug("Switched to page directory at virtual %x, physical %x",
               (unsigned)dir, (unsigned)V2P(dir));
}

/**
 * Flush a single TLB entry
 * @param addr Virtual address to flush
 */
void flush_tlb_entry(uint32_t addr) {
    __asm__ __volatile__("invlpg (%0)" : : "r"(addr) : "memory");
}

/**
 * Enable paging
 */
void enable_paging(void) {
    uint32_t cr0;
    __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ __volatile__("movl %0, %%cr0" : : "r"(cr0));
    debug_info("Paging enabled");
}

/**
 * Disable paging
 */
void disable_paging(void) {
    uint32_t cr0;
    __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000;
    __asm__ __volatile__("movl %0, %%cr0" : : "r"(cr0));
    debug_info("Paging disabled");
}

/**
 * Check if paging is enabled
 * @return true if paging is enabled, false otherwise
 */
bool is_paging_enabled(void) {
    uint32_t cr0;
    __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000) != 0;
}

/**
 * Print information about the paging system
 */
void print_paging_info(void) {
    printf("Paging Information:\n");
    printf("  Paging enabled: %s\n", is_paging_enabled() ? "YES" : "NO");

    uint32_t cr3_value;
    __asm__ __volatile__("movl %%cr3, %0" : "=r"(cr3_value));
    printf("  Page Directory (CR3): %x (Physical)\n", cr3_value);
    printf("  Page Directory Virtual: %x\n", (uint32_t)current_page_directory);

    uint32_t used_frames = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        for (uint32_t j = 0; j < 32; j++) {
            if (frame_bitmap[i] & (1 << j)) {
                used_frames++;
            }
        }
    }

    printf("  Used physical frames: %u/%u (%u KB)\n",
        used_frames, TOTAL_FRAMES, used_frames * PAGE_SIZE / 1024);
    printf("  Free physical frames: %u/%u (%u KB)\n",
        TOTAL_FRAMES - used_frames, TOTAL_FRAMES,
        (TOTAL_FRAMES - used_frames) * PAGE_SIZE / 1024);

    debug_trace("Page directory at physical %x, virtual %x",
               cr3_value, (unsigned)current_page_directory);
}
