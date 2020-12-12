#ifndef queries_h
#define queries_h

#include "storage.h"
#include "file_type.h"

typedef struct fileTypesCount {
    size_t jpeg;
    size_t zip;
    size_t gzip;
    size_t png;
    size_t dir;
} fileTypesCount_t;

fileTypesCount_t get_index_count(indexListNode_t *head);
indexListNode_t *get_larger_than(indexListNode_t *head, size_t than);
indexListNode_t *get_namepart(indexListNode_t *head, const char *namepart);
indexListNode_t *get_with_owner(indexListNode_t *head, uid_t owner);

#endif
