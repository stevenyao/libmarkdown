#include <stdlib.h>
#include <string.h>
#include "../include/markdown/document.h"

void md_document_free(md_document_t *doc) {
    if (!doc) return;
    if (doc->source) free(doc->source);
    if (doc->root) md_node_free(doc->root);
    free(doc);
}

const char *md_document_get_source(const md_document_t *doc) {
    return doc ? doc->source : NULL;
}

size_t md_document_get_source_len(const md_document_t *doc) {
    return doc ? doc->source_len : 0;
}

static int visit_node(const md_node_t *node, md_node_callback_t callback, void *data, int *depth) {
    if (!node) return 0;
    
    if (callback(node, data)) return 1;
    
    (*depth)++;
    const md_node_t *child = node->first_child;
    while (child) {
        if (visit_node(child, callback, data, depth)) return 1;
        child = child->next;
    }
    (*depth)--;
    
    return 0;
}

int md_document_visit(const md_document_t *doc, md_node_callback_t callback, void *data) {
    if (!doc || !callback) return -1;
    int depth = 0;
    return visit_node(doc->root, callback, data, &depth);
}

md_node_type_t md_node_get_type(const md_node_t *node) {
    return node ? node->type : MD_NODE_DOCUMENT;
}

const char *md_node_get_content(const md_node_t *node) {
    return node ? node->content : NULL;
}

const md_node_t *md_node_get_parent(const md_node_t *node) {
    return node ? node->parent : NULL;
}

const md_node_t *md_node_get_first_child(const md_node_t *node) {
    return node ? node->first_child : NULL;
}

const md_node_t *md_node_get_last_child(const md_node_t *node) {
    return node ? node->last_child : NULL;
}

const md_node_t *md_node_get_prev(const md_node_t *node) {
    return node ? node->prev : NULL;
}

const md_node_t *md_node_get_next(const md_node_t *node) {
    return node ? node->next : NULL;
}

int md_node_get_heading_level(const md_node_t *node) {
    return node && node->type == MD_NODE_HEADING ? node->data.block.heading_level : 0;
}

const char *md_node_get_code_language(const md_node_t *node) {
    if (!node) return NULL;
    if (node->type == MD_NODE_FENCED_CODE || node->type == MD_NODE_INDENTED_CODE) {
        return node->data.block.language;
    }
    return NULL;
}

int md_node_get_list_start(const md_node_t *node) {
    return node && node->type == MD_NODE_LIST ? node->data.block.list_start : 0;
}

char md_node_get_list_marker(const md_node_t *node) {
    return node ? node->data.block.list_marker : 0;
}

int md_node_is_task_list(const md_node_t *node) {
    return node && (node->type == MD_NODE_TASK_LIST_ITEM || 
           (node->type == MD_NODE_LIST && node->data.block.list_type == MD_LIST_TASK));
}

int md_node_is_checked(const md_node_t *node) {
    return node && node->type == MD_NODE_TASK_LIST_ITEM ? node->data.block.is_checked : 0;
}

const char *md_node_get_url(const md_node_t *node) {
    if (!node) return NULL;
    if (node->type == MD_NODE_LINK || node->type == MD_NODE_IMAGE) {
        return node->data.inline_.url;
    }
    return NULL;
}

const char *md_node_get_title(const md_node_t *node) {
    if (!node) return NULL;
    if (node->type == MD_NODE_LINK) {
        return node->data.inline_.title;
    }
    return NULL;
}

const char *md_node_get_alt(const md_node_t *node) {
    if (!node) return NULL;
    if (node->type == MD_NODE_IMAGE) {
        return node->data.inline_.alt;
    }
    return NULL;
}

int md_node_get_emphasis_level(const md_node_t *node) {
    if (!node) return 0;
    if (node->type == MD_NODE_EMPHASIS) return 1;
    if (node->type == MD_NODE_STRONG) return 2;
    if (node->type == MD_NODE_STRONG_EMPHASIS) return 3;
    return 0;
}

static const char *type_names[] = {
    "document",
    "heading",
    "fenced_code",
    "indented_code",
    "list",
    "list_item",
    "task_list_item",
    "blockquote",
    "table",
    "table_head",
    "table_row",
    "table_cell",
    "thematic_break",
    "html_block",
    "paragraph",
    "link_ref_def",
    "front_matter",
    "text",
    "emphasis",
    "strong",
    "strong_emphasis",
    "strikethrough",
    "code_span",
    "link",
    "image",
    "autolink",
    "email_autolink",
    "line_break",
    "soft_line_break",
    "html_inline",
    "footnote_ref",
    "footnote_def",
    "mark",
    "subscript",
    "superscript",
    "span"
};

const char *md_node_type_name(md_node_type_t type) {
    int idx = (int)type;
    if (idx < 0 || idx >= (int)(sizeof(type_names) / sizeof(type_names[0]))) {
        return "unknown";
    }
    return type_names[idx];
}
