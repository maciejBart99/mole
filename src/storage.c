#include <stdio.h>
#include <string.h>

#include "storage.h"
#include "shared.h"

void destory_index_list(indexListNode_t** head) {
    indexListNode_t *next;
    while(*head) {
        free((*head)->data);
        next = (*head)->next;
        free(*head);
        *head = next;
    }
}

void insert_to_index_list(indexListNode_t** head, const indexObject_t* element) {
    indexListNode_t* node = malloc(sizeof(indexListNode_t));
    if (node == NULL) ERR("malloc");
    node->data = malloc(sizeof(indexListNode_t));
    if (node->data == NULL) ERR("malloc");
    if (memcpy(node->data, element, sizeof(indexObject_t)) == NULL) ERR("memcpy");
    node->next = *head;
    *head = node;
}

void save_index_list(indexListNode_t** head, const char *path) {
    int fd;
    indexListNode_t *p = *head;
    if ((fd = open(path, O_TRUNC | O_CREAT | O_WRONLY)) < 0) ERR("open");
    
    while(p) {
        if (write(fd, p->data, sizeof(indexObject_t)) != sizeof(indexObject_t)) ERR("write");
        p = p->next;
    }
    
    if (close(fd)) ERR("close");
}

void load_index_list(indexListNode_t** head, const char *path) {
    int fd;
    ssize_t rd;
    indexObject_t p;
    if ((fd = open(path, O_RDONLY)) < 0) ERR("open");
    
    while((rd = read(fd, &p, sizeof(indexObject_t))) != 0) {
        if (rd < 0) ERR("read");
        if (rd != sizeof(indexObject_t)) ERR("corrupted index file format");
        insert_to_index_list(head, &p);
    }
    
    if (close(fd)) ERR("close");
}

