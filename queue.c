#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head) {
        return NULL;
    }

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }

    element_t *entry = NULL;
    element_t *safe = NULL;
    list_for_each_entry_safe (entry, safe, l, list) {
        list_del(&entry->list);
        q_release_element(entry);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
element_t *create_element(char *s)
{
    element_t *elem = malloc(sizeof(element_t));
    if (!elem) {
        return NULL;
    }

    int length = strlen(s) + 1;
    elem->value = malloc(length);
    if (!elem->value) {
        free(elem);
        return NULL;
    }

    strncpy(elem->value, s, length);
    s[length - 1] = '\0';
    return elem;
}

bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }

    element_t *elem = create_element(s);
    if (!elem) {
        return false;
    }

    list_add(&elem->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }

    element_t *elem = create_element(s);
    if (!elem) {
        return false;
    }

    list_add_tail(&elem->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    element_t *elem = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, elem->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del(head->next);
    return elem;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    element_t *elem = list_last_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, elem->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del(head->prev);
    return elem;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head) {
        return 0;
    }

    int size = 0;
    struct list_head *node;
    list_for_each (node, head) {
        size++;
    }

    return size;
}

struct list_head *q_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return head;
    }

    struct list_head *slow = head->next;
    struct list_head *fast = head->next->next;
    while (fast && fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    return slow;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head)) {
        return false;
    }

    struct list_head *mid = q_mid(head);
    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));
    return true;
}

int cmp(const element_t *e1, const element_t *e2)
{
    const char *a = e1->value;
    const char *b = e2->value;

    return strcmp(a, b);
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head) {
        return false;
    }

    if (list_empty(head)) {
        return true;
    }

    struct list_head *node = NULL;
    struct list_head *safe = NULL;
    element_t *cur = NULL;
    list_for_each_safe (node, safe, head) {
        element_t *entry = list_entry(node, element_t, list);
        if (cur && cmp(cur, entry) == 0) {
            list_del(node);
            q_release_element(entry);
        } else {
            cur = entry;
        }
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head)) {
        return;
    }

    element_t *entry = NULL;
    list_for_each_entry (entry, head, list) {
        element_t *next_entry = list_entry(entry->list.next, element_t, list);
        if (&next_entry->list != head) {
            char *temp = entry->value;
            entry->value = next_entry->value;
            next_entry->value = temp;
            entry = next_entry;
        } else {
            break;
        }
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }

    struct list_head *node = NULL;
    struct list_head *safe = NULL;
    list_for_each_safe (node, safe, head) {
        struct list_head *prev = node->prev;
        struct list_head *next = node->next;
        node->prev = next;
        node->next = prev;
    }

    struct list_head *head_next = head->next;
    head->next = head->prev;
    head->prev = head_next;
}

void merge(struct list_head *a, struct list_head *b)
{
    LIST_HEAD(dummy);
    INIT_LIST_HEAD(&dummy);
    while (!list_empty(a) && !list_empty(b)) {
        if (cmp(list_first_entry(a, element_t, list),
                list_first_entry(b, element_t, list)) < 0) {
            list_move_tail(a->next, &dummy);

        } else {
            list_move_tail(b->next, &dummy);
        }
    }

    if (!list_empty(a)) {
        list_splice_tail_init(a, &dummy);
    }

    if (!list_empty(b)) {
        list_splice_tail_init(b, &dummy);
    }

    list_splice(&dummy, a);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || head->prev == head->next) {
        return;
    }

    struct list_head *mid = q_mid(head);
    LIST_HEAD(new_head);
    INIT_LIST_HEAD(&new_head);
    list_cut_position(&new_head, head, mid);

    q_sort(head);
    q_sort(&new_head);
    merge(head, &new_head);
}
