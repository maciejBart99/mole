#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "../lib/storage.h"
#include "../lib/shared.h"
#include "../lib/bulk_io.h"

void destroy_index_list(indexListNode_t** head) {
    indexListNode_t *next;
    while(*head) {
        free((*head)->data);
        next = (*head)->next;
        free(*head);
        *head = next;
    }
}

void insert_to_index_list(indexListNode_t** head, const indexObject_t* element) {
    indexListNode_t* node = (indexListNode_t*) malloc(sizeof(indexListNode_t));
    if (node == NULL) ERR("malloc");
    node->data = malloc(sizeof(indexObject_t));
    if (node->data == NULL) ERR("malloc");
    *(node->data) = *element;
    node->next = *head;
    *head = node;
}

void save_index_list(const indexListNode_t* head, const char *path) {
    int fd;
    if ((fd = TEMP_FAILURE_RETRY(open(path, O_TRUNC | O_CREAT | O_WRONLY, 0777))) < 0) ERR("open");
    
    while(head) {
        if (bulk_write(fd, (char*)head->data, sizeof(indexObject_t)) != sizeof(indexObject_t)) ERR("write");
        head = head->next;
    }
    
    if (TEMP_FAILURE_RETRY(close(fd))) ERR("close");
}

void load_index_list(indexListNode_t** head, const char *path) {
    int fd;
    ssize_t rd;
    indexObject_t p;
    if ((fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY))) < 0) ERR("open");
    
    while((rd = bulk_read(fd, (char*)&p, sizeof(indexObject_t))) != 0) {
        if (rd < 0) ERR("read");
        if (rd != sizeof(indexObject_t)) ERR_PLAIN("Corrupted index file format");
        insert_to_index_list(head, &p);
    }
    
    if (TEMP_FAILURE_RETRY(close(fd))) ERR("close");
}

bool is_index_list_longer_than(const indexListNode_t* head, int than) {
    while (than > 0) {
        if ((head = head->next) == NULL) return false;
        than--;
    }
    return true;
}
