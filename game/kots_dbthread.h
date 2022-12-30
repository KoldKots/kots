#ifndef KOTS_DBTHREAD_H
#define KOTS_DBTHREAD_H

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

#include <pthread.h>
#include <time.h>

//undefine constants quake 2 doesn't like
#undef hyper
#undef small
#undef dprintf

#include "kots_linkedlist.h"

#define KOTS_MAX_DBTHREADS      5
#define KOTS_QUERYBUFFER_SIZE   4096
#define KOTS_MYSQL_TIMEOUT      300
#define KOTS_MYSQL_CONNECT_TIMEOUT  30


typedef struct dbthread_s
{
  //thread based information
  pthread_t thread;
  struct jobitem_s *currentjob;
  int is_running;

  //mysql connection stuff
  MYSQL *mysql;
  struct timespec timeout;
  char query_buffer[KOTS_QUERYBUFFER_SIZE];
  int is_connected;

} dbthread_t;


typedef struct jobitem_s
{
  char *name;
  void (*function)(dbthread_t *thread, struct jobitem_s *job);
  int (*wait_check)(struct jobitem_s *job);
  int (*skip_check)(struct jobitem_s *job);
  void (*complete_callback)(struct jobitem_s *job);
  void *args;
  int result;
  char *last_error;
  time_t queue_time;
} jobitem_t;


dbthread_t *Kots_CreateDbThread();
void Kots_FreeDbThread(dbthread_t *t);
void Kots_StopDbThread(dbthread_t *t);
void Kots_AddDbThreadJob(jobitem_t *job);
void Kots_AddDbOutputJob(jobitem_t *job);
void Kots_InitDbThreads();
void Kots_FreeDbThreads();
void Kots_CloseDbConnections(dbthread_t *thread, jobitem_t *job);
listitem_t *Kots_GetNextDbOutput();
void Kots_FreeJobItem(listitem_t *item);
jobitem_t *Kots_CreateJobItem();
void Kots_WaitForDbThreads();
MYSQL *InitializeDbConnection();
MYSQL *ConnectToDb(MYSQL *mysql);
char *Kots_GetDbUser();
char *Kots_GetDbPass();
char *Kots_GetDbName();
char *Kots_GetDbHostName();
void Kots_SetDbUser(char *value);
void Kots_SetDbPass(char *value);
void Kots_SetDbName(char *value);
void Kots_SetDbHostName(char *value);
int Kots_WaitForSave(jobitem_t *this_job);
int Kots_CheckForNewerSaves(jobitem_t *this_job);

#endif
