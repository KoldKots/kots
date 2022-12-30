#ifndef KOTS_MYSQL_HELPER_H
#define KOTS_MYSQL_HELPER_H

#ifdef WIN32
  #include <windows.h>
  #include <winsock.h>
  #pragma warning (disable: 4514 4786)
  #pragma warning( push, 3 )
#endif

#include <stdio.h>
#include <mysql.h>
#ifndef WIN32
  #include <unistd.h>
#endif


typedef struct st_mysql_iterator
{
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW currentrow;
    MYSQL_FIELD *fields;
    unsigned int numfields;
    unsigned int numrows;
} MYSQL_ITERATOR;

//creates an iterator to more easily manage results and row values
MYSQL_ITERATOR *mysql_iterator_create(MYSQL *);

//frees the memory used by the iterator
void mysql_iterator_free(MYSQL_ITERATOR *);

//clear all the results that have been unread and leve the result null
void mysql_iterator_clearresults(MYSQL_ITERATOR *);

//get the next result and return whether or not there are more
int mysql_iterator_nextresult(MYSQL_ITERATOR *);

//get the next row and return whether or not there are more
int mysql_iterator_nextresult(MYSQL_ITERATOR *);

//get the next row and return whether or not there are more
int mysql_iterator_nextrow(MYSQL_ITERATOR *iterator);

//gets a char pointer for the specified column name (NULL if not found)
void *mysql_iterator_getvalue(MYSQL_ITERATOR *iterator, const char *name);

//gets an integer value for the specified column name (0 if not found or null)
int mysql_iterator_getint(MYSQL_ITERATOR *, const char *);

//gets a character pointer for the specified column
void *mysql_iterator_getvalue_col(MYSQL_ITERATOR *, const int);

//gets an integer value for the specified column
int mysql_iterator_getint_col(MYSQL_ITERATOR *, const int);

//gets an char value for the specified column name ('\0' if not found or null)
char mysql_iterator_getchar(MYSQL_ITERATOR *iterator, const char *name);

//gets an char value for the specified column
char mysql_iterator_getchar_col(MYSQL_ITERATOR *iterator, const int col);

//determines if the current result set has the specified column
int mysql_iterator_hascolumn(MYSQL_ITERATOR *iterator, const char *name);

#endif
