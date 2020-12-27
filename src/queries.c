#include <stdio.h>
#include <string.h>

#include "../lib/queries.h"

fileTypesCount_t get_index_count(const indexListNode_t *head) {
    fileTypesCount_t counts = {
        0,0,0,0,0
    };
    
    while(head) {
        switch (head->data->type) {
            case PNG: counts.png++;
                break;
            case DIR: counts.dir++;
                break;
            case JPEG: counts.jpeg++;
                break;
            case ZIP: counts.zip++;
                break;
            case GZIP: counts.gzip++;
                break;
            default:
                break;
        }
        head = head->next;
    }
    
    return counts;
}

indexListNode_t *get_larger_than(const indexListNode_t *head, size_t than) {
    indexListNode_t* result = NULL;
    while (head) {
        if (head->data->size > than) {
            insert_to_index_list(&result, head->data);
        }
        head = head->next;
    }
    
    return result;
}

indexListNode_t *get_namepart(const indexListNode_t *head, const char *namepart) {
    indexListNode_t* result = NULL;
    
    while (head) {
        if (strstr(head->data->name, namepart) != NULL) {
            insert_to_index_list(&result, head->data);
        }
        head = head->next;
    }
    
    return result;
}

indexListNode_t *get_with_owner(const indexListNode_t *head, uid_t owner) {
    indexListNode_t* result = NULL;
    
    while (head) {
        if (head->data->uid == owner) {
            insert_to_index_list(&result, head->data);
        }
        head = head->next;
    }
    
    return result;
}
