#ifndef KFS_BUDDY_H
#define KFS_BUDDY_H

#include "arch/i386/mm/page.h"

struct page *alloc_pages(enum zone_type ztype, uint8_t order);
void         free_pages(struct page *page, uint8_t order);
void         print_buddy_stats(void);

#endif // KFS_BUDDY_H
