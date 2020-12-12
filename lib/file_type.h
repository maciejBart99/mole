#ifndef file_type_h
#define file_type_h

enum FileType {
    JPEG,
    PNG,
    GZIP,
    ZIP,
    DIR,
    OTHER
};

enum FileType recognize_type_from_magic_number(const int number);
enum FileType recognize_file_type(const char* path);

#endif
