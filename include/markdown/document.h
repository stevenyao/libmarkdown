#ifndef MARKDOWN_DOCUMENT_H
#define MARKDOWN_DOCUMENT_H

#include "export.h"
#include "ast.h"
#include <stdbool.h>

MARKDOWN_API void md_document_free(md_document_t *doc);

MARKDOWN_API const char *md_document_get_source(const md_document_t *doc);
MARKDOWN_API size_t md_document_get_source_len(const md_document_t *doc);

typedef int (*md_node_callback_t)(const md_node_t *node, void *data);

MARKDOWN_API int md_document_visit(const md_document_t *doc, md_node_callback_t callback, void *data);

MARKDOWN_API md_node_type_t md_node_get_type(const md_node_t *node);
MARKDOWN_API const char *md_node_get_content(const md_node_t *node);
MARKDOWN_API const md_node_t *md_node_get_parent(const md_node_t *node);
MARKDOWN_API const md_node_t *md_node_get_first_child(const md_node_t *node);
MARKDOWN_API const md_node_t *md_node_get_last_child(const md_node_t *node);
MARKDOWN_API const md_node_t *md_node_get_prev(const md_node_t *node);
MARKDOWN_API const md_node_t *md_node_get_next(const md_node_t *node);

MARKDOWN_API int md_node_get_heading_level(const md_node_t *node);
MARKDOWN_API const char *md_node_get_code_language(const md_node_t *node);
MARKDOWN_API int md_node_get_list_start(const md_node_t *node);
MARKDOWN_API char md_node_get_list_marker(const md_node_t *node);
MARKDOWN_API bool md_node_is_task_list(const md_node_t *node);
MARKDOWN_API bool md_node_is_checked(const md_node_t *node);

MARKDOWN_API const char *md_node_get_url(const md_node_t *node);
MARKDOWN_API const char *md_node_get_title(const md_node_t *node);
MARKDOWN_API const char *md_node_get_alt(const md_node_t *node);
MARKDOWN_API int md_node_get_emphasis_level(const md_node_t *node);

MARKDOWN_API const char *md_node_type_name(md_node_type_t type);

#endif
