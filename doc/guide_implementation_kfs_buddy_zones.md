# 🛠️ Guide d'Implémentation Modulaire KFS : Buddy Allocator, Zones, Nœud UMA & Bootstrap PMM

> **Projet :** Kernel From Scratch (KFS) — Architecture x86 (IA-32 / i386)  
> **Composants :** Buddy Allocator, Modélisation `struct page` / PFN, Architecture par Zones (`ZONE_DMA`, `ZONE_NORMAL`, `ZONE_HIGHMEM`), Nœud Mémoire (`pg_data_t`) & Mapping Virtuel  
> **Conformité stricte :** `-std=gnu23 -Wall -Wextra -Wpedantic -Wshadow -Werror -ffreestanding`  
> **Référence théorique :** [`doc/guide_buddy_zones_nodes_memoire.md`](file:///home/gcaptari/Documents/Kernel-From-Scratch/doc/guide_buddy_zones_nodes_memoire.md)

---

## Table des Matières
1. [Arborescence et Fichiers Cibles (`include/` & `arch/i386/mm/`)](#1-arborescence-et-fichiers-cibles)
2. [Étape 1 : Linker Script (`boot/linker.ld`)](#étape-1--linker-script)
3. [Étape 2 : Liste Chaînée Circulaire Intrusive (`include/klibc/utility/list.h`)](#étape-2--liste-chaînée-circulaire-intrusive)
4. [Étape 3 : Architecture des Zones & du Nœud UMA (`include/arch/i386/mm/mmzone.h`)](#étape-3--architecture-des-zones--du-nœud-uma)
5. [Étape 4 : Descripteurs de Pages & Arithmétique PFN (`include/arch/i386/mm/page.h`)](#étape-4--descripteurs-de-pages--arithmétique-pfn)
6. [Étape 5 : L'Allocateur Compagnon (`include/arch/i386/mm/buddy.h` & `arch/i386/mm/buddy.c`)](#étape-5--lallocateur-compagnon)
7. [Étape 6 : Initialisation PMM, Nœud & Zones (`include/arch/i386/mm/pmm.h` & `arch/i386/mm/pmm.c`)](#étape-6--initialisation-pmm-nœud--zones)
8. [Étape 7 : Mapping Virtuel Direct & Fenêtre `kmap` (`include/arch/i386/mm/vmm.h` & `arch/i386/mm/vmm.c`)](#étape-7--mapping-virtuel-direct--fenêtre-kmap)
9. [Étape 8 : Intégration Makefile (`arch/i386/mm/module.mk`)](#étape-8--intégration-makefile)
10. [Étape 9 : Câblage dans `kernel/kmain.c` & Tests de Validation](#étape-9--câblage-dans-kernelkmainc--tests-de-validation)

---

## 1. Arborescence et Fichiers Cibles

Tous les en-têtes (`.h`) résident dans l'arborescence centralisée `include/` (compilée avec `INCLUDES := -Iinclude`), tandis que les fichiers sources (`.c`) résident dans `arch/i386/mm/` :

```
Kernel-From-Scratch/
├── boot/
│   └── linker.ld                         <-- [EXISTANT] Symboles __kernel_start et __kernel_end
├── include/
│   ├── klibc/
│   │   └── utility/
│   │       └── list.h                    <-- [À CRÉER] Liste circulaire intrusive Linux (list_head)
│   └── arch/i386/mm/
│       ├── mminfo_ram.h                  <-- [EXISTANT] Détection RAM Multiboot (detect_memory)
│       ├── mmzone.h                      <-- [À COMPLÉTER] Zones ZONE_DMA/NORMAL/HIGHMEM & Nœud UMA (pg_data_t)
│       ├── page.h                        <-- [À CRÉER] struct page, PFN macros, conversions O(1)
│       ├── buddy.h                       <-- [À CRÉER] Prototypes alloc_pages() et free_pages()
│       ├── pmm.h                         <-- [À CRÉER] En-tête du gestionnaire de mémoire physique
│       └── vmm.h                         <-- [À CRÉER] Direct mapping & interface kmap
├── arch/i386/
│   ├── module.mk                         <-- [EXISTANT] Inclut arch/i386/mm/module.mk
│   └── mm/
│       ├── module.mk                     <-- [À MODIFIER] Ajout de buddy.c, pmm.c, vmm.c
│       ├── mminfo_ram.c                  <-- [EXISTANT] Calcul de total_usable_ram & max_physical_addr
│       ├── buddy.c                       <-- [À CRÉER] Cœur de l'algorithme Buddy (Scindage & Fusion)
│       ├── pmm.c                         <-- [À CRÉER] Bootstrap du Nœud UMA, des Zones et de mem_map
│       └── vmm.c                         <-- [À CRÉER] Configuration Page Directory et kmap
└── kernel/
    └── kmain.c                           <-- [À MODIFIER] Appel d'initialisation et tests complets
```

---

## Étape 1 : Linker Script

Le sous-système mémoire doit connaître les frontières physiques exactes occupées par le binaire du noyau afin de ne jamais les allouer ni les corrompre, et pour placer le tableau `mem_map` immédiatement après `__kernel_end`.

Vérifiez [`boot/linker.ld`](file:///home/gcaptari/Documents/Kernel-From-Scratch/boot/linker.ld) :

```ld
ENTRY(_start)

SECTIONS
{
	. = 2M;
	__kernel_start = .;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	. =  ALIGN(4K);
	__kernel_end = .;
}
```

---

## Étape 2 : Liste Chaînée Circulaire Intrusive

Chaque zone mémoire maintient 11 listes de blocs libres (`free_area[0..10]`). Pour éviter toute allocation dynamique lors de la manipulation des pages libres, le Buddy Allocator chaîne directement le membre `lru` de chaque `struct page` libre via une liste circulaire doublement chaînée intrusive (`struct list_head`).

Créez le fichier **`include/klibc/utility/list.h`** :

```c
#ifndef KFS_LIST_H
#define KFS_LIST_H

#include <stddef.h>
#include <stdbool.h>

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

static inline void init_list_head(struct list_head *list) {
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list_head *new_node, struct list_head *head) {
	new_node->next = head->next;
	new_node->prev = head;
	head->next->prev = new_node;
	head->next = new_node;
}

static inline void list_add_tail(struct list_head *new_node, struct list_head *head) {
	new_node->next = head;
	new_node->prev = head->prev;
	head->prev->next = new_node;
	head->prev = new_node;
}

static inline void list_del(struct list_head *entry) {
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
	entry->next = NULL;
	entry->prev = NULL;
}

static inline bool list_empty(const struct list_head *head) {
	return head->next == head;
}

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#endif // KFS_LIST_H
```

---

## Étape 3 : Architecture des Zones & du Nœud UMA

### Fonctionnement architectural (Synthèse de [`guide_buddy_zones_nodes_memoire.md`](file:///home/gcaptari/Documents/Kernel-From-Scratch/doc/guide_buddy_zones_nodes_memoire.md))

1. **Le Nœud UMA (`pg_data_t` / `struct pglist_data`) :**
   - Sur architecture PC x86 32 bits classique (**UMA** — *Uniform Memory Access*), toute la RAM physique partage le même bus et la même latence d'accès.
   - Le noyau instancie donc un **unique nœud global** `g_contig_node` qui contient :
     - la plage globale de PFNs (`node_start_pfn`, `node_present_pages`, `node_spanned_pages`),
     - le pointeur vers le tableau global des descripteurs de cadres (`node_mem_map`),
     - le tableau des 3 zones physiques (`node_zones[ZONE_MAX]`).
2. **Les 3 Zones Physiques (`struct zone`) :**
   - **`ZONE_DMA` (`0` à `16 Mo`, PFN `0` à `4096`) :** préservée pour les transferts DMA ISA 24 bits.
   - **`ZONE_NORMAL` (`16 Mo` à `896 Mo`, PFN `4096` à `229376`) :** mappée linéairement en permanence dans le Higher-Half du noyau (`0xC0000000` + adresse physique). Toutes les structures internes du noyau y sont allouées.
   - **`ZONE_HIGHMEM` (`> 896 Mo`, PFN `229376` à `max_pfn`) :** mémoire non mappée en permanence dans l'espace virtuel noyau, accessible à la demande via `kmap()` / `kunmap()`.
3. **Les Arènes d'Ordres (`struct free_area`) :**
   - Chaque zone embarque son propre tableau `free_area[MAX_ORDER]` (`MAX_ORDER = 11`, ordres `0` à `10`).
   - L'entrée `free_area[k]` chaîne tous les blocs libres de taille $2^k$ pages contiguës ($4\text{ Ko} \times 2^k$) appartenant à cette zone.

Complétez [`include/arch/i386/mm/mmzone.h`](file:///home/gcaptari/Documents/Kernel-From-Scratch/include/arch/i386/mm/mmzone.h) :

```c
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
```

---

## Étape 4 : Descripteurs de Pages & Arithmétique PFN

Chaque trame physique de 4 Ko est représentée par une structure `struct page` de **24 octets** stockée dans le tableau global `mem_map` (et référencée par `g_contig_node.node_mem_map`).

Créez le fichier **`include/arch/i386/mm/page.h`** :

```c
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
```

---

## Étape 5 : L'Allocateur Compagnon (`buddy.h` & `buddy.c`)

### 1. En-tête : **`include/arch/i386/mm/buddy.h`**
```c
#ifndef KFS_BUDDY_H
#define KFS_BUDDY_H

#include "arch/i386/mm/page.h"

struct page *alloc_pages(enum zone_type ztype, uint8_t order);
void         free_pages(struct page *page, uint8_t order);
void         print_buddy_stats(void);

#endif // KFS_BUDDY_H
```

### 2. Implémentation : **`arch/i386/mm/buddy.c`**

Notez comment `page_is_buddy()` garantit l'**étanchéité absolue des zones** :
1. Le `buddy_pfn` doit être compris dans `[zone->zone_start_pfn, zone->zone_start_pfn + zone->spanned_pages)`.
2. Le descripteur compagnon doit avoir `page->zone_id == target_zone_id`. Ainsi, un bloc à la frontière de `ZONE_DMA` (PFN 4095) ne fusionnera jamais avec le premier bloc de `ZONE_NORMAL` (PFN 4096).

```c
#include "arch/i386/mm/buddy.h"
#include "klibc/utility/printf.h"

pg_data_t    g_contig_node;
struct page *mem_map = NULL;

// Teste avec sécurité absolue si un compagnon est libre et fusionnable dans la même zone
static inline bool page_is_buddy(const struct zone *zone, const struct page *page,
                                 size_t pfn, uint8_t order, uint8_t target_zone_id) {
	// 1. Vérification stricte des bornes de la zone
	if (pfn < zone->zone_start_pfn || pfn >= (zone->zone_start_pfn + zone->spanned_pages))
		return false;

	// 2. Le cadre doit être marqué libre dans le Buddy
	if (!(page->flags & PG_BUDDY))
		return false;

	// 3. L'ordre doit correspondre exactement
	if (page->order != order)
		return false;

	// 4. La zone doit être identique (interdiction stricte de fusion inter-zones)
	if (page->zone_id != target_zone_id)
		return false;

	return true;
}

struct page *alloc_pages(enum zone_type ztype, uint8_t order) {
	if (order >= MAX_ORDER || ztype >= ZONE_MAX)
		return NULL;

	struct zone *zone = &g_contig_node.node_zones[ztype];

	// 1. Recherche du plus petit ordre disponible >= order dans la zone demandée
	uint8_t current_order;
	for (current_order = order; current_order < MAX_ORDER; current_order++) {
		if (!list_empty(&zone->free_area[current_order].free_list))
			break;
	}

	if (current_order == MAX_ORDER)
		return NULL; // Épuisement de la mémoire à cet ordre dans cette zone

	// 2. Retrait du bloc en tête de sa liste libre
	struct list_head *entry = zone->free_area[current_order].free_list.next;
	list_del(entry);
	zone->free_area[current_order].nr_free--;

	struct page *page = list_entry(entry, struct page, lru);

	// 3. Scindage récursif (Splitting) vers le bas si current_order > order
	while (current_order > order) {
		current_order--;

		size_t buddy_pfn = page_to_pfn(page) + (1UL << current_order);
		struct page *buddy = pfn_to_page(buddy_pfn);

		buddy->order   = current_order;
		buddy->flags   = PG_BUDDY;
		buddy->zone_id = (uint8_t)ztype;
		buddy->_count  = 0;

		// Insertion du compagnon droit dans la liste de l'ordre inférieur
		list_add(&buddy->lru, &zone->free_area[current_order].free_list);
		zone->free_area[current_order].nr_free++;
	}

	// 4. Finalisation du bloc alloué (moitié gauche)
	page->flags &= ~PG_BUDDY;
	page->flags |= PG_ACTIVE;
	page->order = order;
	page->_count = 1;
	zone->free_pages -= (1UL << order);

	return page;
}

void free_pages(struct page *page, uint8_t order) {
	if (!page || order >= MAX_ORDER)
		return;

	uint8_t ztype = page->zone_id;
	if (ztype >= ZONE_MAX)
		return;

	struct zone *zone = &g_contig_node.node_zones[ztype];
	size_t pfn = page_to_pfn(page);

	zone->free_pages += (1UL << order);

	// Fusion récursive (Coalescing) avec le compagnon via XOR
	while (order < MAX_ORDER - 1) {
		size_t buddy_pfn = pfn ^ (1UL << order);

		// GARDE-FOU CRITIQUE : buddy_pfn doit appartenir à l'espace couvert par mem_map
		if (buddy_pfn >= g_contig_node.node_spanned_pages)
			break;

		struct page *buddy = pfn_to_page(buddy_pfn);
		if (!page_is_buddy(zone, buddy, buddy_pfn, order, ztype))
			break;

		// Extraire le compagnon de sa liste libre
		list_del(&buddy->lru);
		zone->free_area[order].nr_free--;
		buddy->flags &= ~PG_BUDDY;

		// Le bloc parent fusionné commence toujours au PFN le plus bas (aligné)
		pfn = pfn & ~(1UL << order);
		page = pfn_to_page(pfn);
		order++;
	}

	// Insertion du bloc consolidé dans la liste de l'ordre atteint
	page->order = order;
	page->flags = PG_BUDDY;
	page->_count = 0;
	list_add(&page->lru, &zone->free_area[order].free_list);
	zone->free_area[order].nr_free++;
}

void print_buddy_stats(void) {
	printf("\n=== STATISTIQUES DU NOEUD UMA & BUDDY ALLOCATOR ===\n");
	printf("Noeud UMA [PFN %d -> %d] : %d pages presentes, %d pages etendues\n",
	       (int)g_contig_node.node_start_pfn,
	       (int)g_contig_node.node_spanned_pages,
	       (int)g_contig_node.node_present_pages,
	       (int)g_contig_node.node_spanned_pages);

	for (int z = 0; z < ZONE_MAX; z++) {
		struct zone *zone = &g_contig_node.node_zones[z];
		if (!zone->spanned_pages)
			continue;

		printf("Zone %-7s [PFN %d -> %d] : Libres: %d Ko (%d pages) / Presentes: %d Ko / Spanned: %d Ko\n",
		       zone->name,
		       (int)zone->zone_start_pfn,
		       (int)(zone->zone_start_pfn + zone->spanned_pages),
		       (int)(zone->free_pages * 4),
		       (int)zone->free_pages,
		       (int)(zone->present_pages * 4),
		       (int)(zone->spanned_pages * 4));

		printf("  Ordres : ");
		for (int o = 0; o < MAX_ORDER; o++) {
			printf("[%d:%d] ", o, (int)zone->free_area[o].nr_free);
		}
		printf("\n");
	}
	printf("===================================================\n");
}
```

---

## Étape 6 : Initialisation PMM, Nœud & Zones (`pmm.h` & `pmm.c`)

Cette étape réutilise [`detect_memory(mbi)`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/mm/mminfo_ram.c) déjà présent dans ton dépôt pour dimensionner le nœud UMA (`g_contig_node`), découper les 3 zones (`ZONE_DMA`, `ZONE_NORMAL`, `ZONE_HIGHMEM`), placer `mem_map` à `__kernel_end` et injecter toutes les pages libres dans le Buddy Allocator.

### 1. En-tête : **`include/arch/i386/mm/pmm.h`**
```c
#ifndef KFS_PMM_H
#define KFS_PMM_H

#include "boot/multiboot.h"
#include "arch/i386/mm/buddy.h"
#include "arch/i386/mm/mminfo_ram.h"

void init_pmm(multiboot_info_t *mbi);

#endif // KFS_PMM_H
```

### 2. Implémentation : **`arch/i386/mm/pmm.c`**
```c
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
```

---

## Étape 7 : Mapping Virtuel Direct & Fenêtre `kmap` (`vmm.h` & `vmm.c`)

### 1. En-tête : **`include/arch/i386/mm/vmm.h`**
```c
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
```

### 2. Implémentation : **`arch/i386/mm/vmm.c`**
```c
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

	// 1. Identity Mapping des 4 premiers Mo (0x00000000 à 0x00400000)
	for (uint32_t i = 0; i < 1024; i++) {
		g_identity_pt[i] = (i * 4096) | 0x03; // Present, R/W
	}
	g_page_directory[0] = ((uint32_t)g_identity_pt) | 0x03;

	// 2. Higher-Half Direct Mapping : 0xC0000000 -> 0x00000000
	for (uint32_t i = 0; i < 1024; i++) {
		g_kernel_pt[i] = (i * 4096) | 0x03;
	}
	g_page_directory[0xC0000000 >> 22] = ((uint32_t)g_kernel_pt) | 0x03;

	// 3. Fenêtre dynamique kmap à 0xFE000000 (pour ZONE_HIGHMEM)
	kmemset(g_kmap_pt, 0, sizeof(g_kmap_pt));
	g_page_directory[KMAP_VIRT_BASE >> 22] = ((uint32_t)g_kmap_pt) | 0x03;

	// 4. Paging Récursif sur l'entrée 1023
	g_page_directory[1023] = ((uint32_t)g_page_directory) | 0x03;

	// 5. Chargement de CR3 et activation matérielle de CR0.PG
	__asm__ volatile ("movl %0, %%cr3" :: "r"(g_page_directory));
	uint32_t cr0;
	__asm__ volatile ("movl %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	__asm__ volatile ("movl %0, %%cr0" :: "r"(cr0));
}

void *kmap(struct page *page) {
	if (page->zone_id != ZONE_HIGHMEM) {
		return page_to_virt(page); // ZONE_DMA et ZONE_NORMAL sont déjà mappées directement
	}

	for (uint32_t i = 0; i < 1024; i++) {
		if (!(g_kmap_pt[i] & 0x01)) {
			g_kmap_pt[i] = (uint32_t)page_to_phys(page) | 0x03;
			uintptr_t vaddr = KMAP_VIRT_BASE + (i * 4096);
			__asm__ volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
			return (void *)vaddr;
		}
	}
	return NULL; // Fenêtre kmap saturée
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
```

---

## Étape 8 : Intégration Makefile

Mettez à jour [`arch/i386/mm/module.mk`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/mm/module.mk) (qui est déjà inclus par [`arch/i386/module.mk`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/module.mk)) :

```make
# --- Sources to be compiled ---

SRCS += arch/i386/mm/mminfo_ram.c \
        arch/i386/mm/buddy.c      \
        arch/i386/mm/pmm.c        \
        arch/i386/mm/vmm.c
```

---

## Étape 9 : Câblage dans `kernel/kmain.c` & Tests de Validation

Dans [`kernel/kmain.c`](file:///home/gcaptari/Documents/Kernel-From-Scratch/kernel/kmain.c) :

```c
#include "arch/i386/mm/pmm.h"
#include "arch/i386/mm/vmm.h"
#include "arch/i386/mm/buddy.h"
#include "klibc/utility/printf.h"

void test_buddy_allocator(void) {
	printf("\n--- TEST DU BUDDY ALLOCATOR & DES ZONES ---\n");
	printf("[1] Allocation d'un bloc d'ordre 3 (8 pages = 32 Ko) dans ZONE_NORMAL...\n");
	struct page *p1 = alloc_pages(ZONE_NORMAL, 3);
	if (p1) {
		printf("    Succes -> PFN: %d, Zone: %d, Phys: 0x%llx\n",
		       (int)page_to_pfn(p1), (int)p1->zone_id, page_to_phys(p1));
	}

	printf("[2] Allocation d'un bloc d'ordre 1 (2 pages = 8 Ko) dans ZONE_DMA...\n");
	struct page *p2 = alloc_pages(ZONE_DMA, 1);
	if (p2) {
		printf("    Succes -> PFN: %d, Zone: %d, Phys: 0x%llx\n",
		       (int)page_to_pfn(p2), (int)p2->zone_id, page_to_phys(p2));
	}

	print_buddy_stats();

	printf("[3] Liberation des blocs alloues (Test du Coalescing par zone)...\n");
	free_pages(p1, 3);
	free_pages(p2, 1);

	printf("[4] Statistiques apres fusion complete :\n");
	print_buddy_stats();
}
```

Puis dans [`kmain()`](file:///home/gcaptari/Documents/Kernel-From-Scratch/kernel/kmain.c#L77-L102) après `kinit(mbi)` et `init_terminal("Main Terminal")` :
```c
	init_pmm(mbi);
	test_buddy_allocator();
```
