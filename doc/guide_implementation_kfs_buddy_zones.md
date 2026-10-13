# 🛠️ Guide d'Implémentation Modulaire KFS : Buddy Allocator, Zones & Bootstrap PMM

> **Projet :** Kernel From Scratch (KFS) — Architecture x86 (IA-32 / i386)  
> **Composants :** Buddy Allocator, Modélisation `struct page` / PFN, Architecture par Zones (`ZONE_DMA`, `ZONE_NORMAL`, `ZONE_HIGHMEM`), Nœuds Mémoire (`pg_data_t`) & Mapping Virtuel  
> **Conformité stricte :** `-std=gnu23 -Wall -Wextra -Wpedantic -Wshadow -Werror -ffreestanding`  
> **Auteur :** Antigravity AI Assistant

---

## Table des Matières
1. [Arborescence et Fichiers Cibles](#1-arborescence-et-fichiers-cibles)
2. [Étape 1 : Linker Script ([`boot/linker.ld`](file:///home/gcaptari/Documents/Kernel-From-Scratch/boot/linker.ld))](#étape-1--linker-script)
3. [Étape 2 : Liste Chaînée Circulaire Intrusive (`klibc/utility/list.h`)](#étape-2--liste-chaînée-circulaire-intrusive)
4. [Étape 3 : Définition des Zones & Nœud UMA ([`arch/i386/mm/mmzone.h`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/mm/mmzone.h))](#étape-3--définition-des-zones--nœud-uma)
5. [Étape 4 : Descripteurs de Pages & Arithmétique PFN (`arch/i386/mm/page.h`)](#étape-4--descripteurs-de-pages--arithmétique-pfn)
6. [Étape 5 : L'Allocateur Compagnon (`buddy.h` & `buddy.c`)](#étape-5--lallocateur-compagnon)
7. [Étape 6 : Initialisation PMM & Bootstrap Multiboot (`pmm.h` & `pmm.c`)](#étape-6--initialisation-pmm--bootstrap-multiboot)
8. [Étape 7 : Mapping Virtuel Direct & Fenêtre `kmap` (`vmm.h` & `vmm.c`)](#étape-7--mapping-virtuel-direct--fenêtre-kmap)
9. [Étape 8 : Intégration Makefile (`arch/i386/mm/module.mk`)](#étape-8--intégration-makefile)
10. [Étape 9 : Câblage dans [`kernel/kmain.c`](file:///home/gcaptari/Documents/Kernel-From-Scratch/kernel/kmain.c) & Tests de Validation](#étape-9--câblage-dans-kernelkmainc--tests-de-validation)

---

## 1. Arborescence et Fichiers Cibles

Voici la cartographie exacte des fichiers créés et modifiés dans le dépôt KFS :

```
Kernel-From-Scratch/
├── boot/
│   └── linker.ld                  <-- [MODIFIÉ] Symboles _kernel_start et _kernel_end
├── klibc/
│   └── utility/
│       └── list.h                 <-- [CRÉÉ] Liste circulaire intrusive Linux (list_head)
├── arch/i386/
│   ├── module.mk                  <-- [MODIFIÉ] Inclusion de arch/i386/mm/module.mk
│   └── mm/
│       ├── module.mk              <-- [CRÉÉ] Règle de compilation du sous-système MM
│       ├── mmzone.h               <-- [COMPLÉTÉ] Zones ZONE_DMA/NORMAL/HIGHMEM & Nœud UMA
│       ├── page.h                 <-- [CRÉÉ] struct page, PFN macros, conversions
│       ├── buddy.h                <-- [CRÉÉ] Prototypes alloc_pages() et free_pages()
│       ├── buddy.c                <-- [CRÉÉ] Cœur de l'algorithme Buddy (Scindage & Fusion)
│       ├── pmm.h                  <-- [CRÉÉ] En-tête du gestionnaire de mémoire physique
│       ├── pmm.c                  <-- [CRÉÉ] Parsing Multiboot & bootstrap du mem_map
│       ├── vmm.h                  <-- [CRÉÉ] Direct mapping & interface kmap
│       └── vmm.c                  <-- [CRÉÉ] Configuration Page Directory et kmap
└── kernel/
    └── kmain.c                    <-- [MODIFIÉ] Appel d'initialisation et tests complets
```

---

## Étape 1 : Linker Script

Le sous-système mémoire doit connaître les frontières physiques exactes occupées par le binaire du noyau afin de ne jamais les allouer ni les corrompre.

Éditez [`boot/linker.ld`](file:///home/gcaptari/Documents/Kernel-From-Scratch/boot/linker.ld) :

```ld
ENTRY(_start)

SECTIONS
{
	. = 2M;

	/* Symbole marquant le début absolu du binaire noyau */
	_kernel_start = .;

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

	/* Symbole marquant la fin du noyau alignée sur une page de 4 Ko */
	. = ALIGN(4K);
	_kernel_end = .;
}
```

---

## Étape 2 : Liste Chaînée Circulaire Intrusive

Le Buddy Allocator stocke les blocs libres dans des listes chaînées sans recourir à la moindre allocation dynamique supplémentaire. Nous implémentons la structure intrusive standard du noyau Linux (`struct list_head`).

Créez le fichier **`klibc/utility/list.h`** :

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

## Étape 3 : Définition des Zones & Nœud UMA

Mettez à jour [`arch/i386/mm/mmzone.h`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/mm/mmzone.h) pour modéliser les 3 zones physiques et le nœud UMA :

```c
#ifndef KFS_MMZONE_H
#define KFS_MMZONE_H

#include <stddef.h>
#include <stdint.h>
#include "klibc/utility/list.h"

#define MAX_ORDER 11 // Ordres 0 à 10 inclus (1 page à 1024 pages = 4 Mo)

enum zone_type {
    ZONE_DMA = 0,     // 0 à 16 Mo (matériel hérité ISA)
    ZONE_NORMAL,      // 16 Mo à 896 Mo (mappage direct higher-half)
    ZONE_HIGHMEM,     // > 896 Mo (mappings dynamiques via kmap)
    ZONE_MAX
};

struct free_area {
    struct list_head free_list; // Tête de liste des blocs libres
    size_t           nr_free;   // Nombre de blocs disponibles à cet ordre
};

struct zone {
    const char       *name;                 // Nom lisible ("DMA", "Normal", "HighMem")
    size_t            zone_start_pfn;       // Premier PFN de la zone
    size_t            spanned_pages;        // Étendue totale en pages
    size_t            present_pages;        // Pages réelles utilisables
    size_t            free_pages;           // Total des pages actuellement libres
    struct free_area  free_area[MAX_ORDER]; // Listes d'ordres du Buddy Allocator
};

typedef struct pglist_data {
    size_t           node_start_pfn;        // Début du nœud (généralement 0)
    size_t           node_present_pages;    // Total des pages physiques réelles
    struct zone      node_zones[ZONE_MAX];  // Les zones de ce nœud
} pg_data_t;

extern pg_data_t g_contig_node;

#endif // KFS_MMZONE_H
```

---

## Étape 4 : Descripteurs de Pages & Arithmétique PFN

Créez le fichier **`arch/i386/mm/page.h`** :

```c
#ifndef KFS_PAGE_H
#define KFS_PAGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "klibc/utility/list.h"
#include "mmzone.h"

#define PAGE_SIZE       4096ULL
#define PAGE_SHIFT      12

// Masques d'état du cadre physique
#define PG_RESERVED     (1U << 0) // Page réservée au matériel, BIOS ou binaire noyau
#define PG_BUDDY        (1U << 1) // Page libre gérée par le Buddy Allocator
#define PG_ACTIVE       (1U << 2) // Page allouée en cours d'utilisation

struct page {
    uint32_t         flags;     // Masque d'état (PG_RESERVED, PG_BUDDY, etc.)
    uint32_t         _count;    // Compteur de références actives
    uint8_t          order;     // Ordre actuel du bloc (0..MAX_ORDER-1)
    uint8_t          zone_id;   // ZONE_DMA, ZONE_NORMAL ou ZONE_HIGHMEM
    struct list_head lru;       // Maillon de liste pour free_area
};

extern struct page *mem_map;    // Tableau plat de tous les cadres physiques de la RAM

// Primitives de conversion O(1)
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

### 1. En-tête : **`arch/i386/mm/buddy.h`**
```c
#ifndef KFS_BUDDY_H
#define KFS_BUDDY_H

#include "page.h"

struct page *alloc_pages(enum zone_type ztype, uint8_t order);
void         free_pages(struct page *page, uint8_t order);
void         print_buddy_stats(void);

#endif // KFS_BUDDY_H
```

### 2. Implémentation Robuste : **`arch/i386/mm/buddy.c`**
```c
#include "buddy.h"
#include "klibc/utility/printf.h"

pg_data_t    g_contig_node;
struct page *mem_map = NULL;

// Teste avec sécurité absolue si un compagnon est libre et fusionnable
static inline bool page_is_buddy(const struct zone *zone, const struct page *page, 
                                 size_t pfn, uint8_t order, uint8_t target_zone_id) {
    // 1. Vérification stricte des limites de zone
    if (pfn < zone->zone_start_pfn || pfn >= (zone->zone_start_pfn + zone->spanned_pages))
        return false;

    // 2. Le cadre doit être marqué libre dans le Buddy
    if (!(page->flags & PG_BUDDY))
        return false;

    // 3. L'ordre doit correspondre exactement
    if (page->order != order)
        return false;

    // 4. La zone doit être identique (interdiction de fusion inter-zones)
    if (page->zone_id != target_zone_id)
        return false;

    return true;
}

struct page *alloc_pages(enum zone_type ztype, uint8_t order) {
    if (order >= MAX_ORDER || ztype >= ZONE_MAX)
        return NULL;

    struct zone *zone = &g_contig_node.node_zones[ztype];

    // 1. Recherche du plus petit ordre disponible >= order
    uint8_t current_order;
    for (current_order = order; current_order < MAX_ORDER; current_order++) {
        if (!list_empty(&zone->free_area[current_order].free_list))
            break;
    }

    if (current_order == MAX_ORDER)
        return NULL; // Épuisement de la mémoire à cet ordre dans cette zone

    // 2. Retrait du bloc de sa liste libre
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
    if (ztype >= ZONE_MAX) return;

    struct zone *zone = &g_contig_node.node_zones[ztype];
    size_t pfn = page_to_pfn(page);

    zone->free_pages += (1UL << order);

    // Fusion récursive avec le compagnon via XOR
    while (order < MAX_ORDER - 1) {
        size_t buddy_pfn = pfn ^ (1UL << order);

        // GARDE-FOU ESSENTIEL : buddy_pfn doit être contenu dans mem_map
        if (buddy_pfn >= g_contig_node.node_present_pages)
            break;

        struct page *buddy = pfn_to_page(buddy_pfn);
        if (!page_is_buddy(zone, buddy, buddy_pfn, order, ztype))
            break;

        // Extraire le compagnon de sa liste libre
        list_del(&buddy->lru);
        zone->free_area[order].nr_free--;
        buddy->flags &= ~PG_BUDDY;

        // Le bloc fusionné démarre au plus petit PFN
        pfn = pfn & ~(1UL << order);
        page = pfn_to_page(pfn);
        order++;
    }

    // Réinsertion du bloc consolidé dans le bon ordre
    page->order = order;
    page->flags = PG_BUDDY;
    page->_count = 0;
    list_add(&page->lru, &zone->free_area[order].free_list);
    zone->free_area[order].nr_free++;
}

void print_buddy_stats(void) {
    printf("\n=== STATISTIQUES DU BUDDY ALLOCATOR ===\n");
    for (int z = 0; z < ZONE_MAX; z++) {
        struct zone *zone = &g_contig_node.node_zones[z];
        if (!zone->spanned_pages) continue;

        printf("Zone %-7s: Libres: %d Ko (%d pages) / Total: %d Ko\n",
               zone->name,
               (int)(zone->free_pages * 4),
               (int)zone->free_pages,
               (int)(zone->spanned_pages * 4));

        printf("  Ordres : ");
        for (int o = 0; o < MAX_ORDER; o++) {
            printf("[%d:%d] ", o, (int)zone->free_area[o].nr_free);
        }
        printf("\n");
    }
    printf("=========================================\n");
}
```

---

## Étape 6 : Initialisation PMM & Bootstrap Multiboot (`pmm.h` & `pmm.c`)

### 1. En-tête : **`arch/i386/mm/pmm.h`**
```c
#ifndef KFS_PMM_H
#define KFS_PMM_H

#include "multiboot.h"
#include "buddy.h"

// Constantes Multiboot standard pour la détection de mémoire
#ifndef MULTIBOOT_MEMORY_AVAILABLE
#define MULTIBOOT_MEMORY_AVAILABLE 1
#endif

void init_pmm(multiboot_info_t *mbi);

#endif // KFS_PMM_H
```

### 2. Implémentation : **`arch/i386/mm/pmm.c`**
```c
#include "pmm.h"
#include "klibc/memory/mem.h"
#include "klibc/utility/printf.h"

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

static void init_single_zone(struct zone *zone, const char *name, size_t start_pfn, size_t end_pfn) {
    zone->name = name;
    zone->zone_start_pfn = start_pfn;
    zone->spanned_pages = (end_pfn > start_pfn) ? (end_pfn - start_pfn) : 0;
    zone->present_pages = zone->spanned_pages;
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

    // 1. Détection de la RAM maximale
    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *)(uintptr_t)mbi->mmap_addr;
    uintptr_t mmap_end = mbi->mmap_addr + mbi->mmap_length;
    uint64_t max_phys_addr = 0;

    while ((uintptr_t)mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t seg_end = mmap->addr + mmap->len;
            if (seg_end > max_phys_addr)
                max_phys_addr = seg_end;
        }
        mmap = (multiboot_mmap_entry_t *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }

    size_t total_pfn = (size_t)(max_phys_addr >> PAGE_SHIFT);

    // 2. Initialisation du nœud global UMA
    g_contig_node.node_start_pfn = 0;
    g_contig_node.node_present_pages = total_pfn;

    // 3. Découpage des zones physiques
    size_t dma_end_pfn     = (total_pfn < 4096) ? total_pfn : 4096;       // 16 Mo
    size_t normal_end_pfn  = (total_pfn < 229376) ? total_pfn : 229376; // 896 Mo
    size_t highmem_end_pfn = total_pfn;

    init_single_zone(&g_contig_node.node_zones[ZONE_DMA], "DMA", 0, dma_end_pfn);
    init_single_zone(&g_contig_node.node_zones[ZONE_NORMAL], "Normal", dma_end_pfn, normal_end_pfn);
    if (total_pfn > 229376) {
        init_single_zone(&g_contig_node.node_zones[ZONE_HIGHMEM], "HighMem", normal_end_pfn, highmem_end_pfn);
    } else {
        init_single_zone(&g_contig_node.node_zones[ZONE_HIGHMEM], "HighMem", 0, 0);
    }

    // 4. Allocation du mem_map via le Bump Allocator juste après _kernel_end
    size_t mem_map_bytes = total_pfn * sizeof(struct page);
    mem_map = (struct page *)_kernel_end;
    kmemset(mem_map, 0, mem_map_bytes);

    uintptr_t mem_map_end = (uintptr_t)mem_map + mem_map_bytes;
    size_t reserved_pfn_limit = (mem_map_end + PAGE_SIZE - 1) >> PAGE_SHIFT;

    // 5. Marquer initialement tous les cadres comme réservés
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

    // 6. Injection des pages physiques disponibles dans le Buddy Allocator
    mmap = (multiboot_mmap_entry_t *)(uintptr_t)mbi->mmap_addr;
    while ((uintptr_t)mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            size_t start_pfn = (size_t)(mmap->addr >> PAGE_SHIFT);
            size_t count     = (size_t)(mmap->len >> PAGE_SHIFT);

            for (size_t i = 0; i < count; i++) {
                size_t pfn = start_pfn + i;
                // Exclure le premier Mo (pfn < 256) et l'empreinte du noyau + mem_map
                if (pfn >= 256 && pfn >= reserved_pfn_limit && pfn < total_pfn) {
                    mem_map[pfn].flags = 0;
                    mem_map[pfn]._count = 0;
                    // free_pages() va fusionner automatiquement via Coalescing !
                    free_pages(&mem_map[pfn], 0);
                }
            }
        }
        mmap = (multiboot_mmap_entry_t *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }

    printf("[PMM] Initialisation terminee avec succes : %d PFNs physiques geres.\n", (int)total_pfn);
}
```

---

## Étape 7 : Mapping Virtuel Direct & Fenêtre `kmap` (`vmm.h` & `vmm.c`)

### 1. En-tête : **`arch/i386/mm/vmm.h`**
```c
#ifndef KFS_VMM_H
#define KFS_VMM_H

#include "page.h"

#define PAGE_OFFSET    0xC0000000UL // Base virtuelle du noyau (Higher-Half)
#define KMAP_VIRT_BASE 0xFE000000UL // Fenêtre virtuelle kmap (4 Mo)

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
#include "vmm.h"
#include "klibc/memory/mem.h"

static uint32_t g_page_directory[1024] __attribute__((aligned(4096)));
static uint32_t g_identity_pt[1024]    __attribute__((aligned(4096)));
static uint32_t g_kernel_pt[1024]      __attribute__((aligned(4096)));
static uint32_t g_kmap_pt[1024]        __attribute__((aligned(4096)));

void vmm_map_page(uintptr_t virt, uint64_t phys, uint32_t flags) {
    uint32_t pde_idx = virt >> 22;
    uint32_t pte_idx = (virt >> 12) & 0x3FF;

    // Si la table de page n'existe pas dans le Page Directory
    if (!(g_page_directory[pde_idx] & 0x01)) {
        // En bootstrap simplifié, les tables principales sont pré-allouées
        return;
    }

    uint32_t *pt = (uint32_t *)(g_page_directory[pde_idx] & ~0xFFF);
    pt[pte_idx] = ((uint32_t)phys & ~0xFFF) | (flags & 0xFFF) | 0x01;
    asm volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

void vmm_unmap_page(uintptr_t virt) {
    uint32_t pde_idx = virt >> 22;
    uint32_t pte_idx = (virt >> 12) & 0x3FF;

    if (g_page_directory[pde_idx] & 0x01) {
        uint32_t *pt = (uint32_t *)(g_page_directory[pde_idx] & ~0xFFF);
        pt[pte_idx] = 0;
        asm volatile ("invlpg (%0)" :: "r"(virt) : "memory");
    }
}

void init_vmm(void) {
    kmemset(g_page_directory, 0, sizeof(g_page_directory));

    // 1. Identity Mapping des 4 premiers Mo (0x0 à 0x400000)
    for (uint32_t i = 0; i < 1024; i++) {
        g_identity_pt[i] = (i * 4096) | 0x03; // Present, R/W
    }
    g_page_directory[0] = ((uint32_t)g_identity_pt) | 0x03;

    // 2. Higher-Half Direct Mapping : 0xC0000000 -> 0x00000000
    for (uint32_t i = 0; i < 1024; i++) {
        g_kernel_pt[i] = (i * 4096) | 0x03;
    }
    g_page_directory[0xC0000000 >> 22] = ((uint32_t)g_kernel_pt) | 0x03;

    // 3. Fenêtre dynamique kmap à 0xFE000000
    kmemset(g_kmap_pt, 0, sizeof(g_kmap_pt));
    g_page_directory[KMAP_VIRT_BASE >> 22] = ((uint32_t)g_kmap_pt) | 0x03;

    // 4. Paging Récursif sur l'entrée 1023
    g_page_directory[1023] = ((uint32_t)g_page_directory) | 0x03;

    // 5. Chargement de CR3 et activation matérielle de CR0.PG
    asm volatile ("movl %0, %%cr3" :: "r"(g_page_directory));
    uint32_t cr0;
    asm volatile ("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("movl %0, %%cr0" :: "r"(cr0));
}

void *kmap(struct page *page) {
    if (page->zone_id != ZONE_HIGHMEM) {
        return page_to_virt(page); // Déjà mappée directement en lowmem !
    }

    for (uint32_t i = 0; i < 1024; i++) {
        if (!(g_kmap_pt[i] & 0x01)) {
            g_kmap_pt[i] = (uint32_t)page_to_phys(page) | 0x03;
            uintptr_t vaddr = KMAP_VIRT_BASE + (i * 4096);
            asm volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
            return (void *)vaddr;
        }
    }
    return NULL; // Fenêtre kmap saturée
}

void kunmap(struct page *page) {
    if (page->zone_id != ZONE_HIGHMEM) return;

    uint32_t phys = (uint32_t)page_to_phys(page);
    for (uint32_t i = 0; i < 1024; i++) {
        if ((g_kmap_pt[i] & ~0xFFF) == phys) {
            g_kmap_pt[i] = 0;
            uintptr_t vaddr = KMAP_VIRT_BASE + (i * 4096);
            asm volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
            break;
        }
    }
}
```

---

## Étape 8 : Intégration Makefile

Créez le fichier **`arch/i386/mm/module.mk`** :

```make
INCLUDES += -Iarch/i386/mm

# --- Sources to be compiled ---
SRCS += arch/i386/mm/buddy.c \
        arch/i386/mm/pmm.c   \
        arch/i386/mm/vmm.c
```

Ajoutez la ligne suivante à [`arch/i386/module.mk`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/module.mk) :
```make
include arch/i386/mm/module.mk
```

---

## Étape 9 : Câblage dans `kmain.c` & Tests de Validation

Dans [`kernel/kmain.c`](file:///home/gcaptari/Documents/Kernel-From-Scratch/kernel/kmain.c) :

```c
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/buddy.h"
#include "klibc/utility/printf.h"

void test_buddy_allocator(void) {
    printf("\n--- TEST DU BUDDY ALLOCATOR ---\n");
    printf("[1] Allocation d'un bloc d'ordre 3 (8 pages = 32 Ko) dans ZONE_NORMAL...\n");
    struct page *p1 = alloc_pages(ZONE_NORMAL, 3);
    if (p1) {
        printf("    Succes -> PFN: %d, Phys: 0x%llx, Virt: 0x%p\n",
               (int)page_to_pfn(p1), page_to_phys(p1), page_to_virt(p1));
    }

    printf("[2] Allocation d'un bloc d'ordre 1 (2 pages = 8 Ko) dans ZONE_NORMAL...\n");
    struct page *p2 = alloc_pages(ZONE_NORMAL, 1);
    if (p2) {
        printf("    Succes -> PFN: %d\n", (int)page_to_pfn(p2));
    }

    print_buddy_stats();

    printf("[3] Liberation des blocs alloues (Test du Coalescing)...\n");
    free_pages(p1, 3);
    free_pages(p2, 1);

    printf("[4] Statistiques apres fusion complete :\n");
    print_buddy_stats();
}
```

À appeler dans votre fonction `kinit(mbi)` :
```c
init_pmm(mbi);
init_vmm();
test_buddy_allocator();
```
