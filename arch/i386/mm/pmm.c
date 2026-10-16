#include "arch/i386/mm/pmm.h"
#include "klibc/memory/mem.h"
#include "klibc/utility/printf.h"

static void init_single_zone(struct zone *zone, const char *name, size_t start_pfn, size_t end_pfn) {
	zone->name = name;
	zone->zone_start_pfn = start_pfn;
	zone->spanned_pages = (end_pfn > start_pfn) ? (end_pfn - start_pfn) : 0;
	zone->present_pages = 0; // Incrémenté lors du parcours des segments RAM disponibles
	zone->free_pages = 0;

	for (int o = 0; o < MAX_ORDER; o++) {
		init_list_head(&zone->free_area[o].free_list);
		zone->free_area[o].nr_free = 0;
	}
}

void init_pmm(multiboot_info_t *mbi) {
	if (!(mbi->flags & MULTIBOOT_INFO_MMAP)) {
		printf("[PMM ERROR] Multiboot memory map absente !\n");
		return;
	}

	// 1. Détection de la RAM via mminfo_ram.c
	memory_detection_info_t mem_info = detect_memory(mbi);
	size_t total_pfn = (size_t)(mem_info.max_physical_addr >> PAGE_SHIFT);
	size_t present_pfn = (size_t)(mem_info.total_usable_ram >> PAGE_SHIFT);

	// 2. Allocation du tableau mem_map juste après __kernel_end (Bump allocation)
	size_t mem_map_bytes = total_pfn * sizeof(struct page);
	mem_map = (struct page *)(uintptr_t)__kernel_end;
	kmemset(mem_map, 0, mem_map_bytes);

	uintptr_t mem_map_end = (uintptr_t)mem_map + mem_map_bytes;
	size_t reserved_pfn_limit = (mem_map_end + PAGE_SIZE - 1) >> PAGE_SHIFT;

	// 3. Initialisation du Nœud Global UMA (g_contig_node)
	g_contig_node.node_start_pfn     = 0;
	g_contig_node.node_present_pages = present_pfn;
	g_contig_node.node_spanned_pages = total_pfn;
	g_contig_node.node_mem_map       = mem_map;

	// 4. Découpage des 3 zones physiques du nœud UMA
	size_t dma_end_pfn     = (total_pfn < ZONE_DMA_MAX_PFN) ? total_pfn : ZONE_DMA_MAX_PFN;
	size_t normal_end_pfn  = (total_pfn < ZONE_NORMAL_MAX_PFN) ? total_pfn : ZONE_NORMAL_MAX_PFN;
	size_t highmem_end_pfn = total_pfn;

	init_single_zone(&g_contig_node.node_zones[ZONE_DMA], "DMA", 0, dma_end_pfn);
	init_single_zone(&g_contig_node.node_zones[ZONE_NORMAL], "Normal", dma_end_pfn, normal_end_pfn);
	if (total_pfn > ZONE_NORMAL_MAX_PFN) {
		init_single_zone(&g_contig_node.node_zones[ZONE_HIGHMEM], "HighMem", normal_end_pfn, highmem_end_pfn);
	} else {
		init_single_zone(&g_contig_node.node_zones[ZONE_HIGHMEM], "HighMem", 0, 0);
	}

	// 5. Marquer initialement tous les cadres comme PG_RESERVED et leur rattacher leur zone_id
	for (size_t p = 0; p < total_pfn; p++) {
		mem_map[p].flags = PG_RESERVED;
		mem_map[p]._count = 1;
		mem_map[p].order = 0;
		init_list_head(&mem_map[p].lru);

		if (p < dma_end_pfn)
			mem_map[p].zone_id = ZONE_DMA;
		else if (p < normal_end_pfn)
			mem_map[p].zone_id = ZONE_NORMAL;
		else
			mem_map[p].zone_id = ZONE_HIGHMEM;
	}

	// 6. Libération des pages disponibles dans le Buddy Allocator
	multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *)(uintptr_t)mbi->mmap_addr;
	uintptr_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

	while ((uintptr_t)mmap < mmap_end) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			size_t start_pfn = (size_t)(mmap->addr >> PAGE_SHIFT);
			size_t end_pfn   = (size_t)((mmap->addr + mmap->len) >> PAGE_SHIFT);

			for (size_t pfn = start_pfn; pfn < end_pfn && pfn < total_pfn; pfn++) {
				uint8_t ztype = mem_map[pfn].zone_id;
				g_contig_node.node_zones[ztype].present_pages++;

				// Protéger le premier Mo (PFN < 256 : BIOS/VGA) et l'image noyau + mem_map
				if (pfn >= 256 && pfn >= reserved_pfn_limit) {
					mem_map[pfn].flags = 0;
					mem_map[pfn]._count = 0;
					// free_pages() insère la page dans sa zone et fusionne les blocs adjacents
					free_pages(&mem_map[pfn], 0);
				}
			}
		}
		mmap = (multiboot_mmap_entry_t *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
	}

	printf("[PMM] Noeud UMA initialise : %d PFNs geres (mem_map: %d Ko a 0x%p)\n",
	       (int)total_pfn, (int)(mem_map_bytes / 1024), (void *)mem_map);
}
