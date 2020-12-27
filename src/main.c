#include <stdlib.h>

#include "../lib/cli.h"

int main(int argc, char * argv[]) {
    programArgs_t args;
    
    parse_arguments(argc, argv, &args);
    main_command_loop(&args);
    return EXIT_SUCCESS;
}
