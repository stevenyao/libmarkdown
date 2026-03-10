#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include "export.h"
#include "ast.h"
#include "error.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct md_parser_options {
    bool parse_yaml_front_matter;
    bool parse_footnotes;
    bool parse_table;
    bool parse_task_list;
    bool parse_strikethrough;
    bool parse_autolink;
    bool parse_html;
    size_t max_nesting;
} md_parser_options_t;

#define MD_PARSER_OPTIONS_DEFAULT { \
    .parse_yaml_front_matter = false, \
    .parse_footnotes = false,         \
    .parse_table = true,              \
    .parse_task_list = true,          \
    .parse_strikethrough = true,      \
    .parse_autolink = true,           \
    .parse_html = true,               \
    .max_nesting = 16                 \
}

typedef struct md_parser md_parser_t;

MARKDOWN_API md_parser_t *md_parser_create(const md_parser_options_t *opts);
MARKDOWN_API void md_parser_destroy(md_parser_t *parser);

MARKDOWN_API int md_parser_parse(md_parser_t *parser, const char *md, size_t len, md_document_t **doc);
MARKDOWN_API int md_parser_parse_file(md_parser_t *parser, const char *filepath, md_document_t **doc);

MARKDOWN_API md_document_t *md_parse(const char *md, size_t len);
MARKDOWN_API md_document_t *md_parse_file(const char *filepath);

MARKDOWN_API int md_parser_get_error(const md_parser_t *parser, md_error_info_t *err);

#endif
