#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include "../include/markdown/error.h"

const char *md_error_string(md_error_t error) {
    switch (error) {
        case MD_ERROR_NONE: return "No error";
        case MD_ERROR_MEMORY: return "Memory allocation failed";
        case MD_ERROR_FILE_OPEN: return "Failed to open file";
        case MD_ERROR_FILE_READ: return "Failed to read file";
        case MD_ERROR_INVALID_UTF8: return "Invalid UTF-8 encoding";
        case MD_ERROR_SYNTAX: return "Syntax error";
        case MD_ERROR_MAX_NESTING: return "Maximum nesting depth exceeded";
        case MD_ERROR_INTERNAL: return "Internal error";
        default: return "Unknown error";
    }
}

void md_error_init(md_error_info_t *err) {
    if (!err) return;
    err->error = MD_ERROR_NONE;
    err->line = 0;
    err->column = 0;
    err->message[0] = '\0';
}

void md_error_set(md_error_info_t *err, md_error_t error, int line, int column, const char *msg) {
    if (!err) return;
    err->error = error;
    err->line = line;
    err->column = column;
    if (msg) {
        size_t len = sizeof(err->message) - 1;
        size_t msg_len = strlen(msg);
        if (msg_len > len) msg_len = len;
        memcpy(err->message, msg, msg_len);
        err->message[msg_len] = '\0';
    } else {
        err->message[0] = '\0';
    }
}
