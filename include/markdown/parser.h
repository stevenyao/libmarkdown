#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include "ast.h"
#include "error.h"
#include <stdio.h>

typedef struct md_parser_options {
    int parse_yaml_front_matter;
    int parse_footnotes;
    int parse_table;
    int parse_task_list;
    int parse_strikethrough;
    int parse_autolink;
    int parse_html;
    size_t max_nesting;
} md_parser_options_t;

#define MD_PARSER_OPTIONS_DEFAULT { \
    .parse_yaml_front_matter = 0,  \
    .parse_footnotes = 0,            \
    .parse_table = 1,               \
    .parse_task_list = 1,           \
    .parse_strikethrough = 1,       \
    .parse_autolink = 1,            \
    .parse_html = 1,                \
    .max_nesting = 16               \
}

typedef struct md_parser md_parser_t;

md_parser_t *md_parser_create(const md_parser_options_t *opts);
void md_parser_destroy(md_parser_t *parser);

int md_parser_parse(md_parser_t *parser, const char *md, size_t len, md_document_t **doc);
int md_parser_parse_file(md_parser_t *parser, const char *filepath, md_document_t **doc);

md_document_t *md_parse(const char *md, size_t len);
md_document_t *md_parse_file(const char *filepath);

int md_parser_get_error(const md_parser_t *parser, md_error_info_t *err);

#endif
