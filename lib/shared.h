#ifndef shared_h
#define shared_h

#define ERR(source) (perror(source),\
             fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
             exit(EXIT_FAILURE))
#define ERR_PLAIN(source) (fprintf(stderr,"%s\n%s:%d\n", source, __FILE__,__LINE__),\
                            exit(EXIT_FAILURE))
#define WARN(msg) fprintf(stderr,"%s\n", msg)

#define WITH_MUTEX(instruction, mutex) (pthread_mutex_lock(mutex),instruction,pthread_mutex_unlock(mutex))

#define CLI_PROMPT ">"
#define MAX_COMMAND 50
#define MAX_NAME 50
#define MAX_PATH 100
#define MAX_TYPE_STR 5
#define NULL_THREAD 0

#endif
