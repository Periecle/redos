#include "paging.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* External references to kernel start/end addresses from linker script */
extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;
extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;

/* The kernel's page directory (defined in boot.S initially) */
page_directory_t *kernel_page_directory;

/* The current page directory */
page_directory_t *current_page_directory;

/* Frame allocation bitmap - manages physical frames */
#define TOTAL_MEMORY_MB 64  /* Assume 64MB of RAM for now */
#define TOTAL_FRAMES (TOTAL_MEMORY_MB * 1024 * 1024 / PAGE_SIZE)
#define BITMAP_SIZE (TOTAL_FRAMES / 32)

static uint32_t frame_bitmap[BITMAP_SIZE];

/* Set a bit in the frame bitmap */
static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frame_bitmap[idx] |= (1 << off);
}

/* Clear a bit in the frame bitmap */
static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frame_bitmap[idx] &= ~(1 << off);
}

/* Test if a bit is set in the frame bitmap */
static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    return (frame_bitmap[idx] & (1 << off));
}

/* Find the first free frame */
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
    printf("ERROR: No free frames available!\n");
    return (uint32_t)-1;
}

/* Allocate a frame */
static uint32_t alloc_frame() {
    uint32_t frame = first_free_frame();
    if (frame == (uint32_t)-1) {
        return 0; /* Out of memory */
    }

    set_frame(frame * PAGE_SIZE);
    return frame * PAGE_SIZE;
}

/* Free a frame */
static void free_frame(uint32_t frame_addr) {
    clear_frame(frame_addr);
}

/* Get a pointer to the page table containing a given virtual address */
static page_table_t* get_page_table(uint32_t virt_addr, bool create) {
    uint32_t pdindex = virt_addr >> 22;
    uint32_t *page_table_addr;

    /* Check if the page directory entry exists */
    if ((*current_page_directory)[pdindex] & PAGE_PRESENT) {
        page_table_addr = (uint32_t*)((*current_page_directory)[pdindex] & PAGE_FRAME);
        return (page_table_t*)P2V(page_table_addr);
    }

    /* If it doesn't exist and we're asked to create it */
    if (create) {
        page_table_addr = (uint32_t*)alloc_frame();
        if (!page_table_addr) {
            printf("ERROR: Failed to allocate page table!\n");
            return NULL;
        }

        /* Clear the new page table */
        memset(P2V(page_table_addr), 0, PAGE_SIZE);

        /* Add the new page table to the page directory */
        (*current_page_directory)[pdindex] = (uint32_t)page_table_addr | PAGE_PRESENT | PAGE_WRITE;

        return (page_table_t*)P2V(page_table_addr);
    }

    return NULL;
}

/* Initialize the paging system */
void init_paging(void) {
    printf("Initializing paging system...\n");

    /* Set up frame allocation bitmap */
    memset(frame_bitmap, 0, sizeof(frame_bitmap));

    /* Mark the first 1MB as used (for BIOS and other low memory structures) */
    for (uint32_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        set_frame(addr);
    }

    /* Mark the kernel's physical memory as used */
    uint32_t kernel_start = (uint32_t)&kernel_physical_start;
    uint32_t kernel_end = (uint32_t)&kernel_physical_end;

    printf("Marking kernel physical memory as used: 0x%x - 0x%x\n", kernel_start, kernel_end);

    for (uint32_t addr = kernel_start; addr < kernel_end; addr += PAGE_SIZE) {
        set_frame(addr);
    }

    /* Get the current page directory (set up in boot.S) */
    uint32_t cr3_value;
    __asm__ __volatile__("movl %%cr3, %0" : "=r"(cr3_value));

    kernel_page_directory = (page_directory_t*)P2V((void*)cr3_value);
    current_page_directory = kernel_page_directory;

    printf("Current page directory at physical: 0x%x, virtual: 0x%x\n",
        cr3_value, (uint32_t)kernel_page_directory);

    /* Set up recursive page directory mapping - allows accessing page tables through virtual memory */
    (*kernel_page_directory)[1023] = (uint32_t)V2P(kernel_page_directory) | PAGE_PRESENT | PAGE_WRITE;

    /* Flush TLB to refresh page tables */
    __asm__ __volatile__("movl %0, %%cr3" : : "r"(cr3_value));

    printf("Paging system initialized!\n");
}

/* Allocate a physical page */
void* kmalloc_physical_page(void) {
    uint32_t physical_addr = alloc_frame();
    if (!physical_addr) {
        return NULL;
    }

    /* Zero the allocated page */
    memset(P2V((void*)physical_addr), 0, PAGE_SIZE);

    return (void*)physical_addr;
}

/* Free a physical page */
void kfree_physical_page(void* addr) {
    free_frame((uint32_t)addr);
}

/* Map a virtual page to a physical frame */
void map_page_to_frame(void* virtual_addr, void* physical_addr, uint32_t flags) {
    uint32_t virt_addr = (uint32_t)virtual_addr;
    uint32_t phys_addr = (uint32_t)physical_addr;

    uint32_t ptindex = (virt_addr >> 12) & 0x3FF;

    page_table_t *table = get_page_table(virt_addr, true);
    if (!table) {
        printf("ERROR: Failed to get page table for 0x%x\n", virt_addr);
        return;
    }

    /* Set the page table entry */
    (*table)[ptindex] = (phys_addr & PAGE_FRAME) | (flags & 0xFFF) | PAGE_PRESENT;

    /* Flush the TLB entry */
    flush_tlb_entry(virt_addr);
}

/* Unmap a virtual page */
void unmap_page(void* virtual_addr) {
    uint32_t virt_addr = (uint32_t)virtual_addr;

    uint32_t ptindex = (virt_addr >> 12) & 0x3FF;

    page_table_t *table = get_page_table(virt_addr, false);
    if (!table) {
        return; /* Page already unmapped */
    }

    /* Clear the page table entry */
    (*table)[ptindex] = 0;

    /* Flush the TLB entry */
    flush_tlb_entry(virt_addr);
}

/* Get the physical address mapped to a virtual address */
void* get_physical_address(void* virtual_addr) {
    uint32_t virt_addr = (uint32_t)virtual_addr;

    uint32_t ptindex = (virt_addr >> 12) & 0x3FF;

    page_table_t *table = get_page_table(virt_addr, false);
    if (!table) {
        return NULL; /* No page table for this address */
    }

    /* Check if the page is present */
    if (!((*table)[ptindex] & PAGE_PRESENT)) {
        return NULL; /* Page not present */
    }

    /* Get the physical frame address and add the offset */
    uint32_t frame_addr = (*table)[ptindex] & PAGE_FRAME;
    uint32_t offset = virt_addr & 0xFFF;

    return (void*)(frame_addr + offset);
}

/* Switch to a different page directory */
void switch_page_directory(page_directory_t *dir) {
    current_page_directory = dir;
    __asm__ __volatile__("movl %0, %%cr3" : : "r"((uint32_t)V2P(dir)));
}

/* Flush a single TLB entry */
void flush_tlb_entry(uint32_t addr) {
    __asm__ __volatile__("invlpg (%0)" : : "r"(addr) : "memory");
}

/* Enable paging */
void enable_paging(void) {
    uint32_t cr0;
    __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; /* Set the paging bit in CR0 */
    __asm__ __volatile__("movl %0, %%cr0" : : "r"(cr0));
}

/* Disable paging */
void disable_paging(void) {
    uint32_t cr0;
    __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000; /* Clear the paging bit in CR0 */
    __asm__ __volatile__("movl %0, %%cr0" : : "r"(cr0));
}

/* Check if paging is enabled */
bool is_paging_enabled(void) {
    uint32_t cr0;
    __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000) != 0; /* Check PG bit */
}

/* Print information about the paging system */
void print_paging_info(void) {
    printf("Paging Information:\n");
    printf("  Paging enabled: %s\n", is_paging_enabled() ? "YES" : "NO");

    uint32_t cr3_value;
    __asm__ __volatile__("movl %%cr3, %0" : "=r"(cr3_value));
    printf("  Page Directory (CR3): 0x%x (Physical)\n", cr3_value);
    printf("  Page Directory Virtual: 0x%x\n", (uint32_t)current_page_directory);

    /* Count used pages */
    uint32_t used_frames = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        for (uint32_t j = 0; j < 32; j++) {
            if (frame_bitmap[i] & (1 << j)) {
                used_frames++;
            }
        }
    }

    printf("  Used physical frames: %d/%d (%d KB)\n",
        used_frames, TOTAL_FRAMES, used_frames * PAGE_SIZE / 1024);
    printf("  Free physical frames: %d/%d (%d KB)\n",
        TOTAL_FRAMES - used_frames, TOTAL_FRAMES,
        (TOTAL_FRAMES - used_frames) * PAGE_SIZE / 1024);
}
