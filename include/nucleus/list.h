#ifndef _NUCLEUS_LIST_H
#define _NUCLEUS_LIST_H

#include <nucleus/stddef.h>
#include <nucleus/types.h>

/**
 * struct list_head - Doubly linked list structure
 * @next:   pointer to the next element in the list
 * @prev:   pointer to the previous element in the list
 */
struct list_head {
    struct list_head *next, *prev;
};

/**
 * LIST_HEAD_INIT - Static initializer for a list head
 * @name:   name of the list_head variable
 */
#define LIST_HEAD_INIT(name) {&(name), &(name)}

/**
 * LIST_HEAD - Declare and initialize a list head
 * @name:   name of the list_head variable to declare
 */
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

/**
 * INIT_LIST_HEAD - Initialize a list head
 * @list:   the list_head to initialize
 */
static inline void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new, struct list_head *prev,
                              struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * list_add - Add a new entry at the head of the list
 * @new:    new entry to be added
 * @head:   list head to add it after
 */
static inline void list_add(struct list_head *new, struct list_head *head) {
    __list_add(new, head, head->next);
}

/**
 * list_add_tail - Add a new entry at the tail of the list
 * @new:    new entry to be added
 * @head:   list head to add it before
 */
static inline void list_add_tail(struct list_head *new,
                                 struct list_head *head) {
    __list_add(new, head->prev, head);
}

/*
 * Internal helper - delete entry by making the prev/next entries point to each
 * other.
 */
static inline void __list_del(struct list_head *prev, struct list_head *next) {
    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - Delete an entry from the list
 * @entry:  the element to delete from the list
 *
 * Note: list_empty() on entry does not return true after this.
 */
static inline void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

/**
 * list_del_init - Delete an entry from the list and reinitialize it
 * @entry:  the element to delete from the list
 */
static inline void list_del_init(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * list_replace - Replace old entry with new entry
 * @old:    the element to be replaced
 * @new:    the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void list_replace(struct list_head *old, struct list_head *new) {
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

/**
 * list_replace_init - Replace old entry with new entry and initialize the old
 * one
 * @old:    the element to be replaced
 * @new:    the new element to insert
 */
static inline void list_replace_init(struct list_head *old,
                                     struct list_head *new) {
    list_replace(old, new);
    INIT_LIST_HEAD(old);
}

/**
 * list_move - Delete from one list and add as head to another
 * @list:   the entry to move
 * @head:   the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head) {
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/**
 * list_move_tail - Delete from one list and add as tail to another
 * @list:   the entry to move
 * @head:   the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
                                  struct list_head *head) {
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/**
 * list_is_last - Tests whether @list is the last entry in list @head
 * @list:   the entry to test
 * @head:   the head of the list
 */
static inline int list_is_last(const struct list_head *list,
                               const struct list_head *head) {
    return list->next == head;
}

/**
 * list_empty - Tests whether a list is empty
 * @head:   the list to test
 */
static inline int list_empty(const struct list_head *head) {
    return head->next == head;
}

/**
 * list_is_singular - Tests whether a list has just one entry
 * @head:   the list to test
 */
static inline int list_is_singular(const struct list_head *head) {
    return !list_empty(head) && (head->next == head->prev);
}

/**
 * list_entry - Get the struct for this entry
 * @ptr:    the &struct list_head pointer
 * @type:   the type of the struct this is embedded in
 * @member: the name of the list_head within the struct
 */
#define list_entry(ptr, type, member)                                          \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/**
 * list_first_entry - Get the first element from a list
 * @ptr:    the list head to take the element from
 * @type:   the type of the struct this is embedded in
 * @member: the name of the list_head within the struct
 *
 * Note: list must not be empty.
 */
#define list_first_entry(ptr, type, member)                                    \
    list_entry((ptr)->next, type, member)

/**
 * list_last_entry - Get the last element from a list
 * @ptr:    the list head to take the element from
 * @type:   the type of the struct this is embedded in
 * @member: the name of the list_head within the struct
 *
 * Note: list must not be empty.
 */
#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)

/**
 * list_first_entry_or_null - Get the first element from a list or NULL
 * @ptr:    the list head to take the element from
 * @type:   the type of the struct this is embedded in
 * @member: the name of the list_head within the struct
 */
#define list_first_entry_or_null(ptr, type, member)                            \
    (!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)

/**
 * list_next_entry - Get the next element in list
 * @pos:    the type * to cursor
 * @member: the name of the list_head within the struct
 */
#define list_next_entry(pos, member)                                           \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_prev_entry - Get the previous element in list
 * @pos:    the type * to cursor
 * @member: the name of the list_head within the struct
 */
#define list_prev_entry(pos, member)                                           \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each - Iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor
 * @head:   the head for your list
 */
#define list_for_each(pos, head)                                               \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe - Iterate over a list safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list
 */
#define list_for_each_safe(pos, n, head)                                       \
    for (pos = (head)->next, n = pos->next; pos != (head);                     \
         pos = n, n = pos->next)

/**
 * list_for_each_entry - Iterate over list of given type
 * @pos:    the type * to use as a loop cursor
 * @head:   the head for your list
 * @member: the name of the list_head within the struct
 */
#define list_for_each_entry(pos, head, member)                                 \
    for (pos = list_first_entry(head, typeof(*pos), member);                   \
         &pos->member != (head); pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_safe - Iterate over list of given type safe against
 * removal
 * @pos:    the type * to use as a loop cursor
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list
 * @member: the name of the list_head within the struct
 */
#define list_for_each_entry_safe(pos, n, head, member)                         \
    for (pos = list_first_entry(head, typeof(*pos), member),                   \
        n = list_next_entry(pos, member);                                      \
         &pos->member != (head); pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_reverse - Iterate backwards over list of given type
 * @pos:    the type * to use as a loop cursor
 * @head:   the head for your list
 * @member: the name of the list_head within the struct
 */
#define list_for_each_entry_reverse(pos, head, member)                         \
    for (pos = list_last_entry(head, typeof(*pos), member);                    \
         &pos->member != (head); pos = list_prev_entry(pos, member))

#endif /* _NUCLEUS_LIST_H */