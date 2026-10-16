#ifndef KFS_MMZONE_H
#define KFS_MMZONE_H

#include <stddef.h>
#include <stdint.h>
#include "klibc/utility/list.h"

extern char __kernel_start[];
extern char __kernel_end[];

#define MAX_ORDER 11 // Ordres 0 à 10 inclus (1 page = 4 Ko à 1024 pages = 4 Mo)

// Frontières PFN architecturales sous x86 IA-32 (1 PFN = 4096 octets)
#define ZONE_DMA_MAX_PFN    4096UL   // 16 Mo  (0x01000000 >> 12)
#define ZONE_NORMAL_MAX_PFN 229376UL // 896 Mo (0x38000000 >> 12)

enum zone_type {
	ZONE_DMA = 0,     // 0 à 16 Mo (matériel hérité ISA 24 bits)
	ZONE_NORMAL,      // 16 Mo à 896 Mo (mappage direct higher-half)
	ZONE_HIGHMEM,     // > 896 Mo (mappings dynamiques temporaires via kmap)
	ZONE_MAX,
};

struct free_area {
	struct list_head free_list; // Tête de liste circulaire des blocs libres d'ordre k
	size_t           nr_free;   // Nombre de blocs libres disponibles à cet ordre
};

struct zone {
	const char       *name;                 // Nom lisible ("DMA", "Normal", "HighMem")
	size_t            zone_start_pfn;       // Premier PFN de la zone
	size_t            spanned_pages;        // Étendue totale couverte (end_pfn - start_pfn)
	size_t            present_pages;        // Pages physiques réellement présentes dans la zone
	size_t            free_pages;           // Nombre total de pages libres dans la zone
	struct free_area  free_area[MAX_ORDER]; // Listes d'ordres du Buddy Allocator (0..10)
};

struct page; // Déclaration anticipée (définie dans arch/i386/mm/page.h)

typedef struct pglist_data {
	size_t           node_start_pfn;        // Premier PFN géré par ce nœud (0)
	size_t           node_present_pages;    // Nombre total de cadres physiques utilisables
	size_t           node_spanned_pages;    // Étendue totale de l'espace PFN du nœud
	struct zone      node_zones[ZONE_MAX];  // Les 3 zones physiques rattachées au nœud
	struct page     *node_mem_map;          // Pointeur vers le tableau global mem_map
} pg_data_t;

extern pg_data_t g_contig_node;

#endif // KFS_MMZONE_H