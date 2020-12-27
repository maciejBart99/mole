#ifndef bulk_io_h
#define bulk_io_h

#include <stdio.h>

ssize_t bulk_read(int fd, char *buf, size_t count);

ssize_t bulk_write(int fd, const char *buf, size_t count);

#endif