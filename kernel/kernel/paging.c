#include "paging.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/debug.h>

extern uint64_t kernel_physical_start;
extern uint64_t kernel_physical_end;
extern uint64_t kernel_virtual_start;
extern uint64_t kernel_virtual_end;

static page_map_level4_t *kernel_pml4;
static page_map_level4_t *current_pml4;

#define TOTAL_MEMORY_MB 64
#define TOTAL_FRAMES (TOTAL_MEMORY_MB * 1024 * 1024 / PAGE_SIZE)
#define BITMAP_SIZE (TOTAL_FRAMES / 64)
static uint64_t frame_bitmap[BITMAP_SIZE];

/**
 * Mark a frame as used in the bitmap
 * @param frame_addr Physical address of the frame
 */
static void set_frame(uint64_t frame_addr) {
    uint64_t frame = frame_addr / PAGE_SIZE;
    uint64_t idx = frame / 64;
    uint64_t off = frame % 64;

    if (idx >= BITMAP_SIZE) {
        debug_error("set_frame: Frame index %lu out of bounds (max %lu)", idx, BITMAP_SIZE-1);
        return;
    }

    frame_bitmap[idx] |= (1ULL << off);
}

/**
 * Mark a frame as free in the bitmap
 * @param frame_addr Physical address of the frame
 */
static void clear_frame(uint64_t frame_addr) {
    uint64_t frame = frame_addr / PAGE_SIZE;
    uint64_t idx = frame / 64;
    uint64_t off = frame % 64;

    if (idx >= BITMAP_SIZE) {
        debug_error("clear_frame: Frame index %lu out of bounds (max %lu)", idx, BITMAP_SIZE-1);
        return;
    }

    frame_bitmap[idx] &= ~(1ULL << off);
}

/**
 * Test if a frame is used
 * @param frame_addr Physical address of the frame
 * @return Non-zero if the frame is used, 0 otherwise
 */
static uint64_t test_frame(uint64_t frame_addr) {
    uint64_t frame = frame_addr / PAGE_SIZE;
    uint64_t idx = frame / 64;
    uint64_t off = frame % 64;

    if (idx >= BITMAP_SIZE) {
        debug_error("test_frame: Frame index %lu out of bounds (max %lu)", idx, BITMAP_SIZE-1);
        return 0;
    }

    return (frame_bitmap[idx] & (1ULL << off));
}

/**
 * Find the first free frame
 * @return Frame number or (uint64_t)-1 if no frames are available
 */
static uint64_t first_free_frame() {
    for (uint64_t i = 0; i < BITMAP_SIZE; i++) {
        if (frame_bitmap[i] != 0xFFFFFFFFFFFFFFFF) {
            for (uint64_t j = 0; j < 64; j++) {
                uint64_t bit = 1ULL << j;
                if (!(frame_bitmap[i] & bit)) {
                    return i * 64 + j;
                }
            }
        }
    }

    debug_error("No free frames available!");
    return (uint64_t)-1;
}

/**
 * Allocate a physical frame
 * @return Physical address of the allocated frame, or 0 on failure
 */
static uint64_t alloc_frame() {
    uint64_t frame = first_free_frame();
    if (frame == (uint64_t)-1) {
        return 0;
    }

    uint64_t frame_addr = frame * PAGE_SIZE;
    debug_debug("Allocated frame at physical address %lx", frame_addr);
    set_frame(frame_addr);
    return frame_addr;
}

/**
 * Free a physical frame
 * @param frame_addr Physical address of the frame to free
 */
static void free_frame(uint64_t frame_addr) {
    debug_debug("Freeing frame at physical address %lx", frame_addr);
    clear_frame(frame_addr);
}

/**
 * Get or create page table entry at level
 * @param dir Pointer to page structure at current level
 * @param idx Index in current level
 * @param level Current paging level (0=PML4, 1=PDPT, 2=PD, 3=PT)
 * @param create Whether to create the structure if it doesn't exist
 * @return Pointer to the next level structure, or NULL if it doesn't exist and create is false
 */
static void* get_or_create_page_entry(void* dir, uint64_t idx, int level, bool create) {
    uint64_t* table = (uint64_t*)dir;

    // Check if entry exists
    if (table[idx] & PAGE_PRESENT) {
        // Entry exists, extract physical address (mask out flags)
        uint64_t phys_addr = table[idx] & PAGE_FRAME;
        return P2V((void*)phys_addr);
    } else if (create) {
        // Need to create a new page structure
        uint64_t phys_addr = alloc_frame();
        if (!phys_addr) {
            debug_error("Failed to allocate frame for page structure at level %d", level);
            return NULL;
        }

        // Clear the new page structure
        memset(P2V((void*)phys_addr), 0, PAGE_SIZE);

        // Add the entry to the current level with proper flags
        table[idx] = phys_addr | PAGE_PRESENT | PAGE_WRITE;

        return P2V((void*)phys_addr);
    }

    return NULL;
}

/**
 * Initialize the paging system
 */
void init_paging(void) {
    printf("Initializing 64-bit paging system...\n");

    // Clear the frame bitmap
    memset(frame_bitmap, 0, sizeof(frame_bitmap));

    // Mark the first 1MB as used (reserved for BIOS, etc.)
    for (uint64_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        set_frame(addr);
    }

    // Mark kernel physical memory as used
    uint64_t kernel_start = (uint64_t)&kernel_physical_start;
    uint64_t kernel_end = (uint64_t)&kernel_physical_end;
    printf("Marking kernel physical memory as used: %lx - %lx\n", kernel_start, kernel_end);

    for (uint64_t addr = kernel_start; addr < kernel_end; addr += PAGE_SIZE) {
        set_frame(addr);
    }

    // Get the current page directory from CR3
    uint64_t cr3_value;
    __asm__ __volatile__("movq %%cr3, %0" : "=r"(cr3_value));

    // Convert to virtual address
    kernel_pml4 = (page_map_level4_t*)P2V((void*)cr3_value);
    current_pml4 = kernel_pml4;

    printf("Current PML4 at physical: %lx, virtual: %lx\n",
        cr3_value, (uint64_t)kernel_pml4);

    // Set up recursive mapping in the last entry of PML4
    // This allows accessing page tables through a fixed virtual address
    (*kernel_pml4)[511] = (uint64_t)V2P(kernel_pml4) | PAGE_PRESENT | PAGE_WRITE;

    // Update the page directory
    __asm__ __volatile__("movq %0, %%cr3" : : "r"(cr3_value));

    debug_info("64-bit paging system initialized successfully");
    printf("64-bit paging system initialized!\n");
}

/**
 * Allocate a physical page (4KB)
 * @return Physical address of the allocated page, or NULL on failure
 */
void* kmalloc_physical_page(void) {
    uint64_t frame = alloc_frame();
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

    free_frame((uint64_t)addr);
}

/**
 * Map a virtual page to a physical frame
 * @param virtual_addr Virtual address to map
 * @param physical_addr Physical address to map to
 * @param flags Page flags (PAGE_PRESENT, PAGE_WRITE, etc.)
 */
void map_page_to_frame(void* virtual_addr, void* physical_addr, uint64_t flags) {
    uint64_t virt_addr = (uint64_t)virtual_addr;
    uint64_t phys_addr = (uint64_t)physical_addr;

    // Extract page table indices
    uint64_t pml4_idx = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_idx = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_idx = (virt_addr >> 12) & 0x1FF;

    debug_debug("Mapping virtual %lx to physical %lx with flags %lx (PML4:%lu PDPT:%lu PD:%lu PT:%lu)",
               virt_addr, phys_addr, flags, pml4_idx, pdpt_idx, pd_idx, pt_idx);

    // Navigate or create page tables
    page_directory_ptr_t* pdpt = get_or_create_page_entry(current_pml4, pml4_idx, 0, true);
    if (!pdpt) return;

    page_directory_t* pd = get_or_create_page_entry(pdpt, pdpt_idx, 1, true);
    if (!pd) return;

    page_table_t* pt = get_or_create_page_entry(pd, pd_idx, 2, true);
    if (!pt) return;

    // Set the page table entry with physical address and flags
    (*pt)[pt_idx] = (phys_addr & PAGE_FRAME) | (flags & 0xFFF) | PAGE_PRESENT;

    // Flush the TLB entry
    flush_tlb_entry(virt_addr);

    debug_trace("Mapped virtual %lx to physical %lx (PML4:%lu PDPT:%lu PD:%lu PT:%lu)",
               virt_addr, phys_addr, pml4_idx, pdpt_idx, pd_idx, pt_idx);
}

/**
 * Unmap a virtual page
 * @param virtual_addr Virtual address to unmap
 */
void unmap_page(void* virtual_addr) {
    uint64_t virt_addr = (uint64_t)virtual_addr;

    // Extract page table indices
    uint64_t pml4_idx = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_idx = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_idx = (virt_addr >> 12) & 0x1FF;

    debug_debug("Unmapping virtual address %lx (PML4:%lu PDPT:%lu PD:%lu PT:%lu)",
               virt_addr, pml4_idx, pdpt_idx, pd_idx, pt_idx);

    // Navigate page tables (don't create if not exist)
    page_directory_ptr_t* pdpt = get_or_create_page_entry(current_pml4, pml4_idx, 0, false);
    if (!pdpt) return;

    page_directory_t* pd = get_or_create_page_entry(pdpt, pdpt_idx, 1, false);
    if (!pd) return;

    page_table_t* pt = get_or_create_page_entry(pd, pd_idx, 2, false);
    if (!pt) return;

    // Clear the page table entry
    (*pt)[pt_idx] = 0;

    // Flush the TLB entry
    flush_tlb_entry(virt_addr);

    debug_trace("Unmapped virtual address %lx (PML4:%lu PDPT:%lu PD:%lu PT:%lu)",
               virt_addr, pml4_idx, pdpt_idx, pd_idx, pt_idx);
}

/**
 * Get the physical address for a virtual address
 * @param virtual_addr Virtual address
 * @return Physical address or NULL if not mapped
 */
void* get_physical_address(void* virtual_addr) {
    uint64_t virt_addr = (uint64_t)virtual_addr;

    // Extract page table indices
    uint64_t pml4_idx = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_idx = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_idx = (virt_addr >> 12) & 0x1FF;

    // Navigate page tables (don't create if not exist)
    page_directory_ptr_t* pdpt = get_or_create_page_entry(current_pml4, pml4_idx, 0, false);
    if (!pdpt) return NULL;

    page_directory_t* pd = get_or_create_page_entry(pdpt, pdpt_idx, 1, false);
    if (!pd) return NULL;

    page_table_t* pt = get_or_create_page_entry(pd, pd_idx, 2, false);
    if (!pt) return NULL;

    if (!((*pt)[pt_idx] & PAGE_PRESENT)) {
        return NULL;
    }

    uint64_t frame_addr = (*pt)[pt_idx] & PAGE_FRAME;
    uint64_t offset = virt_addr & 0xFFF;
    return (void*)(frame_addr + offset);
}

/**
 * Switch to a different page directory
 * @param pml4 Pointer to the new PML4 table
 */
void switch_page_directory(page_map_level4_t *pml4) {
    current_pml4 = pml4;
    __asm__ __volatile__("movq %0, %%cr3" : : "r"((uint64_t)V2P(pml4)));
    debug_debug("Switched to PML4 at virtual %lx, physical %lx",
               (uint64_t)pml4, (uint64_t)V2P(pml4));
}

/**
 * Flush a single TLB entry
 * @param addr Virtual address to flush
 */
void flush_tlb_entry(uint64_t addr) {
    __asm__ __volatile__("invlpg (%0)" : : "r"(addr) : "memory");
}

/**
 * Enable paging
 */
void enable_paging(void) {
    uint64_t cr0;
    __asm__ __volatile__("movq %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ __volatile__("movq %0, %%cr0" : : "r"(cr0));
    debug_info("Paging enabled");
}

/**
 * Disable paging
 */
void disable_paging(void) {
    uint64_t cr0;
    __asm__ __volatile__("movq %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000;
    __asm__ __volatile__("movq %0, %%cr0" : : "r"(cr0));
    debug_info("Paging disabled");
}

/**
 * Check if paging is enabled
 * @return true if paging is enabled, false otherwise
 */
bool is_paging_enabled(void) {
    uint64_t cr0;
    __asm__ __volatile__("movq %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000) != 0;
}

/**
 * Print information about the paging system
 */
void print_paging_info(void) {
    printf("64-bit Paging Information:\n");
    printf("  Paging enabled: %s\n", is_paging_enabled() ? "YES" : "NO");

    uint64_t cr3_value;
    __asm__ __volatile__("movq %%cr3, %0" : "=r"(cr3_value));
    printf("  PML4 (CR3): %lx (Physical)\n", cr3_value);
    printf("  PML4 Virtual: %lx\n", (uint64_t)current_pml4);

    uint64_t used_frames = 0;
    for (uint64_t i = 0; i < BITMAP_SIZE; i++) {
        for (uint64_t j = 0; j < 64; j++) {
            if (frame_bitmap[i] & (1ULL << j)) {
                used_frames++;
            }
        }
    }

    printf("  Used physical frames: %lu/%lu (%lu KB)\n",
        used_frames, TOTAL_FRAMES, used_frames * PAGE_SIZE / 1024);
    printf("  Free physical frames: %lu/%lu (%lu KB)\n",
        TOTAL_FRAMES - used_frames, TOTAL_FRAMES,
        (TOTAL_FRAMES - used_frames) * PAGE_SIZE / 1024);

    debug_trace("PML4 at physical %lx, virtual %lx",
               cr3_value, (uint64_t)current_pml4);
}
