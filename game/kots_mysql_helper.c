#include "kots_mysql_helper.h"

//creates an iterator to more easily manage results and row values
MYSQL_ITERATOR *mysql_iterator_create(MYSQL *mysql)
{
	MYSQL_ITERATOR *iterator = malloc(sizeof(MYSQL_ITERATOR));

	if (!iterator)
		return NULL;

	iterator->mysql = mysql;
	iterator->result = NULL;
	mysql_iterator_nextresult(iterator);
	return iterator;
}

//frees the memory used by the iterator
void mysql_iterator_free(MYSQL_ITERATOR *iterator)
{
	//if the iterator has a result stored free it
	if (iterator->result)
		mysql_iterator_clearresults(iterator);

	//free memory used by iterator
	iterator->mysql = NULL;
	free(iterator);
	iterator = NULL;
}

void mysql_iterator_clearresults(MYSQL_ITERATOR *iterator)
{
	if (iterator->result)
		mysql_free_result(iterator->result);

	while (mysql_more_results(iterator->mysql))
	{
		mysql_next_result(iterator->mysql);
		iterator->result = mysql_store_result(iterator->mysql);

		if (iterator->result)
			mysql_free_result(iterator->result);
	}

	iterator->result = NULL;
	iterator->currentrow = NULL;
	iterator->fields = NULL;
	iterator->numfields = 0;
	iterator->numrows = 0;
}

//get the next result and return whether or not there are more
int mysql_iterator_nextresult(MYSQL_ITERATOR *iterator)
{
	if (iterator->result)
	{
		mysql_free_result(iterator->result);
		iterator->result = NULL;
	}

	mysql_next_result(iterator->mysql);
	iterator->result = mysql_store_result(iterator->mysql);

	//if a result was obtained get the new field and row info
	if (iterator->result)
	{
		iterator->numfields = mysql_field_count(iterator->mysql);
		iterator->fields = mysql_fetch_fields(iterator->result);
		iterator->currentrow = mysql_fetch_row(iterator->result);
		iterator->numrows = (unsigned long)mysql_num_rows(iterator->result);
	}
	else
	{
		iterator->currentrow = NULL;
		iterator->fields = NULL;
		iterator->numfields = 0;
		iterator->numrows = 0;
	}

	return mysql_more_results(iterator->mysql);
}

//get the next row and return whether or not there are more
int mysql_iterator_nextrow(MYSQL_ITERATOR *iterator)
{
	if (!iterator->result)
		return 0;

	iterator->currentrow = mysql_fetch_row(iterator->result);
	if (iterator->currentrow == NULL)
		return 0;
	else
		return 1;
}

//determines if the current result set has the specified column
int mysql_iterator_hascolumn(MYSQL_ITERATOR *iterator, const char *name)
{
	unsigned int i;

	if (iterator->currentrow)
		for (i = 0; i < iterator->numfields; i++)
			if (strcmp(iterator->fields[i].name, name) == 0)
				return 1;

	return 0;
}

//gets a pointer for the specified column name (NULL if not found)
void *mysql_iterator_getvalue(MYSQL_ITERATOR *iterator, const char *name)
{
	unsigned int i;

	if (iterator->currentrow)
		for (i = 0; i < iterator->numfields; i++)
			if (strcmp(iterator->fields[i].name, name) == 0)
				return iterator->currentrow[i];

	return NULL;
}

//gets a character pointer for the specified column
void *mysql_iterator_getvalue_col(MYSQL_ITERATOR *iterator, const int col)
{
	return (iterator->currentrow ? iterator->currentrow[col] : NULL);
}

//gets an integer value for the specified column name (0 if not found or null)
int mysql_iterator_getint(MYSQL_ITERATOR *iterator, const char *name)
{
	char *value = mysql_iterator_getvalue(iterator, name);
	return (value ? atoi(value) : 0);
}

//gets an integer value for the specified column
int mysql_iterator_getint_col(MYSQL_ITERATOR *iterator, const int col)
{
	char *value = mysql_iterator_getvalue_col(iterator, col);
	return (value ? atoi(value) : 0);
}

//gets an char value for the specified column name ('\0' if not found or null)
char mysql_iterator_getchar(MYSQL_ITERATOR *iterator, const char *name)
{
	char *value = mysql_iterator_getvalue(iterator, name);
	return (value ? *value : '\0');
}

//gets an char value for the specified column
char mysql_iterator_getchar_col(MYSQL_ITERATOR *iterator, const int col)
{
	char *value = mysql_iterator_getvalue_col(iterator, col);
	return (value ? *value : '\0');
}
