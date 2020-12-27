#ifndef indexing_h
#define indexing_h

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#include "storage.h"

#define MAX_OPEN_WALK_DESCRIPTORS 20
#define INDEXING_FINISHED_SIGNAL SIGUSR1
#define INDEX_NEVER_UPDATED 0
#define INDEXING_INTERUPTED 1
#define CONTINUE_INDEXING 0


typedef struct indexState {
    indexListNode_t* indexHandle; // PROTECTED BY indexMutex
    pthread_mutex_t* indexMutex;
    pthread_mutex_t* indexControlMutex;
    pthread_t indexingThread; // PROTECTED BY indexControlMutex
    pthread_t refreshingThread; // PROTECTED BY indexControlMutex
    bool inProgress; // PROTECTED BY indexControlMutex
    bool interputIndexing; // PROTECTED BY indexControlMutex
    time_t lastIndexTime; // PROTECTED BY indexControlMutex

    // NOT PROTECTED - after initizalziation use only in one thread at a time
    sigset_t* commonSigmask;
    char *targetDir;
    char *indexFile;
    int maxIndexLifetime; // 0 - disabled

    // NOT PROTECTED - buffer for indexing - used only by indexing thread
    indexListNode_t* bufferIndexHandle; 
} indexState_t;

void init_index_state(indexState_t *state, char *targetDir, char *indexFile, int t);
void init_index(indexState_t *state); // this function should be called only once per one state struct
void destroy_index_state(indexState_t *state);
void start_auto_index_refresh(indexState_t *state);
bool start_indexing(indexState_t *state); // return false if indexing is already in progress
void stop_auto_index_refresh(indexState_t* state); //stop and join - can be only called from one thread
void stop_index_thread(indexState_t* state, bool force); //stop and join - can be only called from one thread

#endif
