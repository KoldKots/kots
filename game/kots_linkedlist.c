#include <stdlib.h>
#include "kots_linkedlist.h"

list_t *List_Create(void (*freeItem)(listitem_t *))
{
    list_t *list = malloc(sizeof(list_t));
    list->first = NULL;
    list->last = NULL;
    list->length = 0;
    list->freeItem = freeItem;
    return list;
}

void List_Delete(list_t *list)
{
    if (list)
    {
        List_Clear(list);
        free(list);
    }
}

void List_AddItem(list_t *list, void *data)
{
    if (!list->first)
    {
        list->first = malloc(sizeof(listitem_t));
        list->last = list->first;
        list->first->data = data;
        list->first->prev = NULL;
        list->first->next = NULL;
    }
    else
    {
        list->last->next = malloc(sizeof(listitem_t));
        list->last->next->prev = list->last;
        list->last = list->last->next;
        list->last->data = data;
        list->last->next = NULL;
    }

    ++list->length;
}

listitem_t *List_GetItem(list_t *list, int index)
{
    listitem_t *item;

    if (index == 0)
        return list->first;
    else if (index == list->length - 1)
        return list->last;
    else if (index < 0 || index >= list->length)
        return NULL;
    else
    {
        item = list->first;
        while (index != 0 && item)
        {
            item = item->next;
            --index;
        }

        return item;
    }
}

//Used internally to remove an item without freeing it (dangerous)
void List_RemoveItemInternal(list_t *list, listitem_t *item)
{
    if (item->prev)
        item->prev->next = item->next;

    if (item->next)
        item->next->prev = item->prev;

    if (list->first == item)
        list->first = item->next;

    if (list->last == item)
        list->last = item->prev;

    --list->length;
}

void List_RemoveItem(list_t *list, listitem_t *item)
{
    List_RemoveItemInternal(list, item);
    list->freeItem(item);
}

listitem_t *List_Dequeue(list_t *list)
{
    return List_DequeueItem(list, list->first);
}

listitem_t *List_DequeueItem(list_t *list, listitem_t *item)
{
    if (item)
    {
        List_RemoveItemInternal(list, item);
        item->next = NULL;
        item->prev = NULL;
    }

    return item;
}

listitem_t *List_Pop(list_t *list)
{
    listitem_t *item = list->last;

    if (item)
    {
        List_RemoveItemInternal(list, item);
        item->next = NULL;
        item->prev = NULL;
    }

    return item;
}

void List_RemoveAt(list_t *list, int index)
{
    listitem_t *item = List_GetItem(list, index);

    if (item)
        List_RemoveItem(list, item);
}

void List_Clear(list_t *list)
{
    listitem_t *item = list->first;
    listitem_t *next = item;

    while (item)
    {
        next = item->next;
        list->freeItem(item);
        item = next;
    }

    list->first = NULL;
    list->last = NULL;
    list->length = 0;
}

int List_GetLength(list_t *list)
{
    return list->length;
}

void List_FreeItemDefault(listitem_t *item)
{
    free(item->data);
    free(item);
}

void List_FreeItemOnly(listitem_t *item)
{
    free(item);
}
