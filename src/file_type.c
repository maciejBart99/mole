#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "file_type.h"
#include "shared.h"

#define ZIP_SIGNATURE 0x504b0304
#define PNG_SIGNATURE 0x89504e47
#define JPEG_SIGNATURE 0xffd8ffe0
#define GZIP_SIGNATURE 0x1f8b


enum FileType recognize_type_from_magic_number(const int number) {
    switch (number) {
        case ZIP_SIGNATURE:
            return ZIP;
        case PNG_SIGNATURE:
            return PNG;
        case JPEG_SIGNATURE:
            return JPEG;
        default:
            if ((number << 2) == GZIP_SIGNATURE) return GZIP;
            else return OTHER;;
    }
}

enum FileType recognize_file_type(const char* path) {
    int fd, signature;
    ssize_t rd;
    if ((fd = open(path, O_RDONLY)) < 0) ERR("open");
    if ((rd = read(fd, &signature, SIGNATURE_LEN)) < 0) ERR("read");
    if (close(fd)) ERR("close");
    if (rd < SIGNATURE_LEN) return OTHER;
    return recognize_type_from_magic_number(signature);
}
