#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "../include/markdown/extractor.h"

static int collect_nodes(const md_node_t *node, md_node_type_t target_type, 
                        md_node_t ***results, size_t *count, size_t *capacity) {
    if (!node) return 0;
    
    if (node->type == target_type) {
        if (*count >= *capacity) {
            size_t new_cap = *capacity == 0 ? 16 : *capacity * 2;
            md_node_t **new_results = (md_node_t **)realloc(*results, new_cap * sizeof(md_node_t *));
            if (!new_results) return -1;
            *results = new_results;
            *capacity = new_cap;
        }
        (*results)[*count] = (md_node_t *)node;
        (*count)++;
    }
    
    const md_node_t *child = node->first_child;
    while (child) {
        if (collect_nodes(child, target_type, results, count, capacity) < 0) return -1;
        child = child->next;
    }
    
    return 0;
}

int md_extract_headings(const md_document_t *doc, md_heading_t **headings, size_t *count) {
    if (!doc || !headings || !count) return -1;
    
    *headings = NULL;
    *count = 0;
    
    md_node_t **nodes = NULL;
    size_t capacity = 0;
    
    if (collect_nodes(doc->root, MD_NODE_HEADING, &nodes, count, &capacity) < 0) {
        free(nodes);
        return -1;
    }
    
    if (*count == 0) return 0;
    
    *headings = (md_heading_t *)malloc(*count * sizeof(md_heading_t));
    if (!*headings) {
        free(nodes);
        *count = 0;
        return -1;
    }
    
    for (size_t i = 0; i < *count; i++) {
        (*headings)[i].level = nodes[i]->data.block.heading_level;
        (*headings)[i].text = nodes[i]->content ? strdup(nodes[i]->content) : NULL;
        (*headings)[i].node = nodes[i];
    }
    
    free(nodes);
    return 0;
}

void md_headings_free(md_heading_t *headings, size_t count) {
    if (!headings) return;
    for (size_t i = 0; i < count; i++) {
        free(headings[i].text);
    }
    free(headings);
}

int md_extract_code_blocks(const md_document_t *doc, md_code_block_t **blocks, size_t *count) {
    if (!doc || !blocks || !count) return -1;
    
    *blocks = NULL;
    *count = 0;
    
    md_node_t **nodes = NULL;
    size_t capacity = 0;
    
    if (collect_nodes(doc->root, MD_NODE_FENCED_CODE, &nodes, count, &capacity) < 0) {
        free(nodes);
        return -1;
    }
    
    size_t indented_count = 0;
    if (collect_nodes(doc->root, MD_NODE_INDENTED_CODE, &nodes, &indented_count, &capacity) < 0) {
        free(nodes);
        return -1;
    }
    
    *count += indented_count;
    
    if (*count == 0) return 0;
    
    *blocks = (md_code_block_t *)malloc(*count * sizeof(md_code_block_t));
    if (!*blocks) {
        free(nodes);
        *count = 0;
        return -1;
    }
    
    for (size_t i = 0; i < *count; i++) {
        (*blocks)[i].language = nodes[i]->data.block.language ? strdup(nodes[i]->data.block.language) : NULL;
        (*blocks)[i].code = nodes[i]->content ? strdup(nodes[i]->content) : NULL;
        (*blocks)[i].node = nodes[i];
    }
    
    free(nodes);
    return 0;
}

void md_code_blocks_free(md_code_block_t *blocks, size_t count) {
    if (!blocks) return;
    for (size_t i = 0; i < count; i++) {
        free(blocks[i].language);
        free(blocks[i].code);
    }
    free(blocks);
}

int md_extract_links(const md_document_t *doc, md_link_t **links, size_t *count) {
    if (!doc || !links || !count) return -1;
    
    *links = NULL;
    *count = 0;
    
    md_node_t **nodes = NULL;
    size_t capacity = 0;
    
    if (collect_nodes(doc->root, MD_NODE_LINK, &nodes, count, &capacity) < 0) {
        free(nodes);
        return -1;
    }
    
    if (*count == 0) return 0;
    
    *links = (md_link_t *)malloc(*count * sizeof(md_link_t));
    if (!*links) {
        free(nodes);
        *count = 0;
        return -1;
    }
    
    for (size_t i = 0; i < *count; i++) {
        (*links)[i].text = nodes[i]->content ? strdup(nodes[i]->content) : NULL;
        (*links)[i].url = nodes[i]->data.inline_.url ? strdup(nodes[i]->data.inline_.url) : NULL;
        (*links)[i].title = nodes[i]->data.inline_.title ? strdup(nodes[i]->data.inline_.title) : NULL;
        (*links)[i].node = nodes[i];
    }
    
    free(nodes);
    return 0;
}

void md_links_free(md_link_t *links, size_t count) {
    if (!links) return;
    for (size_t i = 0; i < count; i++) {
        free(links[i].text);
        free(links[i].url);
        free(links[i].title);
    }
    free(links);
}

int md_extract_images(const md_document_t *doc, md_image_t **images, size_t *count) {
    if (!doc || !images || !count) return -1;
    
    *images = NULL;
    *count = 0;
    
    md_node_t **nodes = NULL;
    size_t capacity = 0;
    
    if (collect_nodes(doc->root, MD_NODE_IMAGE, &nodes, count, &capacity) < 0) {
        free(nodes);
        return -1;
    }
    
    if (*count == 0) return 0;
    
    *images = (md_image_t *)malloc(*count * sizeof(md_image_t));
    if (!*images) {
        free(nodes);
        *count = 0;
        return -1;
    }
    
    for (size_t i = 0; i < *count; i++) {
        (*images)[i].alt = nodes[i]->data.inline_.alt ? strdup(nodes[i]->data.inline_.alt) : NULL;
        (*images)[i].url = nodes[i]->data.inline_.url ? strdup(nodes[i]->data.inline_.url) : NULL;
        (*images)[i].node = nodes[i];
    }
    
    free(nodes);
    return 0;
}

void md_images_free(md_image_t *images, size_t count) {
    if (!images) return;
    for (size_t i = 0; i < count; i++) {
        free(images[i].alt);
        free(images[i].url);
    }
    free(images);
}

int md_extract_tables(const md_document_t *doc, md_table_t **tables, size_t *count) {
    (void)doc;
    (void)tables;
    (void)count;
    return 0;
}

void md_tables_free(md_table_t *tables, size_t count) {
    (void)tables;
    (void)count;
}

static void collect_text(const md_node_t *node, char **result, size_t *len) {
    if (!node || !result) return;
    
    if (node->type == MD_NODE_TEXT && node->content) {
        size_t node_len = node->content_len;
        char *new_result = (char *)realloc(*result, *len + node_len);
        if (new_result) {
            memcpy(new_result + *len, node->content, node_len);
            *len += node_len;
            *result = new_result;
        }
    }
    
    const md_node_t *child = node->first_child;
    while (child) {
        collect_text(child, result, len);
        child = child->next;
    }
}

char *md_extract_plain_text(const md_document_t *doc) {
    if (!doc) return NULL;
    
    char *result = NULL;
    size_t len = 0;
    
    collect_text(doc->root, &result, &len);
    
    if (result) {
        char *new_result = (char *)realloc(result, len + 1);
        if (new_result) {
            new_result[len] = '\0';
            return new_result;
        }
        result[len] = '\0';
    }
    
    return result;
}
