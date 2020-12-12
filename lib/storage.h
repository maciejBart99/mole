#ifndef storage_h
#define storage_h

#include "shared.h"
#include "file_type.h"

typedef struct indexObject {
    char name[MAX_NAME];
    char path[MAX_PATH];
    enum FileType type;
    uid_t uid;
    size_t size;
} indexObject_t;

typedef struct indexListNode {
    indexObject_t *data;
    struct indexListNode *next;
} indexListNode_t;

void delete_index_list(indexListNode_t** head);
void insert_to_index_list(indexListNode_t** head, const indexObject_t* element);
void save_index_list(indexListNode_t** head, const char *path);
void load_index_list(indexListNode_t** head, const char *path);

#endif
