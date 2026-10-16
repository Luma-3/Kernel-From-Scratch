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