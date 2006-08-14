/*
** 2002 January 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements a simple standalone program used to test whether
** or not the SQLite library is threadsafe.
**
** Testing the thread safety of SQLite is difficult because there are very
** few places in the code that are even potentially unsafe, and those
** places execute for very short periods of time.  So even if the library
** is compiled with its mutexes disabled, it is likely to work correctly
** in a multi-threaded program most of the time.  
*/
#include "sqlite.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
** Enable for tracing
*/
static int verbose = 0;

/*
** Come here to die.
*/
static void Exit(int rc){
  exit(rc);
}

extern char *sqlite_mprintf(const char *zFormat, ...);
extern char *sqlite_vmprintf(const char *zFormat, va_list);

/*
** When a lock occurs, yield.
*/
static int db_is_locked(void *NotUsed, const char *zNotUsed, int iNotUsed){
  /* sched_yield(); */
  if( verbose ) printf("BUSY %s\n", (char*)NotUsed);
  usleep(100);
  return 1;
}

/*
** Used to accumulate query results by db_query()
*/
struct QueryResult {
  const char *zFile;  /* Filename - used for error reporting */
  int nElem;          /* Number of used entries in azElem[] */
  int nAlloc;         /* Number of slots allocated for azElem[] */
  char **azElem;      /* The result of the query */
};

/*
** The callback function for db_query
*/
static int db_query_callback(
  void *pUser,     /* Pointer to the QueryResult structure */
  int nArg,        /* Number of columns in this result row */
  char **azArg,    /* Text of data in all columns */
  char **NotUsed   /* Names of the columns */
){
  struct QueryResult *pResult = (struct QueryResult*)pUser;
  int i;
  if( pResult->nElem + nArg >= pResult->nAlloc ){
    if( pResult->nAlloc==0 ){
      pResult->nAlloc = nArg+1;
    }else{
      pResult->nAlloc = pResult->nAlloc*2 + nArg + 1;
    }
    pResult->azElem = realloc( pResult->azElem, pResult->nAlloc*sizeof(char*));
    if( pResult->azElem==0 ){
      fprintf(stdout,"%s: malloc failed\n", pResult->zFile);
      return 1;
    }
  }
  if( azArg==0 ) return 0;
  for(i=0; i<nArg; i++){
    pResult->azElem[pResult->nElem++] =
        sqlite_mprintf("%s",azArg[i] ? azArg[i] : ""); 
  }
  return 0;
}

/*
** Execute a query against the database.  NULL values are returned
** as an empty string.  The list is terminated by a single NULL pointer.
*/
char **db_query(sqlite *db, const char *zFile, const char *zFormat, ...){
  char *zSql;
  int rc;
  char *zErrMsg = 0;
  va_list ap;
  struct QueryResult sResult;
  va_start(ap, zFormat);
  zSql = sqlite_vmprintf(zFormat, ap);
  va_end(ap);
  memset(&sResult, 0, sizeof(sResult));
  sResult.zFile = zFile;
  if( verbose ) printf("QUERY %s: %s\n", zFile, zSql);
  rc = sqlite_exec(db, zSql, db_query_callback, &sResult, &zErrMsg);
  if( rc==SQLITE_SCHEMA ){
    if( zErrMsg ) free(zErrMsg);
    rc = sqlite_exec(db, zSql, db_query_callback, &sResult, &zErrMsg);
  }
  if( verbose ) printf("DONE %s %s\n", zFile, zSql);
  if( zErrMsg ){
    fprintf(stdout,"%s: query failed: %s - %s\n", zFile, zSql, zErrMsg);
    free(zErrMsg);
    free(zSql);
    Exit(1);
  }
  sqlite_freemem(zSql);
  if( sResult.azElem==0 ){
    db_query_callback(&sResult, 0, 0, 0);
  }
  sResult.azElem[sResult.nElem] = 0;
  return sResult.azElem;
}

/*
** Execute an SQL statement.
*/
void db_execute(sqlite *db, const char *zFile, const char *zFormat, ...){
  char *zSql;
  int rc;
  char *zErrMsg = 0;
  va_list ap;
  va_start(ap, zFormat);
  zSql = sqlite_vmprintf(zFormat, ap);
  va_end(ap);
  if( verbose ) printf("EXEC %s: %s\n", zFile, zSql);
  rc = sqlite_exec(db, zSql, 0, 0, &zErrMsg);
  if( rc==SQLITE_SCHEMA ){
    if( zErrMsg ) free(zErrMsg);
    rc = sqlite_exec(db, zSql, 0, 0, &zErrMsg);
  }
  if( verbose ) printf("DONE %s: %s\n", zFile, zSql);
  if( zErrMsg ){
    fprintf(stdout,"%s: command failed: %s - %s\n", zFile, zSql, zErrMsg);
    free(zErrMsg);
    sqlite_freemem(zSql);
    Exit(1);
  }
  sqlite_freemem(zSql);
}

/*
** Free the results of a db_query() call.
*/
void db_query_free(char **az){
  int i;
  for(i=0; az[i]; i++){
    sqlite_freemem(az[i]);
  }
  free(az);
}

/*
** Check results
*/
void db_check(const char *zFile, const char *zMsg, char **az, ...){
  va_list ap;
  int i;
  char *z;
  va_start(ap, az);
  for(i=0; (z = va_arg(ap, char*))!=0; i++){
    if( az[i]==0 || strcmp(az[i],z)!=0 ){
      fprintf(stdout,"%s: %s: bad result in column %d: %s\n",
        zFile, zMsg, i+1, az[i]);
      db_query_free(az);
      Exit(1);
    }
  }
  va_end(ap);
  db_query_free(az);
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sig = PTHREAD_COND_INITIALIZER;
int thread_cnt = 0;

static void *worker_bee(void *pArg){
  const char *zFilename = (char*)pArg;
  char *azErr;
  int i, cnt;
  int t = atoi(zFilename);
  char **az;
  sqlite *db;

  pthread_mutex_lock(&lock);
  thread_cnt++;
  pthread_mutex_unlock(&lock);
  printf("%s: START\n", zFilename);
  fflush(stdout);
  for(cnt=0; cnt<10; cnt++){
    db = sqlite_open(&zFilename[2], 0, &azErr);
    if( db==0 ){
      fprintf(stdout,"%s: can't open\n", zFilename);
      Exit(1);
    }
    sqlite_busy_handler(db, db_is_locked, zFilename);
    db_execute(db, zFilename, "CREATE TABLE t%d(a,b,c);", t);
    for(i=1; i<=100; i++){
      db_execute(db, zFilename, "INSERT INTO t%d VALUES(%d,%d,%d);",
         t, i, i*2, i*i);
    }
    az = db_query(db, zFilename, "SELECT count(*) FROM t%d", t);
    db_check(zFilename, "tX size", az, "100", 0);  
    az = db_query(db, zFilename, "SELECT avg(b) FROM t%d", t);
    db_check(zFilename, "tX avg", az, "101", 0);  
    db_execute(db, zFilename, "DELETE FROM t%d WHERE a>50", t);
    az = db_query(db, zFilename, "SELECT avg(b) FROM t%d", t);
    db_check(zFilename, "tX avg2", az, "51", 0);
    for(i=1; i<=50; i++){
      char z1[30], z2[30];
      az = db_query(db, zFilename, "SELECT b, c FROM t%d WHERE a=%d", t, i);
      sprintf(z1, "%d", i*2);
      sprintf(z2, "%d", i*i);
      db_check(zFilename, "readback", az, z1, z2, 0);
    }
    db_execute(db, zFilename, "DROP TABLE t%d;", t);
    sqlite_close(db);
  }
  printf("%s: END\n", zFilename);
  /* unlink(zFilename); */
  fflush(stdout);
  pthread_mutex_lock(&lock);
  thread_cnt--;
  if( thread_cnt<=0 ){
    pthread_cond_signal(&sig);
  }
  pthread_mutex_unlock(&lock);
  return 0;
}

int main(int argc, char **argv){
  char *zFile;
  int i, n;
  pthread_t id;
  if( argc>2 && strcmp(argv[1], "-v")==0 ){
    verbose = 1;
    argc--;
    argv++;
  }
  if( argc<2 || (n=atoi(argv[1]))<1 ) n = 10;
  for(i=0; i<n; i++){
    char zBuf[200];
    sprintf(zBuf, "testdb-%d", (i+1)/2);
    unlink(zBuf);
  }
  for(i=0; i<n; i++){
    zFile = sqlite_mprintf("%d.testdb-%d", i%2+1, (i+2)/2);
    unlink(zFile);
    pthread_create(&id, 0, worker_bee, (void*)zFile);
    pthread_detach(id);
  }
  pthread_mutex_lock(&lock);
  while( thread_cnt>0 ){
    pthread_cond_wait(&sig, &lock);
  }
  pthread_mutex_unlock(&lock);
  for(i=0; i<n; i++){
    char zBuf[200];
    sprintf(zBuf, "testdb-%d", (i+1)/2);
    unlink(zBuf);
  }
  return 0;
}
