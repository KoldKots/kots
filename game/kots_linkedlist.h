#ifndef KOTS_LINKEDLIST_H
#define KOTS_LINKEDLIST_H

#ifndef NULL
#define NULL 0
#endif

typedef struct listitem
{
	void *data;
	struct listitem *next;
	struct listitem *prev;
} listitem_t;

typedef struct list
{
	listitem_t *first;
	listitem_t *last;
	void (*freeItem)(listitem_t *item);
	int length;
} list_t;

//Create a linked list and specify the freeItem function
list_t *List_Create(void (*freeItem)(listitem_t *));

//Delete a linked list and free all items
void List_Delete(list_t *list);

//Get the listitem at the specified index
listitem_t *List_GetItem(list_t *list, int index);

//Add a new listitem to the end of the linked list
void List_AddItem(list_t *list, void *data);

//Removes and returns the first item in the linked list
//The item is not freed so you must do so yourself
listitem_t *List_Dequeue(list_t *list);

//Removes the specified item from the linked list
//The item is not freed so you must do it yourself
listitem_t *List_DequeueItem(list_t *list, listitem_t *item);

//Removes and returns the last item in the linked list
//The item is not freed so you must do so yourself
listitem_t *List_Pop(list_t *list);

//Remove the specified listitem and free it
void List_RemoveItem(list_t *list, listitem_t *item);

//Remove the listitem at the specified index and free it
void List_RemoveAt(list_t *list, int index);

//Clear the linked list of all items and free them
void List_Clear(list_t *list);

//Default free function just calls free on data pointer and the item
void List_FreeItemDefault(listitem_t *item);

//Alternate free function that only frees the item
void List_FreeItemOnly(listitem_t *item);

//Gets the length of the linked list
int List_GetLength(list_t *list);

#endif
