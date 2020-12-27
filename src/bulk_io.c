#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


#include "../lib/bulk_io.h"

ssize_t bulk_read(int fd, char *buf, size_t count){
    ssize_t c, len = 0;
    do{
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if(c < 0) return c;
        if(c == 0) return len;
        buf += c;
        len += c;
        count -= c;
    } while(count > 0);
    return len;
}
ssize_t bulk_write(int fd, const char *buf, size_t count){
    ssize_t c, len = 0;
    do{
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if(c < 0) return c;
        buf += c;
        len += c;
        count -= c;
    } while(count > 0);
    return len;
}