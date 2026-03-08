#ifndef MARKDOWN_EXTRACTOR_H
#define MARKDOWN_EXTRACTOR_H

#include "ast.h"

typedef struct {
    int level;
    char *text;
    md_node_t *node;
} md_heading_t;

typedef struct {
    char *language;
    char *code;
    md_node_t *node;
} md_code_block_t;

typedef struct {
    char *text;
    char *url;
    char *title;
    md_node_t *node;
} md_link_t;

typedef struct {
    char *alt;
    char *url;
    md_node_t *node;
} md_image_t;

typedef struct {
    char ***rows;
    size_t row_count;
    size_t col_count;
    md_table_align_t *aligns;
    md_node_t *node;
} md_table_t;

int md_extract_headings(const md_document_t *doc, md_heading_t **headings, size_t *count);
void md_headings_free(md_heading_t *headings, size_t count);

int md_extract_code_blocks(const md_document_t *doc, md_code_block_t **blocks, size_t *count);
void md_code_blocks_free(md_code_block_t *blocks, size_t count);

int md_extract_links(const md_document_t *doc, md_link_t **links, size_t *count);
void md_links_free(md_link_t *links, size_t count);

int md_extract_images(const md_document_t *doc, md_image_t **images, size_t *count);
void md_images_free(md_image_t *images, size_t count);

int md_extract_tables(const md_document_t *doc, md_table_t **tables, size_t *count);
void md_tables_free(md_table_t *tables, size_t count);

char *md_extract_plain_text(const md_document_t *doc);

#endif
