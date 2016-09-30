#include <stdio.h>
#include <stdlib.h>
#include "kots_array.h"

#ifndef ULONG
#define ULONG unsigned long int
#endif

array_t *Array_Create(array_t *array, ULONG initial_capacity, ULONG realloc_size)
{
	if (!array)
		array = malloc(sizeof(*array));

	array->values = NULL;
	array->length = 0;
	array->capacity = 0;
	array->realloc_size = realloc_size;
	array->FreeItem = Array_FreeItem;
	Array_SetCapacity(array, initial_capacity);

	return array;
}

void Array_Delete(array_t *array)
{
	if (array)
	{
		Array_DeleteAll(array);
		free(array->values);
		array->values = NULL;
		array->capacity = 0;
		free(array);
	}
}

void Array_FreeItem(void *item)
{
	free(item);
}

void Array_CopyValues(array_t *from, array_t *to)
{
	ULONG i;

	//ensure that the destination can hold all of the source items
	Array_EnsureCapacity(to, from->length);

	//directly copy values to the destination array without freeing them
	//we assume any cleanup is done prior to calling this
	for (i = 0; i < from->length; i++)
		to->values[i] = from->values[i];

	to->length = from->length;
}

void Array_SetCapacity(array_t *array, ULONG capacity)
{
	ULONG i;

	if (array->capacity > capacity)
	{
		fprintf(stderr, "Error trying to set capacity lower than current capacity (OLD: %d, NEW: %d).", array->capacity, capacity);
		exit(1);
	}
	else if (array->capacity == capacity && capacity > 0)
		return;

	if (capacity > 0)
		array->values = realloc(array->values, sizeof(void *) * capacity);
	else
		array->values = NULL;

	//set new values in array to NULL
	for (i = array->capacity; i < capacity; i++)
		array->values[i] = NULL;
	
	array->capacity = capacity;
}

void Array_CheckRealloc(array_t *array)
{
	if (array->length >= array->capacity)
	{
		if (array->realloc_size <= 0)
			array->realloc_size = array->capacity;

		Array_SetCapacity(array, array->capacity + array->realloc_size);
	}
}

void Array_EnsureCapacity(array_t *array, ULONG capacity)
{
	if (array->capacity < capacity)
		Array_SetCapacity(array, capacity);
}

void Array_Truncate(array_t *array, ULONG capacity)
{
	ULONG i;

	if (array->capacity <= capacity)
		return;

	if (array->FreeItem)
	{
		for (i = capacity; i < array->length; i++)
			array->FreeItem(array->values[i]);
	}

	//reset capacity to ensure the new capacity is enforced with no errors
	array->capacity = 0;
	Array_SetCapacity(array, capacity);
}

ULONG Array_IndexOf(array_t *array, void *value)
{
	//no need to do anything if the array is empty
	if (array->length > 0)
	{
		ULONG i;

		//loop through each item to find it's index
		for (i = 0; i < array->length; i++)
			if (array->values[i] == value)
				return i;

	}

	return -1; //index not found
}

ULONG Array_FindIndex(array_t *array, void *search, Array_FindFunc find_func)
{
	//no need to do anything if the array is empty
	if (array->length > 0)
	{
		ULONG i;

		//loop through each item to find it's index
		for (i = 0; i < array->length; i++)
			if (find_func(array->values[i], search) == 0)
				return i;

	}

	return -1; //index not found
}

void *Array_Find(array_t *array, void *search, Array_FindFunc find_func)
{
	ULONG index = Array_FindIndex(array, search, find_func);

	if (index > 0 && index < array->length)
		return Array_GetValueAt(array, index);
	else
		return NULL;
}


void Array_Sort(array_t *array, Array_CompareFunc compare)
{
	//no need to do anything if the array is empty
	if (array->length > 0)
	{
		ULONG i, j;
		void *temp;

		//loop through each item to find it's correct position
		for (i = 0; i < array->length; i++)
		{
			//find an item lower than the current item
			for (j = i + 1; j < array->length; j++)
			{
				//if item at j is lower than item at i swap them
				if (compare(array->values[i], array->values[j]) > 0)
				{
					//store the temp value and randomly pick an index to swap with
					temp = array->values[j];
					array->values[j] = array->values[i];
					array->values[i] = temp;
				}
			}
		}
	}
}

void Array_Shuffle(array_t *array, ULONG times)
{
	//no need to do anything if the array is empty
	if (array->length > 0)
	{
		ULONG i, j, index;
		void *temp;

		//shuffle all items the specified number of times
		for (i = 0; i < times; i++)
		{
			//shuffle each item in the array
			for (j = 0; j < array->length; j++)
			{
				//store the temp value and randomly pick an index to swap with
				temp = array->values[j];
				index = rand() % array->length;

				//sweap the two values
				array->values[j] = array->values[index];
				array->values[index] = temp;
			}
		}
	}
}

void *Array_GetValueAt(array_t *array, ULONG index)
{
	if (index < 0 || index >= array->length)
	{
		fprintf(stderr, "Error accessing array index outside of bounds (LENGTH: %d, INDEX: %d).", array->length, index);
		exit(1);
	}

	return array->values[index];
}

void Array_SetValueAt(array_t *array, ULONG index, void *value)
{
	if (index < 0 || index >= array->length)
	{
		fprintf(stderr, "Error accessing array index outside of bounds (LENGTH: %d, INDEX: %d).", array->length, index);
		exit(1);
	}

	if (array->FreeItem)
		array->FreeItem(array->values[index]);

	array->values[index] = value;
}

void Array_PushBack(array_t *array, void *value)
{
	Array_CheckRealloc(array);
	array->values[array->length] = value;
	array->length++;
}

void Array_PushFront(array_t *array, void *value)
{
	Array_InsertAt(array, value, 0);
}

void Array_InsertAt(array_t *array, void *value, ULONG index)
{
	ULONG i, j;

	if (index < 0 || index > array->length)
	{
		fprintf(stderr, "Error accessing array index outside of bounds (LENGTH: %d, INDEX: %d).", array->length, index);
		exit(1);
	}

	//push the last value in the array back another element
	Array_PushBack(array, array->values[array->length - 1]);

	//move all the elements from the index up one
	if (array->length >= 2)
	{
		i = array->length;
		j = i - 1;

		do
		{
			array->values[--i] = array->values[--j];
		} while (j != index);
	}

	//now insert the new value
	array->values[index] = value;
}

void *Array_PopBack(array_t *array)
{
	if (!array->length)
		return NULL;

	array->length--;
	return array->values[array->length];
}

void *Array_PopFront(array_t *array)
{
	if (!array->length)
		return NULL;
	else
		return Array_PopAt(array, 0);
}

void *Array_PopAt(array_t *array, ULONG index)
{
	ULONG i, j;
	void *value = NULL;

	if (index < 0 || index >= array->length)
	{
		fprintf(stderr, "Error accessing array index outside of bounds (LENGTH: %d, INDEX: %d).", array->length, index);
		exit(1);
	}

	value = array->values[index];
	for (i = index, j = i + 1; j < array->length; i++, j++)
		array->values[i] = array->values[j];

	array->length--;
	array->values[array->length] = NULL;

	return value;
}

void Array_Remove(array_t *array, void *value)
{
	ULONG index = Array_IndexOf(array, value);
	Array_PopAt(array, index);
}

void Array_DeleteAt(array_t *array, ULONG index)
{
	void *value = Array_PopAt(array, index);
	
	if (array->FreeItem)
		array->FreeItem(value);
}

void Array_DeleteAll(array_t *array)
{
	ULONG i;

	if (!array->FreeItem)
	{
		Array_ClearAll(array);
		return;
	}

	for (i = 0; i < array->length; i++)
	{
		array->FreeItem(array->values[i]);
		array->values[i] = NULL;
	}
	
	array->length = 0;
}

void Array_ClearAll(array_t *array)
{
	ULONG i;

	for (i = 0; i < array->length; i++)
		array->values[i] = NULL;

	array->length = 0;
}
