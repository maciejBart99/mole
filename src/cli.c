#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "../lib/cli.h"
#include "../lib/indexing.h"
#include "../lib/queries.h"
#include "../lib/shared.h"


static void usage(const char *name) {
    fprintf(stderr, "Usage: %s [-d] [-f] [-t]\n", name);
    fprintf(stderr, "-d - target directory (%s env. if not present)\n", DIR_ENV_PATH);
    fprintf(stderr, "-f - path to index file (%s env. or ~/%s if not present)\n", INDEX_ENV_PATH, DEFAULT_INDEX_FILE);
    fprintf(stderr, "-t - max index lifetime from range [%d,%d] (disabled by default)\n", MIN_ARG_T, MAX_ARG_T);
    exit(EXIT_FAILURE);
}

static FILE* get_list_output(const indexListNode_t* head) {
    FILE* out;

    char* pager = getenv(PAGINATION_ENV_NAME);
    if (is_index_list_longer_than(head, MAX_RECORDS_WITHOUT_PAGINATION) && pager != NULL) {
        if((out = popen(pager, "w")) == NULL) ERR("popen");
    } else out = stdout;

    return out;
}

static void close_list_output(FILE* output) {
    if (output != stdout) {
        errno = 0;
        if (pclose(output) < 0) {
            // the -1 can be retuned as pager exit status code
            if (errno == 0) ERR_PLAIN("Pager problem has occured");
            else ERR("pclose");
        } 
    }
}

static void display_list_output(const indexListNode_t* head) {
    // +1 to make place for \0 character
    char buffer[MAX_TYPE_STR + 1];
    FILE* out;

    if (head == NULL) {
        printf("--Nothing to show--\n");
        return;
    }

    out = get_list_output(head);
    
    fprintf(out ,"%-*s %-*s %-*s\n", MAX_PATH, "Path", MAX_TYPE_STR, "Type", MAX_SIZE_STR_LEN, "Size");
    while (head) {
        get_string_file_type(head->data->type, buffer);
        fprintf(out, "%-*s %-*s %-*zu\n", MAX_PATH, head->data->path, MAX_TYPE_STR, buffer, MAX_SIZE_STR_LEN, head->data->size);
        head = head->next;
    }

    close_list_output(out);
}

static void display_count_output(fileTypesCount_t counts) {
    printf("%-*s %-10s\n", MAX_TYPE_STR, "Type", "Count");
    printf("%-*s %-10zu\n", MAX_TYPE_STR, "PNG", counts.png);
    printf("%-*s %-10zu\n", MAX_TYPE_STR, "JPEG", counts.jpeg);
    printf("%-*s %-10zu\n", MAX_TYPE_STR, "DIR", counts.dir);
    printf("%-*s %-10zu\n", MAX_TYPE_STR, "GZIP", counts.gzip);
    printf("%-*s %-10zu\n", MAX_TYPE_STR, "ZIP", counts.zip);
}

static void execute_exit(indexState_t *state, bool force) {
    stop_auto_index_refresh(state);
    stop_index_thread(state, force);
    destroy_index_state(state);
    printf("\n");
    exit(EXIT_SUCCESS);
}

static void execute_count(const indexState_t *state) {
    fileTypesCount_t counts;
    WITH_MUTEX(counts = get_index_count(state->indexHandle), state->indexMutex);
    display_count_output(counts);
}

static void execute_index(indexState_t *state) {
    if (!start_indexing(state)) WARN("Indexing already in progress");
}

static void execute_largerthan(const indexState_t *state, long size) {
    indexListNode_t *listRepsonseHead;
    WITH_MUTEX(listRepsonseHead = get_larger_than(state->indexHandle, size), state->indexMutex);
    display_list_output(listRepsonseHead);
}

static void execute_namepart(const indexState_t *state, const char *search) {
    indexListNode_t *listRepsonseHead;
    WITH_MUTEX(listRepsonseHead = get_namepart(state->indexHandle, search), state->indexMutex);
    display_list_output(listRepsonseHead);
}

static void execute_owner(const indexState_t *state, long uid) {
    indexListNode_t *listRepsonseHead;
    WITH_MUTEX(listRepsonseHead = get_with_owner(state->indexHandle, (uid_t) uid), state->indexMutex);
    display_list_output(listRepsonseHead);
}

static void get_default_index_path(char *buffer) {
    char* home = getenv(HOME_ENV_NAME);
    if (home == NULL) ERR_PLAIN("Home env. variable not set");
    sprintf(buffer, "%s/%s", home, DEFAULT_INDEX_FILE);
}

void parse_arguments(int argc, char** argv, programArgs_t* dest) {
    int c;
    char *buffer;

    dest->diPath = NULL;
    dest->indexPath = NULL;
    dest->t = 0;
    while((c = getopt(argc, argv, "d:f:t:")) != -1) {
        switch (c) {
            case 'd':
                dest->diPath = strdup(optarg);
                break;
            case 'f':
                dest->indexPath = strdup(optarg);
                break;
            case 't':
                dest->t = atoi(optarg);
                if (dest->t > MAX_ARG_T || dest->t < MIN_ARG_T) usage(argv[0]);
                break;
            default: usage(argv[0]);
        }
    }
    if (dest->diPath == NULL) {
        if ((buffer = getenv(DIR_ENV_PATH)) == NULL) usage(argv[0]);
        dest->diPath = strdup(buffer);
    }
    if (dest->indexPath == NULL) {
        if ((buffer = getenv(INDEX_ENV_PATH)) == NULL) {
            dest->indexPath = malloc((MAX_PATH + 1) * sizeof(char)); // max +1 for null terminator
            get_default_index_path(dest->indexPath);
        } else dest->indexPath = strdup(buffer);
    }
}

void execute_command(indexState_t* state, const char *command) {
    char *argPos, *namePtr;
    long numBuffer;

    argPos = strchr(command, COMMAND_DELIM);
    if (argPos != NULL) {
        *argPos = '\0';
        argPos++;
        numBuffer = strtol(argPos, &namePtr, 10);
    }

    if (strcmp(command, "exit") == 0) {
        execute_exit(state, false);
    } else if (strcmp(command, "exit!") == 0) {
        execute_exit(state, true); // with force
    } else if (strcmp(command, "count") == 0) {
        execute_count(state);
    } else if (strcmp(command, "index") == 0) {
        execute_index(state);
    } else if (strcmp(command, "largerthan") == 0) {
        if (argPos == NULL || namePtr == argPos || numBuffer < 0) WARN("Bad command syntax");
        else execute_largerthan(state, numBuffer);
    } else if (strcmp(command, "namepart") == 0) {
        if (argPos == NULL) WARN("Bad command syntax");
        else execute_namepart(state, argPos);
    } else if (strcmp(command, "owner") == 0) {
        if (argPos == NULL || namePtr == argPos || numBuffer < 0) WARN("Bad command syntax");
        else execute_owner(state, numBuffer);
    } else WARN("Unknown command");
}

void main_command_loop(const programArgs_t* args) {
    // +2 for \n and \0
    char buffer[MAX_COMMAND + 2];
    char* eol;
    indexState_t state;
    
    init_index_state(&state, args->diPath, args->indexPath, args->t);
    init_index(&state);

    while(1) {
        printf(CLI_PROMPT);
        fgets(buffer, MAX_COMMAND + 2, stdin);
        if ((eol = strchr(buffer, '\n')) != NULL) *eol = '\0';
        execute_command(&state, buffer);
        printf("\n");
    }
}
