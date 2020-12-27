#ifndef file_type_h
#define file_type_h

#define SIGNATURE_LEN 4
#define ZIP_SIGNATURE 0x504b0304
#define PNG_SIGNATURE 0x89504e47
#define JPEG_SIGNATURE 0xffd8ffe0
#define GZIP_SIGNATURE 0x1f8b
#define GZIP_SIGNATURE_OFFSET 16

enum FileType {
    JPEG,
    PNG,
    GZIP,
    ZIP,
    DIR,
    OTHER
};

enum FileType recognize_type_from_magic_number(const unsigned int number);
enum FileType recognize_file_type(const char* path);
void get_string_file_type(enum FileType type, char* dest);

#endif
