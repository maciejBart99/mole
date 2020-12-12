#ifndef indexing_h
#define indexing_h

#include <pthread.h>
#include <stdbool.h>

#include "storage.h"

typedef struct indexState {
    indexListNode_t* indexHandle;
    pthread_mutex_t* indexMutex;
    pthread_mutex_t* indexingThreadMutex;
    pthread_t indexingThread;
    pthread_t refreshingThread;
    bool inProgress;
    char *targetDir;
    char *storageLocation;
    int autoRefresh;
} indexState_t;

void init_index_state(indexState_t *state);
void destroy_index_state(indexState_t *state);
void start_auto_index_refresh(indexState_t *state);
void start_indexing(indexState_t *state);
void stop_threads(indexState_t* state);
void index_directory(indexState_t* state);

#endif
