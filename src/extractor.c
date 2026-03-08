#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "../include/markdown/extractor.h"

// Forward declarations
static void measure_text_len(const md_node_t *node, size_t *len);
static void copy_text_content(const md_node_t *node, char *buffer, size_t *pos);

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

static char *extract_node_text(const md_node_t *node) {
    if (!node) return NULL;
    size_t len = 0;
    measure_text_len(node, &len);
    
    char *text = (char *)malloc(len + 1);
    if (!text) return NULL;
    
    size_t pos = 0;
    copy_text_content(node, text, &pos);
    text[pos] = '\0';
    return text;
}

int md_extract_tables(const md_document_t *doc, md_table_t **tables, size_t *count) {
    if (!doc || !tables || !count) return -1;
    
    *tables = NULL;
    *count = 0;
    
    md_node_t **nodes = NULL;
    size_t capacity = 0;
    
    if (collect_nodes(doc->root, MD_NODE_TABLE, &nodes, count, &capacity) < 0) {
        free(nodes);
        return -1;
    }
    
    if (*count == 0) return 0;
    
    *tables = (md_table_t *)malloc(*count * sizeof(md_table_t));
    if (!*tables) {
        free(nodes);
        *count = 0;
        return -1;
    }
    
    for (size_t i = 0; i < *count; i++) {
        md_table_t *tbl = &(*tables)[i];
        md_node_t *table_node = nodes[i];
        
        memset(tbl, 0, sizeof(md_table_t));
        tbl->node = table_node;
        
        md_node_t **rows = NULL;
        size_t r_count = 0;
        size_t r_cap = 0;
        
        if (collect_nodes(table_node, MD_NODE_TABLE_ROW, &rows, &r_count, &r_cap) == 0 && r_count > 0) {
            tbl->row_count = r_count;
            tbl->rows = (char ***)malloc(r_count * sizeof(char **));
            
            size_t max_cols = 0;
            for (size_t r = 0; r < r_count; r++) {
                size_t c = 0;
                md_node_t *cell = rows[r]->first_child;
                while (cell) {
                    if (cell->type == MD_NODE_TABLE_CELL) c++;
                    cell = cell->next;
                }
                if (c > max_cols) max_cols = c;
            }
            tbl->col_count = max_cols;
            
            tbl->aligns = (md_table_align_t *)malloc(max_cols * sizeof(md_table_align_t));
            if (tbl->aligns) {
                 memset(tbl->aligns, 0, max_cols * sizeof(md_table_align_t));
                 if (r_count > 0) {
                     md_node_t *cell = rows[0]->first_child;
                     size_t c = 0;
                     while (cell && c < max_cols) {
                         if (cell->type == MD_NODE_TABLE_CELL) {
                             tbl->aligns[c] = cell->data.block.table_align;
                             c++;
                         }
                         cell = cell->next;
                     }
                 }
            }

            if (tbl->rows) {
                for (size_t r = 0; r < r_count; r++) {
                    tbl->rows[r] = (char **)malloc(max_cols * sizeof(char *));
                    if (tbl->rows[r]) {
                        memset(tbl->rows[r], 0, max_cols * sizeof(char *));
                        md_node_t *cell = rows[r]->first_child;
                        size_t c = 0;
                        while (cell && c < max_cols) {
                            if (cell->type == MD_NODE_TABLE_CELL) {
                                tbl->rows[r][c] = extract_node_text(cell);
                                c++;
                            }
                            cell = cell->next;
                        }
                    }
                }
            }
        }
        free(rows);
    }
    
    free(nodes);
    return 0;
}

void md_tables_free(md_table_t *tables, size_t count) {
    if (!tables) return;
    for (size_t i = 0; i < count; i++) {
        if (tables[i].rows) {
            for (size_t r = 0; r < tables[i].row_count; r++) {
                if (tables[i].rows[r]) {
                    for (size_t c = 0; c < tables[i].col_count; c++) {
                        free(tables[i].rows[r][c]);
                    }
                    free(tables[i].rows[r]);
                }
            }
            free(tables[i].rows);
        }
        free(tables[i].aligns);
    }
    free(tables);
}

static void measure_text_len(const md_node_t *node, size_t *len) {
    if (!node) return;
    
    if (node->content && node->content_len > 0) {
        *len += node->content_len;
    }
    
    const md_node_t *child = node->first_child;
    while (child) {
        measure_text_len(child, len);
        child = child->next;
    }
}

static void copy_text_content(const md_node_t *node, char *buffer, size_t *pos) {
    if (!node) return;
    
    if (node->content && node->content_len > 0) {
        memcpy(buffer + *pos, node->content, node->content_len);
        *pos += node->content_len;
    }
    
    const md_node_t *child = node->first_child;
    while (child) {
        copy_text_content(child, buffer, pos);
        child = child->next;
    }
}

char *md_extract_plain_text(const md_document_t *doc) {
    if (!doc) return NULL;
    
    size_t len = 0;
    measure_text_len(doc->root, &len);
    
    char *result = (char *)malloc(len + 1);
    if (!result) return NULL;
    
    size_t pos = 0;
    copy_text_content(doc->root, result, &pos);
    result[pos] = '\0';
    
    return result;
}
