#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>

/* Define the virtual base address for kernel */
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)

/* Page size (4KB) */
#define PAGE_SIZE 4096

/* Page table/directory entry flags */
#define PAGE_PRESENT   0x001
#define PAGE_WRITE     0x002
#define PAGE_USER      0x004
#define PAGE_ACCESSED  0x020
#define PAGE_DIRTY     0x040
#define PAGE_FRAME     0xFFFFF000

/* Page Directory and Page Table typedefs */
typedef uint32_t page_directory_t[1024];
typedef uint32_t page_table_t[1024];

/* Paging functions */
void init_paging(void);
void* kmalloc_physical_page(void);
void kfree_physical_page(void* addr);
void map_page_to_frame(void* virtual_addr, void* physical_addr, uint32_t flags);
void unmap_page(void* virtual_addr);
void* get_physical_address(void* virtual_addr);
void switch_page_directory(page_directory_t *dir);
void flush_tlb_entry(uint32_t addr);
void enable_paging(void);
void disable_paging(void);
bool is_paging_enabled(void);
void print_paging_info(void);

/* Inline functions to convert between virtual and physical addresses */
static inline void* P2V(void* addr) {
    return (void*)((uint32_t)addr + KERNEL_VIRTUAL_BASE);
}

static inline void* V2P(void* addr) {
    return (void*)((uint32_t)addr - KERNEL_VIRTUAL_BASE);
}

#endif /* PAGING_H */
