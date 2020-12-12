#ifndef indexing_h
#define indexing_h

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#include "storage.h"

typedef struct indexState {
    indexListNode_t* indexHandle;
    pthread_mutex_t* indexMutex;
    pthread_mutex_t* indexControlMutex;
    pthread_t indexingThread;
    pthread_t refreshingThread;
    bool inProgress;
    const char *targetDir;
    const char *indexFile;
    int maxIndexLifetime;
    time_t lastIndexTime;
} indexState_t;

void init_index_state(indexState_t *state, const char *targetDir, const char *indexFile, int t);
void destroy_index_state(indexState_t *state);
void start_auto_index_refresh(indexState_t *state);
void start_indexing(indexState_t *state);
void stop_threads(indexState_t* state);

#endif
