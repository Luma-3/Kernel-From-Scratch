#include "arch/i386/mm/buddy.h"
#include "klibc/utility/printf.h"

pg_data_t    g_contig_node;
struct page *mem_map = NULL;

// Teste avec sécurité absolue si un compagnon est libre et fusionnable dans la même zone
static inline bool page_is_buddy(const struct zone *zone, const struct page *page,
                                 size_t pfn, uint8_t order, uint8_t target_zone_id) {
	if (pfn < zone->zone_start_pfn || pfn >= (zone->zone_start_pfn + zone->spanned_pages))
		return false;

	if (!(page->flags & PG_BUDDY))
		return false;

	if (page->order != order)
		return false;

	if (page->zone_id != target_zone_id)
		return false;

	return true;
}

struct page *alloc_pages(enum zone_type ztype, uint8_t order) {
	if (order >= MAX_ORDER || ztype >= ZONE_MAX)
		return NULL;

	struct zone *zone = &g_contig_node.node_zones[ztype];

	uint8_t current_order;
	for (current_order = order; current_order < MAX_ORDER; current_order++) {
		if (!list_empty(&zone->free_area[current_order].free_list))
			break;
	}

	if (current_order == MAX_ORDER)
		return NULL;

	struct list_head *entry = zone->free_area[current_order].free_list.next;
	list_del(entry);
	zone->free_area[current_order].nr_free--;

	struct page *page = list_entry(entry, struct page, lru);

	while (current_order > order) {
		current_order--;

		size_t buddy_pfn = page_to_pfn(page) + (1UL << current_order);
		struct page *buddy = pfn_to_page(buddy_pfn);

		buddy->order   = current_order;
		buddy->flags   = PG_BUDDY;
		buddy->zone_id = (uint8_t)ztype;
		buddy->_count  = 0;

		list_add(&buddy->lru, &zone->free_area[current_order].free_list);
		zone->free_area[current_order].nr_free++;
	}

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

	while (order < MAX_ORDER - 1) {
		size_t buddy_pfn = pfn ^ (1UL << order);

		if (buddy_pfn >= g_contig_node.node_spanned_pages)
			break;

		struct page *buddy = pfn_to_page(buddy_pfn);
		if (!page_is_buddy(zone, buddy, buddy_pfn, order, ztype))
			break;

		list_del(&buddy->lru);
		zone->free_area[order].nr_free--;
		buddy->flags &= ~PG_BUDDY;

		pfn = pfn & ~(1UL << order);
		page = pfn_to_page(pfn);
		order++;
	}

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

		printf("Zone %s [PFN %d -> %d] : Libres: %d Ko (%d pages) / Presentes: %d Ko / Spanned: %d Ko\n",
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
