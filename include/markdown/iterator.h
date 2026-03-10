#ifndef MARKDOWN_ITERATOR_H
#define MARKDOWN_ITERATOR_H

#include "export.h"
#include "ast.h"
#include <stdbool.h>

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
            bool is_task;
            bool is_checked;
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

MARKDOWN_API md_iterator_t *md_iterator_create(const md_document_t *doc);
MARKDOWN_API void md_iterator_destroy(md_iterator_t *iter);
MARKDOWN_API void md_iterator_reset(md_iterator_t *iter);

MARKDOWN_API md_element_t *md_iterator_next(md_iterator_t *iter);
MARKDOWN_API const md_element_t *md_iterator_get(const md_iterator_t *iter);

#endif
