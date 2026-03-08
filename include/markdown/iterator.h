#ifndef MARKDOWN_ITERATOR_H
#define MARKDOWN_ITERATOR_H

#include "ast.h"

typedef struct md_iterator md_iterator_t;

typedef struct md_element {
    md_node_type_t type;
    int line;
    int column;
    char *content;
    size_t content_len;
    
    union {
        struct {
            int heading_level;
            char *language;
            int list_start;
            char list_marker;
            int is_task;
            int is_checked;
        } block;
        
        struct {
            char *url;
            char *title;
            char *alt;
            int emphasis_level;
        } inline_;
    } data;
    
    md_node_type_t parent_type;
    int depth;
} md_element_t;

md_iterator_t *md_iterator_create(const md_document_t *doc);
void md_iterator_destroy(md_iterator_t *iter);
void md_iterator_reset(md_iterator_t *iter);

md_element_t *md_iterator_next(md_iterator_t *iter);
const md_element_t *md_iterator_get(const md_iterator_t *iter);

#endif
