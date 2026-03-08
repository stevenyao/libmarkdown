#ifndef MARKDOWN_ERROR_H
#define MARKDOWN_ERROR_H

#include "export.h"

typedef enum {
    MD_ERROR_NONE = 0,
    MD_ERROR_MEMORY,
    MD_ERROR_FILE_OPEN,
    MD_ERROR_FILE_READ,
    MD_ERROR_INVALID_UTF8,
    MD_ERROR_SYNTAX,
    MD_ERROR_MAX_NESTING,
    MD_ERROR_INTERNAL
} md_error_t;

MARKDOWN_API const char *md_error_string(md_error_t error);

typedef struct {
    md_error_t error;
    int line;
    int column;
    char message[256];
} md_error_info_t;

MARKDOWN_API void md_error_init(md_error_info_t *err);
MARKDOWN_API void md_error_set(md_error_info_t *err, md_error_t error, int line, int column, const char *msg);

#endif
