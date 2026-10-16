#ifndef KFS_VMM_H
#define KFS_VMM_H

#include "arch/i386/mm/page.h"

#define PAGE_OFFSET    0xC0000000UL // Base virtuelle du noyau (Higher-Half)
#define KMAP_VIRT_BASE 0xFE000000UL // Fenêtre virtuelle kmap (4 Mo pour ZONE_HIGHMEM)

static inline void *page_to_virt(const struct page *page) {
	uint64_t phys = page_to_phys(page);
	return (void *)(uintptr_t)(phys + PAGE_OFFSET);
}

static inline struct page *virt_to_page(const void *kaddr) {
	uintptr_t phys = (uintptr_t)kaddr - PAGE_OFFSET;
	return phys_to_page(phys);
}

void  init_vmm(void);
void *kmap(struct page *page);
void  kunmap(struct page *page);
void  vmm_map_page(uintptr_t virt, uint64_t phys, uint32_t flags);
void  vmm_unmap_page(uintptr_t virt);

#endif // KFS_VMM_H