#include <stdlib.h>
#include <string.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if ((list == NULL) || (element == NULL)) return;

    SortedListElement_t *list_ptr = list -> next;
    //error handling
    while ((list_ptr != list) && (strcmp(element -> key, list_ptr -> key) > 0))
        list_ptr = list_ptr -> next;
    element -> next = list_ptr;
    element -> prev = list_ptr -> prev;
    list_ptr -> prev -> next = element;
    list_ptr -> prev = element;
    return;
}

int SortedList_delete( SortedListElement_t *element)
{
    if (element -> key == NULL)
        return 1;
    else if ((element -> prev == NULL) || (element -> next == NULL))
        return 1;
    else if ((element -> prev -> next != element) || (element -> next -> prev != element))
        return 1;
    else
    {
        element -> prev -> next = element -> next;
        element -> next -> prev = element -> prev;
        // free(element);
        return 0;
    }
    
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t *list_ptr = list -> next;
    if (key == NULL) return NULL;
    while (list_ptr != list)
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
    if (list_ptr -> prev != list) return -1;
    while (list_ptr != list)
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