#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#define _XOPEN_SOURCE 500
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>

#include "../lib/indexing.h"
#include "../lib/storage.h"
#include "../lib/file_type.h"
#include "../lib/shared.h"

static indexState_t* walk_state_handle;

static int walk_handler(const char *path, const struct stat *s, int type, struct FTW *f)
{
    indexObject_t obj;
    enum FileType fType = OTHER;
    bool willStop;
    // check if walk should be interupted
    WITH_MUTEX(willStop = walk_state_handle->interputIndexing, walk_state_handle->indexControlMutex);
    if (willStop) return INDEXING_INTERUPTED;

    switch (type){
        case FTW_DNR:
        case FTW_D: fType = DIR;
            break;
        case FTW_F: fType = recognize_file_type(path);
    }
    
    if (fType != OTHER) {
        if (strlen(path) > MAX_PATH) {
            WARN("File path too long detected - omitting!");
            return CONTINUE_INDEXING;
        }
        if (strlen(path + f->base) > MAX_NAME) {
            WARN("File name too long detected - omitting!");
            return CONTINUE_INDEXING;
        }
        strncpy(obj.path, path, MAX_PATH);
        strncpy(obj.name, path + f->base, MAX_NAME);
        obj.type = fType;
        obj.size = s->st_size;
        obj.uid = s->st_uid;
        insert_to_index_list(&walk_state_handle->bufferIndexHandle, &obj);
    }
    return CONTINUE_INDEXING;
}


static void *indexing_thread(void *args) {
    int indexRes;
    indexState_t *state = (indexState_t*) args;
    indexListNode_t *oldIndex = state->indexHandle;
    walk_state_handle = state;

    // this state fields are specific for this thread only so no protection (after init)
    state->bufferIndexHandle = NULL;
    if ((indexRes = nftw(state->targetDir, walk_handler, MAX_OPEN_WALK_DESCRIPTORS, 0)) == -1) ERR("nftw");
    if (indexRes != INDEXING_INTERUPTED) {
        save_index_list(state->bufferIndexHandle, state->indexFile);   
        // use new index as main index
        pthread_mutex_lock(state->indexMutex);
        state->indexHandle = state->bufferIndexHandle;
        state->bufferIndexHandle = NULL;
        state->lastIndexTime = time(NULL);
        pthread_mutex_unlock(state->indexMutex);
        // destory old index
        destroy_index_list(&oldIndex);

        printf("Indexing completed\n\n%s", CLI_PROMPT);
        fflush(stdout);
    } else printf("Indexing interupted\n");
    // the last thing is set the inProgress flag to false, after this operations THIS THREAD CANNOT ACCESS STATE
    WITH_MUTEX(state->inProgress = false, state->indexControlMutex);
    // inform auto refresh thread
    if (kill(getpid(), INDEXING_FINISHED_SIGNAL)) ERR("kill");
    
    return NULL;
}

static void *auto_refresh_thread(void* args) {
    indexState_t *data = (indexState_t*) args;
    // after init only used in this thread so no mutex
    time_t t, toSleep, maxLifetime = data->maxIndexLifetime;
    sigset_t *mask = data->commonSigmask;
    int sig;
    bool wasInProgress;

    while (true) {
        pthread_mutex_lock(data->indexControlMutex);
        wasInProgress = data->inProgress;
        t = data->lastIndexTime;
        pthread_mutex_unlock(data->indexControlMutex);

        if (wasInProgress) {
            if (sigwait(mask, &sig)) ERR("sigwait");
            if (sig != INDEXING_FINISHED_SIGNAL) ERR_PLAIN("Unexpected signal");
        } else {
            toSleep = t + maxLifetime + 1 - time(NULL); // we want more than 'maxLifetime' so add one second
            if (toSleep > 0) sleep(toSleep);
            else start_indexing(data);
        }
    }

    return NULL;
}

void init_index_state(indexState_t *state, char *targetDir, char *indexFile, int t) {
    state->indexMutex = malloc(sizeof(pthread_mutex_t));
    if (state->indexMutex == NULL) ERR("malloc");
    state->indexControlMutex = malloc(sizeof(pthread_mutex_t));
    if (state->indexControlMutex == NULL) ERR("malloc");
    if (pthread_mutex_init(state->indexMutex, NULL)) ERR("pthread_mutex_init");
    if (pthread_mutex_init(state->indexControlMutex, NULL)) ERR("pthread_mutex_init");
    state->commonSigmask = malloc(sizeof(sigset_t));
    if (state->commonSigmask == NULL) ERR("malloc");
    if (sigemptyset(state->commonSigmask)) ERR("sigemptyset");
    if (sigaddset(state->commonSigmask, INDEXING_FINISHED_SIGNAL)) ERR("sigaddset");
    state->maxIndexLifetime = t;
    state->bufferIndexHandle = NULL;
    state->targetDir = targetDir;
    state->lastIndexTime = INDEX_NEVER_UPDATED;
    state->indexFile = indexFile;
    state->inProgress = false;
    state->interputIndexing = false;
    state->indexHandle = NULL;
    state->indexingThread = NULL_THREAD;
    state->refreshingThread = NULL_THREAD;
}

// this function should be called only once per onERR("sigemptyset");e state struct
void init_index(indexState_t *state) {
    struct stat filestat;

    if (pthread_sigmask(SIG_BLOCK, state->commonSigmask, NULL)) ERR("pthread_sigmask");

    if (access(state->indexFile ,F_OK) == 0) {
        if(stat(state->indexFile, &filestat)) ERR("stat");
        load_index_list(&state->indexHandle, state->indexFile);
        state->lastIndexTime = filestat.st_mtim.tv_sec;
    }
    else start_indexing(state);
    
    if (state->maxIndexLifetime != 0) start_auto_index_refresh(state);
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
    free(state->commonSigmask);
    free(state->targetDir);
    free(state->indexFile);
}

void start_auto_index_refresh(indexState_t *state) {
    pthread_t tid;

    pthread_mutex_lock(state->indexControlMutex);
    if (!state->refreshingThread) {
        if (pthread_create(&tid, NULL, auto_refresh_thread, state)) ERR("pthread_create");
        state->refreshingThread = tid;
    }
    pthread_mutex_unlock(state->indexControlMutex);
}

static void start_indexing_cleanup_handler(void *args) {
    indexState_t *data = (indexState_t*) args;
    pthread_mutex_unlock(data->indexControlMutex);
}

// this function can also be called by auto_refresh_thread which can be cancelled so handle this situation
bool start_indexing(indexState_t *state) {
    pthread_t tid;
    bool result = false;

    pthread_mutex_lock(state->indexControlMutex);
    pthread_cleanup_push(start_indexing_cleanup_handler, state);
    // check if indexing currently running
    if (!state->inProgress) {
        tid = state->indexingThread;
        // if there is an unjoined thread hanging
        if (state->indexingThread) {
            // it is safe to join this thread while possesing the mutex, as the last thing which the indexing thread did with state,
            // was setting the inProgress flag to false
            if (pthread_join(state->indexingThread, NULL)) ERR("pthread_join"); // cancellation point here
        }
        // set inProgress flag after we are sure that cancellation has not occured
        state->inProgress = true;
        state->interputIndexing = false;
        if (pthread_create(&tid, NULL, indexing_thread, state)) ERR("pthread_create");
        state->indexingThread = tid;
        result = true;
    }
    pthread_cleanup_pop(1);
    return result;
}

// this function cannot be called from multiple threads - otherwise undefiend behaviour
void stop_auto_index_refresh(indexState_t* state) {
    pthread_t refreshTid;

    WITH_MUTEX(refreshTid = state->refreshingThread, state->indexControlMutex);
    if (refreshTid) {
        // we cannot join this thread in mutex
        if (pthread_cancel(refreshTid)) ERR("pthread_cancel");
        if (pthread_join(refreshTid, NULL)) ERR("pthread_join");
        WITH_MUTEX(state->refreshingThread = NULL_THREAD, state->indexControlMutex);
    }
}

// this function cannot be called from multiple threads - otherwise undefiend behaviour
void stop_index_thread(indexState_t* state, bool force) {
    pthread_t indexTid;

    pthread_mutex_lock(state->indexControlMutex);
    indexTid = state->indexingThread;
    if (force && indexTid) state->interputIndexing = true;
    pthread_mutex_unlock(state->indexControlMutex);
    
    if (indexTid) {
        // we cannot wait in mutex here
        if (pthread_join(indexTid, NULL)) ERR("pthread_join");
        WITH_MUTEX(state->indexingThread = NULL_THREAD, state->indexControlMutex);
    }
}
