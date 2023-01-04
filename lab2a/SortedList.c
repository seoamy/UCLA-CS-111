// NAME: Amy Seo
// EMAIL: amyseo@g.ucla.edu
// ID: 505328863

#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    if (opt_yield & INSERT_YIELD) {
        sched_yield();
    }

    if (list == NULL || element == NULL || list->key != NULL) return;

    SortedListElement_t *pre = list;
    SortedListElement_t *cur = list->next;

    if(cur == NULL || cur->key == NULL) {
		list->next = element;
		element->prev = list;
		element->next = NULL;
        return;
    }

    while(cur != NULL && strcmp(cur->key, element->key) <= 0) {
        pre = cur;
        cur = cur->next;
    }

    pre->next = element;
    element->prev = pre;
    element->next = cur;

    if (cur != NULL) {
        cur->prev=element;
    }

    return;
}

int SortedList_delete(SortedListElement_t *element) {
    if (opt_yield & DELETE_YIELD) {
        sched_yield();
    }
    if (element == NULL) return 1;
    if(element->next != NULL) {
        //check that element's next prev is itself before deleting
        if (element->next->prev == element) {
            element->next->prev = element->prev;
        }
        // return 1 otherwise
        else {
            return 1;
        }
    }
  
    if (element->prev != NULL) {
        // check that element's prev next is itself before deleting
        if (element->prev->next == element) {
            element->prev->next = element->next;
        }
        // return 1 otherwise
        else {
            return 1;
        }
    }

    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    if(list == NULL) return NULL;
    SortedListElement_t *cur = list->next;
    while (cur != NULL) {
        if (opt_yield & LOOKUP_YIELD) {
            sched_yield();
        }
        if(strcmp(key, cur->key) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list) {
    if (list == NULL) return -1;
    int length = 0;

    // length does not include head
    SortedListElement_t *cur = list->next;
    while(cur != NULL) {
        if (opt_yield & LOOKUP_YIELD) {
            sched_yield();
        }
        cur=cur->next;
        length++;
    }
    return length;
}