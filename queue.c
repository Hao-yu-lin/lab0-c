#include <stdint.h>
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
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    struct list_head *tmp = l->next;

    while (tmp != l) {
        struct list_head *del = tmp;
        tmp = tmp->next;
        q_release_element(list_entry(del, element_t, list));
    }
    free(l);
}
/*
 * It will new an element for s, and allocate memory for s
 * Return NULL when allocation failed
 */
element_t *new_ele(char *s)
{
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return NULL;
    int str_size = strlen(s);
    new->value = malloc((str_size + 1) * sizeof(char));
    if (!new->value) {
        free(new);
        return NULL;
    }
    strncpy(new->value, s, str_size);
    *(new->value + str_size) = '\0';

    return new;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = new_ele(s);
    if (new) {
        list_add(&new->list, head);
        return true;
    }
    return false;
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
    if (!head)
        return false;

    element_t *new = new_ele(s);
    if (new) {
        list_add_tail(&new->list, head);
        return true;
    }
    return false;
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
    if (!head || list_empty(head))
        return NULL;
    element_t *target = list_entry(head->next, element_t, list);
    list_del(&target->list);

    if (bufsize > 0) {
        strncpy(sp, target->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }
    return target;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *target = list_entry(head->prev, element_t, list);
    list_del(&target->list);

    if (bufsize > 0) {
        strncpy(sp, target->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }
    return target;
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
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
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
    if (!head || !head->next)
        return false;
    struct list_head *fast = head->next;
    struct list_head *slow = head->next;

    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 * https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head *curr = head->next;
    struct list_head *next = curr->next;
    bool key = false;

    while (curr != head && next != head) {
        element_t *curr_entry = list_entry(curr, element_t, list);
        element_t *next_entry = list_entry(next, element_t, list);

        while (next != head && !strcmp(curr_entry->value, next_entry->value)) {
            list_del(next);
            q_release_element(next_entry);
            next = curr->next;
            next_entry = list_entry(next, element_t, list);
            key = true;
        }

        if (key) {
            list_del(curr);
            q_release_element(curr_entry);
            key = false;
        }

        curr = next;
        next = next->next;
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 * https://leetcode.com/problems/swap-nodes-in-pairs/
 */
void q_swap(struct list_head *head)
{
    if (!head || !head->next || list_empty(head))
        return;
    struct list_head *first = head->next;

    while (first != head && first->next != head) {
        struct list_head *second = first->next;
        list_del(first);
        list_add(first, second);
        first = first->next;
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
    if (!head || list_is_singular(head) || list_empty(head))
        return;
    struct list_head *curr = head;
    struct list_head *next = NULL;
    struct list_head *prev = head->prev;

    while (next != head) {
        next = curr->next;
        curr->next = prev;
        curr->prev = next;
        prev = curr;
        curr = next;
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
static struct list_head *merge(struct list_head *left, struct list_head *right)
{
    struct list_head *head = NULL;
    struct list_head **ptr = &head;
    while (left && right) {
        element_t *left_entry = list_entry(left, element_t, list);
        element_t *right_entry = list_entry(right, element_t, list);

        /*
         * Str1 < Str2 return < 0
         * Str1 > Str2 return > 0
         * Str1 = Str2 return = 0
         */
        if (strcmp(left_entry->value, right_entry->value) <= 0) {
            *ptr = left;
            left = left->next;
            // if(!left){
            //     *ptr = right;
            //     break;
            // }

        } else {
            *ptr = right;
            right = right->next;
            // if(!right){
            //     *ptr = left;
            //     break;
            // }
        }
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uint64_t) left | (uint64_t) right);
    return head;
}


static struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head->next;  // b8
    struct list_head *slow = head->next;  // b8
    while (fast && fast->next) {
        fast = fast->next->next;  // NULL
        slow = slow->next;        // 38
    }
    struct list_head *mid = slow;  // mid = 38
    slow->prev->next = NULL;       // b8->next = NULL

    struct list_head *left = mergesort(head);  // b8 38
    struct list_head *right = mergesort(mid);

    return merge(left, right);
}


void q_sort(struct list_head *head)
{
    if (!head || list_is_singular(head) || list_empty(head))
        return;
    head->prev->next = NULL;
    head->next = mergesort(head->next);

    struct list_head *curr = head;
    struct list_head *next = head->next;

    while (next) {
        next->prev = curr;
        curr = next;
        next = next->next;
    }
    curr->next = head;
    head->prev = curr;
}
