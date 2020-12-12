#ifndef shared_h
#define shared_h

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define ERR(source) (perror(source),\
             fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
             exit(EXIT_FAILURE))

#define MAX_NAME 100
#define MAX_PATH 200

#define SIGNATURE_LEN 4

#endif
