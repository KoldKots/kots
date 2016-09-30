#include "kots_dbthread.h"
#include "kots_server.h"
#include "kots_utils.h"


extern dbthread_t *threads[KOTS_MAX_DBTHREADS];

//Static function prototypes
static void *RunDbThread(void *args);
static void ProcessQueue(dbthread_t *t);
static void OnConnectionTimeout(dbthread_t *t);
static void DbThread_ConnectToDb(dbthread_t *t);
static void CloseDbConnection(dbthread_t *t);

//functions from server
void MysqlSaveCharacter(dbthread_t *thread, jobitem_t *args);

//IMPORTANT INFO ON LOCKING
//ALWAYS LOCK THE INPUT LOCK FIRST BEFORE THE OUTPUT LOCK
//OTHERWISE YOU *WILL* END UP WITH DEADLOCKS

//global variables
static pthread_cond_t condition;
static pthread_mutex_t dbuser_lock;
static pthread_mutex_t input_lock;
static pthread_mutex_t output_lock;
static list_t *input = NULL;
static list_t *output = NULL;
char dbhostname[129];
char dbuser[65];
char dbpass[65];
char dbname[65];

void Kots_InitDbThreads()
{
	dbuser[0] = '\0';
	dbpass[0] = '\0';
	dbname[0] = '\0';

	//create input output lists
	input = List_Create(Kots_FreeJobItem);
	output = List_Create(Kots_FreeJobItem);

	//create thread locking structures
	pthread_mutex_init(&dbuser_lock, NULL);
	pthread_mutex_init(&input_lock, NULL);
	pthread_mutex_init(&output_lock, NULL);
	pthread_cond_init(&condition, NULL);

	mysql_library_init(0, NULL, NULL);
}

void Kots_FreeDbThreads()
{
	List_Delete(input);
	List_Delete(output);
	input = NULL;
	output = NULL;

	pthread_mutex_destroy(&dbuser_lock);
	pthread_mutex_destroy(&input_lock);
	pthread_mutex_destroy(&output_lock);
	pthread_cond_destroy(&condition);

	mysql_library_end();
}

dbthread_t *Kots_CreateDbThread()
{
	dbthread_t *t = malloc(sizeof(*t));

	//initialize some of the values
	t->mysql = NULL;
	t->is_running = 1;
	t->is_connected = 0;
	t->timeout.tv_nsec = 0;
	t->timeout.tv_sec = 0;
	t->query_buffer[0] = '\0';
	t->currentjob = NULL;

	//create the thread
	pthread_create(&t->thread, NULL, RunDbThread, t);

	return t; //return the thread
}

void Kots_FreeDbThread(dbthread_t *t)
{
	//make sure the mysql connection gets closed
	CloseDbConnection(t);
	free(t);
}

void Kots_WaitForDbThreads()
{
	pthread_mutex_lock(&input_lock);

	//wait until no more jobs are queued
	while (input->length > 0)
	{
		gi.dprintf("DB: %i jobs are still queued. Waiting for them to complete...\n", input->length);
		pthread_cond_wait(&condition, &input_lock);
	}
	
	pthread_mutex_unlock(&input_lock);

	pthread_mutex_lock(&output_lock);

	//wait for the jobs to actually finish
	while (true)
	{
		int i;
		int found = 0;
		struct timespec timeout;
		timeout.tv_sec = time(NULL) + 10;
		timeout.tv_nsec = 0;

		for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
		{
			//a job is still running so wait and then run the loop again
			if (threads[i]->currentjob)
			{
				found = 1;
				gi.dprintf("DB: One or more jobs is still in progress...\n");
				pthread_cond_timedwait(&condition, &output_lock, &timeout);
				break;
			}
		}

		//no jobs are running
		if (!found)
			break;
	}
	
	gi.dprintf("DB: All jobs are complete.\n");
	pthread_mutex_unlock(&output_lock);
}

//NOTE: Input queue is already locked during check
int Kots_WaitForSave(jobitem_t *this_job)
{
	int i, found = 0;
	jobitem_t *job;
	character_info_t *info, *this_info;

	this_info = (character_info_t *)this_job->args;

	//TODO: Change to use an actual lock and condition on the thread
	pthread_mutex_lock(&output_lock);

	for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
	{
		job = threads[i]->currentjob;

		if (job && job->function == MysqlSaveCharacter)
		{
			info = (character_info_t *)job->args;
			
			if (Q_stricmp(this_info->name, info->name) == 0)
			{
				found = 1;
				break;
			}
		}
	}
	
	pthread_mutex_unlock(&output_lock);

	//wait if we found one
	return found;
}

//NOTE: Input queue is already locked during check
//Also note that this_job is not in the queue at this point
int Kots_CheckForNewerSaves(jobitem_t *this_job)
{
	int found = 0;
	listitem_t *item;
	jobitem_t *job;
	character_info_t *info, *this_info;

	this_info = (character_info_t *)this_job->args;

	//TODO: Change to use an actual lock and condition on the thread
	pthread_mutex_lock(&output_lock);

	for (item = input->first; item != NULL; item = item->next)
	{
		job = (jobitem_t *)item->data;

		if (job && job->function == MysqlSaveCharacter)
		{
			info = (character_info_t *)job->args;
			
			if (Q_stricmp(this_info->name, info->name) == 0 && job->queue_time >= this_job->queue_time)
			{
				found = 1;
				break;
			}
		}
	}
	
	pthread_mutex_unlock(&output_lock);

	//skip if we found a newer save
	return found;
}

void Kots_StopDbThread(dbthread_t *t)
{
	pthread_mutex_lock(&input_lock);
	t->is_running = 0;
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&input_lock);
	pthread_join(t->thread, NULL);
}

void Kots_AddDbThreadJob(jobitem_t *job)
{
	//lock the input queue while we modify it
	pthread_mutex_lock(&input_lock);
	time(&job->queue_time);
	List_AddItem(input, job);
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&input_lock);
}

void Kots_AddDbOutputJob(jobitem_t *job)
{
	//lock the output queue while we modify it
	pthread_mutex_lock(&output_lock);
	List_AddItem(output, job);
	pthread_mutex_unlock(&output_lock);
}

listitem_t *Kots_GetNextDbOutput()
{
	listitem_t *item;

	//lock the output queue while we modify it
	pthread_mutex_lock(&output_lock);
	item = List_Dequeue(output);
	pthread_mutex_unlock(&output_lock);

	return item;
}

void Kots_FreeJobItem(listitem_t *item)
{
	jobitem_t *job = (jobitem_t *)item->data;

	if (job->args != NULL)
	{
		free(job->args);
		job->args = NULL;
	}

	if (job->last_error != NULL)
	{
		free(job->last_error);
		job->last_error = NULL;
	}

	free(job->name);
	free(job);
	free(item);
}

jobitem_t *Kots_CreateJobItem(char *name)
{
	jobitem_t *job = malloc(sizeof(*job));
	memset(job, 0, sizeof(*job));

	//set the name of the job
	job->name = strdup(name);
	
	return job;
}

void Kots_CloseDbConnections(dbthread_t *thread, jobitem_t *job)
{
	int i, found;

	//lock the input so we don't allow any more new jobs until current are complete
	pthread_mutex_lock(&input_lock);

	//lock the output while we check to ensure all jobs have completed
	pthread_mutex_lock(&output_lock);

	//now wait while it's the current job in a queue
	while (1)
	{
		found = 0;
		for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
		{
			if (threads[i]->currentjob && threads[i]->currentjob != job)
			{
				found = 1;
				break;
			}
		}

		//if found keep waiting
		if (found)
			pthread_cond_wait(&condition, &output_lock);
		else
			break; //otherwise break
	}

	//Now that all jobs have completed let's close all the connections
	for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
		OnConnectionTimeout(threads[i]);
	
	pthread_mutex_unlock(&output_lock);
	pthread_mutex_unlock(&input_lock);
}

static void *RunDbThread(void *args)
{
	dbthread_t *t = (dbthread_t *)args;

	t->mysql = InitializeDbConnection();

	while (1)
	{
		ProcessQueue(t);

		if (!t->is_running)
			break;
	}

	return NULL;
}

static void ProcessQueue(dbthread_t *t)
{
	int run_job = 0;
	int skip_job = 0;
	listitem_t *item = NULL;
	jobitem_t *job = NULL;

	//prevent more jobs from being added
	pthread_mutex_lock(&input_lock);
	item = List_Dequeue(input);

	//if we grabbed a job from the queue, then ensure we can execute it at this time
	if (item != NULL)
	{
		int wait, skip;

		//determine if we need to wait
		//we may need to wait for a prior save of the same char if we're saving again
		//or wait for the previous save when logging in, etc
		job = (jobitem_t *)item->data;
		wait = (job->wait_check && job->wait_check(job));
		skip = (job->skip_check && job->skip_check(job));

		if (wait || skip)
		{
			//ok so we will need to either wait or skip this job
			//check to see if we should skip it first
			//this may be the case if another save was queued
			//we obviously don't want to overwrite a newer save with an old one
			if (skip)
			{
				skip_job = 1;
				Kots_ServerAddDebugMessage("Skipped job...\n");

				//skipped the job, but still dequeued one
				//so if we're shutting down we need to know this
				pthread_cond_broadcast(&condition);
			}
			else
			{
				//we need to wait so put the job back in the queue
				List_AddItem(input, job);

				//Note that we don't want to signal at this time because
				//The job currently executing that we're waiting for
				//will pick up this job when it's finished
			}

			//free the old input list item
			free(item);
			item = NULL;
		}
		else
		{
			//it's safe to run the job now
			run_job = 1;
		}
	}

	//no jobs, lets wait for some
	if (!run_job)
	{
		//we got here because we skipped a job so let's go back immediately and try again
		if (!skip_job)
		{
			int wait_result;

			//set the db timeout interval if we're not continuing
			//from a previous wait
			if (!t->timeout.tv_sec)
			{
				t->timeout.tv_sec += time(NULL) + KOTS_MYSQL_TIMEOUT;
			}

			//only use timer if we're connected
			if (t->is_connected)
			{
				//wait for more jobs or the db timeout interval
				wait_result = pthread_cond_timedwait(&condition, &input_lock, &t->timeout);

				//if the wait timed out then it's time to close the db conn
				if (wait_result /* == ETIMEDOUT*/)
				{
					OnConnectionTimeout(t);
				}
			}

			else //we're not connected so no need to worry about timer
				pthread_cond_wait(&condition, &input_lock);
		}
	}

	else //no waiting, run this job now
	{
		//TODO: this is crappy we need a lock on the actual thread itself or something
		pthread_mutex_lock(&output_lock);

		//keep track of the current job
		t->currentjob = job;

		//we got a job so let everyone know of the change
		pthread_cond_broadcast(&condition);

		//refer to TODO above
		pthread_mutex_unlock(&output_lock);
	}

	//we're done checking for jobs for now
	pthread_mutex_unlock(&input_lock);

	//if there was a job
	if (item != NULL)
	{
		//ensure we have a db connection
		DbThread_ConnectToDb(t);

		//process this job
		job = (jobitem_t *)item->data;
		job->function(t, job);

		//prevent output jobs from being processed while we add this one
		pthread_mutex_lock(&output_lock);
		List_AddItem(output, job);

		//done with the current job
		t->currentjob = NULL;
		pthread_cond_broadcast(&condition);

		//allow other jobs now
		pthread_mutex_unlock(&output_lock);

		//free the input list item
		free(item);
		item = NULL;

		//reset the connection timeout timer
		t->timeout.tv_sec = 0;
	}
}

static void OnConnectionTimeout(dbthread_t *t)
{
	//if we're not connected we don't need to do anything
	if (t->is_connected)
	{
		//close the connection, but also reinitialize it for later
		CloseDbConnection(t);
		t->mysql = InitializeDbConnection();
	}
}

static void DbThread_ConnectToDb(dbthread_t *t)
{
	if (!t->is_connected)
	{
		if (ConnectToDb(t->mysql))
			t->is_connected = 1;
	}
}

static void CloseDbConnection(dbthread_t *t)
{
	if (t->mysql)
		mysql_close(t->mysql);

	t->mysql = NULL;
	t->is_connected = 0;
}

void SetMysqlOptions(MYSQL *mysql)
{
	char secure_auth = 1;
	char reconnect = 1;
	unsigned int connect_timeout = KOTS_MYSQL_CONNECT_TIMEOUT;

	mysql_options(mysql, MYSQL_SECURE_AUTH, &secure_auth);
	mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);
	mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&connect_timeout);
	mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, (char *)&connect_timeout);
	mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, (char *)&connect_timeout);
}

MYSQL *InitializeDbConnection()
{
	MYSQL *mysql = NULL;

	if (!(mysql = mysql_init(NULL)))
	{
		printf("Failed to initialize mysql library. Unable to start server.");
		exit(1);
		return NULL;
	}

	SetMysqlOptions(mysql);

	return mysql;
}

char *Kots_GetDbUser()
{
	char *user;
	pthread_mutex_lock(&dbuser_lock);

	if (dbuser[0] == '\0')
		user = va("%s", "default");
	else
		user = va("%s", dbuser);

	pthread_mutex_unlock(&dbuser_lock);
	return user;
}

char *Kots_GetDbPass()
{
	char *pass;
	pthread_mutex_lock(&dbuser_lock);

	if (dbuser[0] == '\0')
		pass = va("%s", "default");
	else
		pass = va("%s", dbpass);

	pthread_mutex_unlock(&dbuser_lock);
	return pass;
}

char *Kots_GetDbName()
{
	char *name;
	pthread_mutex_lock(&dbuser_lock);

	if (dbuser[0] == '\0')
		name = va("%s", "default");
	else
		name = va("%s", dbname);

	pthread_mutex_unlock(&dbuser_lock);
	return name;
}

char *Kots_GetDbHostName()
{
	char *name;
	pthread_mutex_lock(&dbuser_lock);

	if (dbuser[0] == '\0')
		name = va("%s", "default");
	else
		name = va("%s", dbhostname);

	pthread_mutex_unlock(&dbuser_lock);
	return name;
}

void Kots_SetDbUser(char *value)
{
	pthread_mutex_lock(&dbuser_lock);
	Kots_strncpy(dbuser, value, 65);
	pthread_mutex_unlock(&dbuser_lock);
}

void Kots_SetDbPass(char *value)
{
	pthread_mutex_lock(&dbuser_lock);
	Kots_strncpy(dbpass, value, 65);
	pthread_mutex_unlock(&dbuser_lock);
}

void Kots_SetDbName(char *value)
{
	if (Q_stricmp(dbname, value) != 0)
	{
		pthread_mutex_lock(&dbuser_lock);
		Kots_strncpy(dbname, value, 65);
		pthread_mutex_unlock(&dbuser_lock);

		Kots_ServerDbNameChanged();
	}
}

void Kots_SetDbHostName(char *value)
{
	if (Q_stricmp(dbhostname, value) != 0)
	{
		pthread_mutex_lock(&dbuser_lock);
		Kots_strncpy(dbhostname, value, 65);
		pthread_mutex_unlock(&dbuser_lock);

		Kots_ServerDbNameChanged();
	}
}

//WARNING:
//This method is pretty dangerous and should only be used when debugging problems
//Should probably only be used during deadlock situations
void Kots_DebugDbInfo(edict_t *ent)
{
	int i;
	int input_locked = 0;
	int output_locked = 0;
	dbthread_t *thread = NULL;
	jobitem_t *job = NULL;

	//Print start time as a reference
	gi.cprintf(ent, PRINT_HIGH, "Database Debug Information - Time: %i\n", time(NULL));

	//Test input lock
	if (!pthread_mutex_trylock(&input_lock))
	{
		input_locked = 1;
		gi.cprintf(ent, PRINT_HIGH, "Input Lock Test: Successful\n");
	}
	else
		gi.cprintf(ent, PRINT_HIGH, "Input Lock Test: Failed\n");

	//Test output lock
	if (!pthread_mutex_trylock(&output_lock))
	{
		output_locked = 1;
		gi.cprintf(ent, PRINT_HIGH, "Input Lock Test: Successful\n");
	}
	else
		gi.cprintf(ent, PRINT_HIGH, "Output Lock Test: Failed\n");

	//Print input and output summary
	gi.cprintf(ent, PRINT_HIGH, "Input Queue: %i jobs\n", input->length);
	gi.cprintf(ent, PRINT_HIGH, "Output Queue: %i jobs\n", output->length);

	//Print thread information
	for (i = 0; i < KOTS_MAX_DBTHREADS; i++)
	{
		thread = threads[i];
		job = thread->currentjob;

		gi.cprintf(ent, PRINT_HIGH, "Thread %i Information:\n", i);
		gi.cprintf(ent, PRINT_HIGH, "  IsRunning: %i\n", thread->is_running);
		gi.cprintf(ent, PRINT_HIGH, "  IsConnected: %i\n", thread->is_connected);
		gi.cprintf(ent, PRINT_HIGH, "  Timeout: %i\n", thread->timeout);
		gi.cprintf(ent, PRINT_HIGH, "  Current Job Info:\n");
		if (job != NULL)
		{
			gi.cprintf(ent, PRINT_HIGH, "    Job Name: %s\n", job->name);
			gi.cprintf(ent, PRINT_HIGH, "    Function: %p\n", job->function);
			gi.cprintf(ent, PRINT_HIGH, "    Callback: %p\n", job->complete_callback);
			gi.cprintf(ent, PRINT_HIGH, "    WaitCheck: %p\n", job->wait_check);
			gi.cprintf(ent, PRINT_HIGH, "    Args: %p\n", job->args);
			gi.cprintf(ent, PRINT_HIGH, "    Result: %i\n", job->result);
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "    No job in progress.\n");

		gi.cprintf(ent, PRINT_HIGH, "  LastError: %s\n", mysql_error(thread->mysql));
		gi.cprintf(ent, PRINT_HIGH, "  QueryBuffer: %s\n\n", thread->query_buffer);
	}

	//Cleanup locks
	if (input_locked)
		pthread_mutex_unlock(&input_lock);
	if (output_locked)
		pthread_mutex_unlock(&output_lock);
}

MYSQL *ConnectToDb(MYSQL *mysql)
{
	char host_name[129];
	char database_name[65];
	char username[65];
	char pass[65];
	int i;

	//lock the dbuser info while until we create local copies of it
	pthread_mutex_lock(&dbuser_lock);

	if (dbname[0] != '\0')
		strcpy(&database_name[0], dbname);
	else
		strcpy(&database_name[0], MYSQL_DB);
	
	if (dbuser[0] != '\0')
	{
		strcpy(&username[0], dbuser);
		strcpy(&pass[0], dbpass);
	}
	else
	{
		strcpy(&username[0], MYSQL_USER);
		strcpy(&pass[0], MYSQL_PASS);
	}
	
	if (dbhostname[0] != '\0')
		strcpy(&host_name[0], dbhostname);
	else
		strcpy(&host_name[0], MYSQL_HOST);

	pthread_mutex_unlock(&dbuser_lock);

	for (i = 0; i < 3; i++)
	{
		//if we connect successfuly then go ahead and return now
		if (mysql_real_connect(mysql, host_name, username, pass, database_name, MYSQL_PORT, NULL, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS))
		{
			//Some options get reset after connection so ensure they're correct
			SetMysqlOptions(mysql);

			return mysql;
		}
	}

	//couldn't connect
	return NULL;
}
