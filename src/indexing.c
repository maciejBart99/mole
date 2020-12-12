#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define _XOPEN_SOURCE 700
#include <ftw.h>

#include "indexing.h"
#include "storage.h"
#include "file_type.h"

static indexState_t* walk_index_state;

static int walk_handler(const char *path, const struct stat *s, int type, struct FTW *f)
{
    indexObject_t obj;
    enum FileType fType;
    switch (type){
        case FTW_DNR:
        case FTW_D: fType = DIR;
            break;
        case FTW_F: fType = recognize_file_type(path);
            break;
        default : fType = OTHER;
    }
    
    if (fType != OTHER) {
        if (strlen(path) > MAX_PATH) {
            WARN("File path too long!");
            return 0;
        }
        if (strlen(path) > MAX_PATH) {
            WARN("File name too long!");
            return 0;
        }
        strncpy(obj.path, path, MAX_PATH);
        strncpy(obj.name, path + f->base, MAX_NAME);
        obj.type = fType;
        obj.size = s->st_size;
        obj.uid = s->st_uid;
        pthread_mutex_lock(walk_index_state->indexMutex);
        insert_to_index_list(&walk_index_state->indexHandle, &obj);
        pthread_mutex_unlock(walk_index_state->indexMutex);
    }
    
    return 0;
}

static void *indexing_thread(void *args) {
    indexState_t *state = (indexState_t*) args;
    walk_index_state = state;
    if (nftw(state->targetDir, walk_handler, MAX_OPEN_WALK_DESCRIPTORS, FTW_PHYS) != 0) ERR("nftw");
    
    pthread_mutex_lock(state->indexControlMutex);
    state->inProgress = false;
    state->lastIndexTime = time(NULL);
    pthread_mutex_unlock(state->indexControlMutex);
    
    return NULL;
}

static void *auto_refresh_thread(void* args) {
    indexState_t *data = (indexState_t*) args;
    time_t t;
    
    while (true) {
        sleep(1);
        pthread_mutex_lock(data->indexControlMutex);
        t = data->maxIndexLifetime + data->lastIndexTime;
        pthread_mutex_unlock(data->indexControlMutex);
        if (t > time(NULL)) start_indexing(data);
    }
    
    return NULL;
}


void init_index_state(indexState_t *state, const char *targetDir, const char *indexFile, int t) {
    state->indexMutex = malloc(sizeof(pthread_mutex_t));
    if (state->indexMutex == NULL) ERR("malloc");
    state->indexControlMutex = malloc(sizeof(pthread_mutex_t));
    if (state->indexControlMutex == NULL) ERR("malloc");
    if (pthread_mutex_init(state->indexMutex, NULL)) ERR("pthread_mutex_init");
    if (pthread_mutex_init(state->indexControlMutex, NULL)) ERR("pthread_mutex_init");
    state->maxIndexLifetime = t;
    state->targetDir = targetDir;
    state->indexFile = indexFile;
    state->inProgress = false;
    state->indexHandle = NULL;
    state->indexingThread = 0;
    state->refreshingThread = 0;
}

void destroy_index_state(indexState_t *state) {
    destroy_index_list(&state->indexHandle);
    if (state->indexControlMutex != NULL) {
        if (pthread_mutex_destroy(state->indexControlMutex)) ERR("pthread_mutex_destroy");
        free(state->indexControlMutex);
    }
    if (state->indexMutex != NULL) {
        if (pthread_mutex_destroy(state->indexMutex)) ERR("pthread_mutex_destroy");
        free(state->indexMutex);
    }
}

void start_auto_index_refresh(indexState_t *state) {
    pthread_t tid;
    
    pthread_mutex_lock(state->indexControlMutex);
    if (pthread_create(&tid, NULL, auto_refresh_thread, state)) ERR("pthread_create");
    state->refreshingThread = tid;
    pthread_mutex_unlock(state->indexControlMutex);
}

void start_indexing(indexState_t *state) {
    pthread_t tid;
    
    pthread_mutex_lock(state->indexControlMutex);
    
    if (!state->inProgress) {
        state->inProgress = true;
        if (pthread_create(&tid, NULL, auto_refresh_thread, state)) ERR("pthread_create");
        state->indexingThread = tid;
        pthread_mutex_unlock(state->indexControlMutex);
    } else pthread_mutex_unlock(state->indexControlMutex);
}

void stop_threads(indexState_t* state) {
    pthread_t indexTid, refreshTid;
    pthread_mutex_lock(state->indexControlMutex);
    indexTid = state->indexingThread;
    refreshTid = state->refreshingThread;
    pthread_mutex_unlock(state->indexControlMutex);
    
    if (refreshTid) {
        if (pthread_cancel(refreshTid)) ERR("pthread_cancel");
        if (pthread_join(refreshTid, NULL)) ERR("pthread_join");
        pthread_mutex_lock(state->indexControlMutex);
        state->refreshingThread = 0;
        pthread_mutex_unlock(state->indexControlMutex);
    }
    if (indexTid) {
        if (pthread_cancel(indexTid)) ERR("pthread_cancel");
        if (pthread_join(indexTid, NULL)) ERR("pthread_join");
        pthread_mutex_lock(state->indexControlMutex);
        state->indexingThread = 0;
        pthread_mutex_unlock(state->indexControlMutex);
    }
}
