#ifndef storage_h
#define storage_h

#include <stdbool.h>
#include <unistd.h>

#include "shared.h"
#include "file_type.h"

typedef struct indexObject {
    char name[MAX_NAME + 1]; // +1 for null terminator
    char path[MAX_PATH + 1]; // +1 for null terminator
    enum FileType type;
    uid_t uid;
    size_t size;
} indexObject_t;

typedef struct indexListNode {
    indexObject_t *data;
    struct indexListNode *next;
} indexListNode_t;

void destroy_index_list(indexListNode_t** head);
// the element before inserting to the list is copied
void insert_to_index_list(indexListNode_t** head, const indexObject_t* element);
void save_index_list(const indexListNode_t* head, const char *path);
void load_index_list(indexListNode_t** head, const char *path);
bool is_index_list_longer_than(const indexListNode_t* head, int than);

#endif
