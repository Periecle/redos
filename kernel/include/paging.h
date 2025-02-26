#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 39)

/* Page size (4KB) */
#define PAGE_SIZE 4096

/* Page table/directory entry flags */
#define PAGE_PRESENT   0x001
#define PAGE_WRITE     0x002
#define PAGE_USER      0x004
#define PAGE_ACCESSED  0x020
#define PAGE_DIRTY     0x040
#define PAGE_FRAME     0xFFFFFFFFFFFFF000

/* Page Directory and Page Table typedefs for 64-bit paging */
typedef uint64_t page_map_level4_t[512];
typedef uint64_t page_directory_ptr_t[512];
typedef uint64_t page_directory_t[512];
typedef uint64_t page_table_t[512];

/* Paging functions */
void init_paging(void);
void* kmalloc_physical_page(void);
void kfree_physical_page(void* addr);
void map_page_to_frame(void* virtual_addr, void* physical_addr, uint64_t flags);
void unmap_page(void* virtual_addr);
void* get_physical_address(void* virtual_addr);
void switch_page_directory(page_map_level4_t *pml4);
void flush_tlb_entry(uint64_t addr);
void enable_paging(void);
void disable_paging(void);
bool is_paging_enabled(void);
void print_paging_info(void);

/* Inline functions to convert between virtual and physical addresses */
static inline void* P2V(void* addr) {
    return (void*)((uint64_t)addr + KERNEL_VIRTUAL_BASE);
}

static inline void* V2P(void* addr) {
    return (void*)((uint64_t)addr - KERNEL_VIRTUAL_BASE);
}

#endif /* PAGING_H */
