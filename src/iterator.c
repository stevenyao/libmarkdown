#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../include/markdown/iterator.h"

struct md_iterator {
    const md_document_t *doc;
    md_node_t *current;
    md_element_t element;
    bool started;
};

static void node_to_element(const md_node_t *node, md_element_t *elem) {
    if (!node || !elem) return;
    
    *elem = (md_element_t){0};
    
    elem->type = node->type;
    elem->content = node->content;
    elem->content_len = node->content_len;
    
    switch (node->type) {
        case MD_NODE_HEADING:
            elem->data.block.heading_level = node->data.block.heading_level;
            break;
        case MD_NODE_FENCED_CODE:
        case MD_NODE_INDENTED_CODE:
            elem->data.block.language = node->data.block.language;
            break;
        case MD_NODE_LIST:
        case MD_NODE_TASK_LIST_ITEM:
            elem->data.block.list_start = node->data.block.list_start;
            elem->data.block.list_marker = node->data.block.list_marker;
            elem->data.block.is_task = node->data.block.is_task;
            elem->data.block.is_checked = node->data.block.is_checked;
            break;
        case MD_NODE_LINK:
        case MD_NODE_IMAGE:
            elem->data.inline_.url = node->data.inline_.url;
            elem->data.inline_.title = node->data.inline_.title;
            elem->data.inline_.alt = node->data.inline_.alt;
            break;
        case MD_NODE_EMPHASIS:
        case MD_NODE_STRONG:
        case MD_NODE_STRONG_EMPHASIS:
            elem->data.inline_.emphasis_level = node->data.inline_.emphasis_level;
            break;
        default:
            break;
    }
    
    if (node->parent) {
        elem->parent_type = node->parent->type;
    }
}

md_iterator_t *md_iterator_create(const md_document_t *doc) {
    if (!doc) return NULL;
    
    md_iterator_t *iter = (md_iterator_t *)malloc(sizeof(md_iterator_t));
    if (!iter) return NULL;
    
    iter->doc = doc;
    iter->current = NULL;
    iter->started = false;
    memset(&iter->element, 0, sizeof(md_element_t));
    
    return iter;
}

void md_iterator_destroy(md_iterator_t *iter) {
    free(iter);
}

void md_iterator_reset(md_iterator_t *iter) {
    if (iter) {
        iter->current = NULL;
        iter->started = false;
    }
}

md_element_t *md_iterator_next(md_iterator_t *iter) {
    if (!iter || !iter->doc) return NULL;
    
    if (!iter->started) {
        iter->current = iter->doc->root ? iter->doc->root->first_child : NULL;
        iter->started = true;
    } else if (iter->current) {
        if (iter->current->first_child) {
            iter->current = iter->current->first_child;
        } else if (iter->current->next) {
            iter->current = iter->current->next;
        } else {
            // Go up the tree looking for a next sibling
            while (iter->current->parent) {
                if (iter->current->parent->next) {
                    iter->current = iter->current->parent->next;
                    break;
                }
                iter->current = iter->current->parent;
                if (iter->current == iter->doc->root) {
                    // Reached root, no more nodes
                    iter->current = NULL;
                    return NULL;
                }
            }
            if (!iter->current) {
                return NULL;
            }
        }
    }
    
    if (iter->current) {
        node_to_element(iter->current, &iter->element);
        return &iter->element;
    }
    
    return NULL;
}

const md_element_t *md_iterator_get(const md_iterator_t *iter) {
    return iter ? &iter->element : NULL;
}
