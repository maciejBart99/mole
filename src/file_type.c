#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "../lib/file_type.h"
#include "../lib/bulk_io.h"
#include "../lib/shared.h"


enum FileType recognize_type_from_magic_number(const unsigned int number) {
    switch (number) {
        case ZIP_SIGNATURE:
            return ZIP;
        case PNG_SIGNATURE:
            return PNG;
        case JPEG_SIGNATURE:
            return JPEG;
        default:
            if ((number >> GZIP_SIGNATURE_OFFSET) == GZIP_SIGNATURE) return GZIP;
            else return OTHER;
    }
}

enum FileType recognize_file_type(const char* path) {
    int fd;
    unsigned int signature;
    ssize_t rd;
    if ((fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY))) < 0) ERR("open");
    if ((rd = bulk_read(fd, (char*)&signature, SIGNATURE_LEN)) < 0) ERR("read");
    if (TEMP_FAILURE_RETRY(close(fd))) ERR("close");
    // swap byte order
    return recognize_type_from_magic_number(ntohl(signature));
}

void get_string_file_type(enum FileType type, char* dest) {
    switch (type) {
        case GZIP: strcpy(dest, "GZIP");
            break;
        case DIR: strcpy(dest, "DIR");
            break;
        case ZIP: strcpy(dest, "ZIP");
            break;
        case JPEG: strcpy(dest, "JPEG");
            break;
        case PNG: strcpy(dest, "PNG");
            break;
        case OTHER: strcpy(dest, "OTHER");
            break;
    }
}
