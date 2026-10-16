#ifndef KFS_PAGE_H
#define KFS_PAGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "klibc/utility/list.h"
#include "arch/i386/mm/mmzone.h"

#define PAGE_SIZE       4096ULL
#define PAGE_SHIFT      12

// Masques de bits d'état du cadre physique (champ flags)
#define PG_RESERVED     (1U << 0) // Page réservée (BIOS, VRAM, code noyau, mem_map)
#define PG_BUDDY        (1U << 1) // Page libre appartenant actuellement au Buddy Allocator
#define PG_ACTIVE       (1U << 2) // Page allouée et en cours d'utilisation
#define PG_DIRTY        (1U << 3) // Données modifiées en mémoire
#define PG_SLAB         (1U << 4) // Page gérée par un allocateur Slab / kmalloc

struct page {
	uint32_t         flags;     // Masque d'état (PG_RESERVED, PG_BUDDY, PG_ACTIVE...)
	uint32_t         _count;    // Compteur de références actives (0 = libre)
	uint8_t          order;     // Ordre actuel du bloc dans le Buddy (0..MAX_ORDER-1)
	uint8_t          zone_id;   // Identifiant de zone (ZONE_DMA, ZONE_NORMAL, ZONE_HIGHMEM)
	struct list_head lru;       // Chaînage intrusif dans zone->free_area[order].free_list
};

extern struct page *mem_map;    // Tableau plat de tous les cadres physiques de la RAM

// Primitives de conversion en temps constant O(1)
#define pfn_to_page(pfn)        (&mem_map[(pfn)])
#define page_to_pfn(page)       ((size_t)((page) - mem_map))

#define pfn_to_phys(pfn)        ((uint64_t)(pfn) << PAGE_SHIFT)
#define phys_to_pfn(phys)       ((size_t)((phys) >> PAGE_SHIFT))

#define page_to_phys(page)      pfn_to_phys(page_to_pfn(page))
#define phys_to_page(phys)      pfn_to_page(phys_to_pfn(phys))

#endif // KFS_PAGE_H