#ifndef KOTS_ARRAY_H
#define KOTS_ARRAY_H

typedef unsigned long int ULONG;

//array structure
typedef struct array_s
{
    void **values;
    ULONG length;
    ULONG capacity;
    ULONG realloc_size;
    void (*FreeItem)(void *item);
} array_t;

typedef int (*Array_CompareFunc)(void *item1, void *item2);
typedef int (*Array_FindFunc)(void *item, void *search);

//function prototypes
array_t *Array_Create(array_t *array, ULONG initial_capacity, ULONG realloc_size);
void Array_Delete(array_t *array);
void Array_FreeItem(void *);
void Array_CopyValues(array_t *from, array_t *to);
void Array_SetCapacity(array_t *array, ULONG capacity);
void Array_EnsureCapacity(array_t *array, ULONG capacity);
void Array_Truncate(array_t *array, ULONG capacity);
ULONG Array_IndexOf(array_t *array, void *value);
ULONG Array_FindIndex(array_t *array, void *search, Array_FindFunc find_func);
void *Array_Find(array_t *array, void *search, Array_FindFunc find_func);
void Array_Sort(array_t *array, Array_CompareFunc compare);
void Array_Shuffle(array_t *array, ULONG times);
void *Array_GetValueAt(array_t *array, ULONG index);
void Array_SetValueAt(array_t *array, ULONG index, void *value);
void Array_PushBack(array_t *array, void *value);
void Array_PushFront(array_t *array, void *value);
void Array_InsertAt(array_t *array, void *value, ULONG index);
void *Array_PopBack(array_t *array);
void *Array_PopFront(array_t *array);
void *Array_PopAt(array_t *array, ULONG index);
void Array_DeleteAt(array_t *array, ULONG index);
void Array_DeleteAll(array_t *array);
void Array_ClearAll(array_t *array);
void Array_Remove(array_t *array, void *value);

#undef ULONG

#endif
