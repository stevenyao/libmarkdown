#include <stdlib.h>
#include <string.h>
#include "../include/markdown/ast.h"

int md_node_add_child(md_node_t *parent, md_node_t *child) {
    if (!parent || !child) return -1;
    
    child->parent = parent;
    child->prev = parent->last_child;
    child->next = NULL;
    
    if (parent->last_child) {
        parent->last_child->next = child;
    } else {
        parent->first_child = child;
    }
    parent->last_child = child;
    
    return 0;
}

void md_node_free(md_node_t *node) {
    if (!node) return;
    
    md_node_t *child = node->first_child;
    while (child) {
        md_node_t *next = child->next;
        md_node_free(child);
        child = next;
    }
    
    if (node->content) free(node->content);
    free(node);
}
