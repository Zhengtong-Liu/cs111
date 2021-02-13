// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdlib.h>
#include <string.h>
#include "SortedList.h"
#include <sched.h>

int opt_yield = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    // error checking
    if ((list == NULL) || (element == NULL)) return;

    // start from the first element, find place to insert
    // if the key of the element is greater than the current one, go to next one
    SortedListElement_t *list_ptr = list -> next;
    while ((list_ptr != list) && (strcmp(element -> key, list_ptr -> key) > 0))
        list_ptr = list_ptr -> next;
    // now list_ptr should point to the first element in sorted list whose key
    // is greater than the key of the element to insert
    element -> prev = list_ptr -> prev;
    element -> next = list_ptr;
    // this is in a critical section
    if (opt_yield & INSERT_YIELD) {
        sched_yield();
    }
    list_ptr -> prev -> next = element;
    list_ptr -> prev = element;
    return;
}

int SortedList_delete( SortedListElement_t *element)
{
    // error checking, make sure that the sorted list is not corrupted
    if (element -> key == NULL)
        return 1;
    else if ((element -> prev == NULL) || (element -> next == NULL))
        return 1;
    else if ((element -> prev -> next != element) || (element -> next -> prev != element))
        return 1;
    else
    {
        element -> prev -> next = element -> next;
        // this is in a critical section
        if (opt_yield & DELETE_YIELD) {
            sched_yield();
        }
        element -> next -> prev = element -> prev;
        return 0;
    }
    
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t *list_ptr = list -> next;

    // this is in a critical section
    if (opt_yield & LOOKUP_YIELD) {
        sched_yield();
    }

    // go to next until finding the key
    while ((list_ptr != NULL) && (list_ptr != list))
    {
        if (strcmp(key, list_ptr -> key) == 0)
            return list_ptr;
        list_ptr = list_ptr -> next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list)
{
    int len = 0;
    SortedListElement_t* list_ptr = list -> next;
    // error checking
    if (list_ptr -> prev != list) return -1;
    // this is in a critical section
    if (opt_yield & LOOKUP_YIELD) {
        sched_yield();
    }
    // check the sorted list is not corrupted and add to length each time
    while ((list_ptr != NULL) && (list_ptr != list))
    {
        if ((list_ptr -> next == NULL) || (list_ptr -> prev == NULL))
            return -1;
        else if ((list_ptr -> next -> prev != list_ptr) || (list_ptr -> prev -> next != list_ptr))
            return -1;
        else
        {
            len += 1;
            list_ptr = list_ptr -> next;
        }
    }
    return len;
}