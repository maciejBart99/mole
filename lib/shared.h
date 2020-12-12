#ifndef shared_h
#define shared_h

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define ERR(source) (perror(source),\
             fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
             exit(EXIT_FAILURE))
#define WARN(msg) fprintf(stderr,"%s\n", msg)

#define MAX_NAME 100
#define MAX_PATH 200
#define MAX_OPEN_WALK_DESCRIPTORS 20

#define SIGNATURE_LEN 4

#endif
