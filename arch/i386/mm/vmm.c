#include "arch/i386/mm/vmm.h"
#include "klibc/memory/mem.h"

static uint32_t g_page_directory[1024] __attribute__((aligned(4096)));
static uint32_t g_identity_pt[1024]    __attribute__((aligned(4096)));
static uint32_t g_kernel_pt[1024]      __attribute__((aligned(4096)));
static uint32_t g_kmap_pt[1024]        __attribute__((aligned(4096)));

void vmm_map_page(uintptr_t virt, uint64_t phys, uint32_t flags) {
	uint32_t pde_idx = virt >> 22;
	uint32_t pte_idx = (virt >> 12) & 0x3FF;

	if (!(g_page_directory[pde_idx] & 0x01)) {
		return;
	}

	uint32_t *pt = (uint32_t *)(g_page_directory[pde_idx] & ~0xFFF);
	pt[pte_idx] = ((uint32_t)phys & ~0xFFF) | (flags & 0xFFF) | 0x01;
	__asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

void vmm_unmap_page(uintptr_t virt) {
	uint32_t pde_idx = virt >> 22;
	uint32_t pte_idx = (virt >> 12) & 0x3FF;

	if (g_page_directory[pde_idx] & 0x01) {
		uint32_t *pt = (uint32_t *)(g_page_directory[pde_idx] & ~0xFFF);
		pt[pte_idx] = 0;
		__asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
	}
}

void init_vmm(void) {
	kmemset(g_page_directory, 0, sizeof(g_page_directory));

	for (uint32_t i = 0; i < 1024; i++) {
		g_identity_pt[i] = (i * 4096) | 0x03; // Present, R/W
	}
	g_page_directory[0] = ((uint32_t)g_identity_pt) | 0x03;

	for (uint32_t i = 0; i < 1024; i++) {
		g_kernel_pt[i] = (i * 4096) | 0x03;
	}
	g_page_directory[0xC0000000 >> 22] = ((uint32_t)g_kernel_pt) | 0x03;

	kmemset(g_kmap_pt, 0, sizeof(g_kmap_pt));
	g_page_directory[KMAP_VIRT_BASE >> 22] = ((uint32_t)g_kmap_pt) | 0x03;

	g_page_directory[1023] = ((uint32_t)g_page_directory) | 0x03;

	__asm__ volatile ("movl %0, %%cr3" :: "r"(g_page_directory));
	uint32_t cr0;
	__asm__ volatile ("movl %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	__asm__ volatile ("movl %0, %%cr0" :: "r"(cr0));
}

void *kmap(struct page *page) {
	if (page->zone_id != ZONE_HIGHMEM) {
		return page_to_virt(page); 
	}

	for (uint32_t i = 0; i < 1024; i++) {
		if (!(g_kmap_pt[i] & 0x01)) {
			g_kmap_pt[i] = (uint32_t)page_to_phys(page) | 0x03;
			uintptr_t vaddr = KMAP_VIRT_BASE + (i * 4096);
			__asm__ volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
			return (void *)vaddr;
		}
	}
	return NULL;
}

void kunmap(struct page *page) {
	if (page->zone_id != ZONE_HIGHMEM)
		return;

	uint32_t phys = (uint32_t)page_to_phys(page);
	for (uint32_t i = 0; i < 1024; i++) {
		if ((g_kmap_pt[i] & ~0xFFF) == phys) {
			g_kmap_pt[i] = 0;
			uintptr_t vaddr = KMAP_VIRT_BASE + (i * 4096);
			__asm__ volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
			break;
		}
	}
}