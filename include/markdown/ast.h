#ifndef MARKDOWN_AST_H
#define MARKDOWN_AST_H

#include "export.h"
#include <stddef.h>

typedef enum md_node_type {
    MD_NODE_DOCUMENT,
    
    MD_NODE_HEADING,
    MD_NODE_FENCED_CODE,
    MD_NODE_INDENTED_CODE,
    MD_NODE_LIST,
    MD_NODE_LIST_ITEM,
    MD_NODE_TASK_LIST_ITEM,
    MD_NODE_BLOCKQUOTE,
    MD_NODE_TABLE,
    MD_NODE_TABLE_HEAD,
    MD_NODE_TABLE_ROW,
    MD_NODE_TABLE_CELL,
    MD_NODE_THEMATIC_BREAK,
    MD_NODE_HTML_BLOCK,
    MD_NODE_PARAGRAPH,
    MD_NODE_LINK_REF_DEF,
    MD_NODE_FRONT_MATTER,
    
    MD_NODE_TEXT,
    MD_NODE_EMPHASIS,
    MD_NODE_STRONG,
    MD_NODE_STRONG_EMPHASIS,
    MD_NODE_STRIKETHROUGH,
    MD_NODE_CODE_SPAN,
    MD_NODE_LINK,
    MD_NODE_IMAGE,
    MD_NODE_AUTOLINK,
    MD_NODE_EMAIL_AUTOLINK,
    MD_NODE_LINE_BREAK,
    MD_NODE_SOFT_LINE_BREAK,
    MD_NODE_HTML_INLINE,
    MD_NODE_FOOTNOTE_REF,
    MD_NODE_FOOTNOTE_DEF,
    MD_NODE_MARK,
    MD_NODE_SUBSCRIPT,
    MD_NODE_SUPERSCRIPT,
    MD_NODE_SPAN,
} md_node_type_t;

typedef enum md_list_type {
    MD_LIST_UNORDERED,
    MD_LIST_ORDERED,
    MD_LIST_TASK
} md_list_type_t;

typedef enum md_list_delim {
    MD_LIST_DELIM_PERIOD,
    MD_LIST_DELIM_PAREN
} md_list_delim_t;

typedef enum md_table_align {
    MD_TABLE_ALIGN_NONE,
    MD_TABLE_ALIGN_LEFT,
    MD_TABLE_ALIGN_CENTER,
    MD_TABLE_ALIGN_RIGHT
} md_table_align_t;

typedef enum md_emphasis_type {
    MD_EMPHASIS_ITALIC,
    MD_EMPHASIS_BOLD,
    MD_EMPHASIS_BOLD_ITALIC
} md_emphasis_type_t;

typedef struct md_node {
    md_node_type_t type;
    int flags;
    union {
        struct {
            int heading_level;
            char *language;
            char *info_string;
            char *raw_html;
            int list_start;
            char list_marker;
            md_list_type_t list_type;
            md_list_delim_t list_delim;
            int is_task;
            int is_checked;
            md_table_align_t table_align;
        } block;
        struct {
            char *url;
            char *title;
            char *alt;
            int emphasis_level;
        } inline_;
    } data;
    char *content;
    size_t content_len;
    struct md_node *parent;
    struct md_node *first_child;
    struct md_node *last_child;
    struct md_node *prev;
    struct md_node *next;
} md_node_t;

typedef struct {
    md_node_t *root;
    char *source;
    size_t source_len;
    md_node_t *link_refs;
} md_document_t;

MARKDOWN_API int md_node_add_child(md_node_t *parent, md_node_t *child);
MARKDOWN_API void md_node_free(md_node_t *node);

#endif
