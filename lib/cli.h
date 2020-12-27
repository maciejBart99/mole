#ifndef cli_h
#define cli_h

#include "indexing.h"

#define PAGINATION_ENV_NAME "PAGER"
#define HOME_ENV_NAME "HOME"
#define DIR_ENV_PATH "MOLE_DIR"
#define INDEX_ENV_PATH "MOLE_INDEX_PATH"
#define DEFAULT_INDEX_FILE ".mole-index"
#define MAX_RECORDS_WITHOUT_PAGINATION 3
#define MAX_SIZE_STR_LEN 10
#define COMMAND_DELIM ' '
#define MAX_ARG_T 7200
#define MIN_ARG_T 30

typedef struct programArgs {
    int t;
    char *diPath;
    char *indexPath;
} programArgs_t;

void parse_arguments(int argc, char** argv, programArgs_t* dest);
void execute_command(indexState_t* state, const char *command);
void main_command_loop(const programArgs_t* args);


#endif
