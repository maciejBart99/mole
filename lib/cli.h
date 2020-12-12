#ifndef cli_h
#define cli_h

typedef struct programArgs {
    int t;
    char *diPath;
    char *indexPath;
} programArgs_t;

typedef enum CommandType {
    EXIT,
    FORCE_EXIT,
    INDEX,
    
} CommandType_t;

void parse_arguments(int argc, const char** argv, programArgs_t* dest);
void main_command_loop(programArgs_t* args);


#endif
