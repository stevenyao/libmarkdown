#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "../include/markdown/parser.h"
#include "../include/markdown/document.h"

struct md_parser {
    md_parser_options_t opts;
    md_error_info_t error;
    int parse_error;
};

md_parser_t *md_parser_create(const md_parser_options_t *opts) {
    md_parser_t *parser = (md_parser_t *)malloc(sizeof(md_parser_t));
    if (!parser) return NULL;
    
    if (opts) {
        parser->opts = *opts;
    } else {
        parser->opts = (md_parser_options_t)MD_PARSER_OPTIONS_DEFAULT;
    }
    
    md_error_init(&parser->error);
    parser->parse_error = 0;
    
    return parser;
}

void md_parser_destroy(md_parser_t *parser) {
    free(parser);
}

static md_document_t *create_document(const char *md, size_t len) {
    md_document_t *doc = (md_document_t *)malloc(sizeof(md_document_t));
    if (!doc) return NULL;
    
    if (len == SIZE_MAX) {
        free(doc);
        return NULL;
    }
    
    doc->source = (char *)malloc(len + 1);
    if (!doc->source) {
        free(doc);
        return NULL;
    }
    
    memcpy(doc->source, md, len);
    doc->source[len] = '\0';
    doc->source_len = len;
    
    doc->root = (md_node_t *)malloc(sizeof(md_node_t));
    if (!doc->root) {
        free(doc->source);
        free(doc);
        return NULL;
    }
    
    memset(doc->root, 0, sizeof(md_node_t));
    doc->root->type = MD_NODE_DOCUMENT;
    doc->link_refs = NULL;
    
    return doc;
}

static int is_atx_heading(const char *line, size_t len) {
    if (len < 2 || line[0] != '#') return 0;
    int level = 1;
    while (level < 6 && line[level] == '#') level++;
    if (level > 6) return 0;
    if (line[level] != ' ' && line[level] != '\t') return 0;
    return level;
}

static int is_thematic_break(const char *line, size_t len) {
    if (len < 3) return 0;
    char c = line[0];
    if (c != '-' && c != '*' && c != '_') return 0;
    int count = 0;
    for (size_t i = 0; i < len; i++) {
        if (line[i] == c) count++;
        else if (line[i] != ' ' && line[i] != '\t') return 0;
    }
    return count >= 3;
}

static int is_setext_underline(const char *line, size_t len) {
    if (len < 3) return 0;
    char c = line[0];
    if (c != '=' && c != '-') return 0;
    for (size_t i = 0; i < len; i++) {
        if (line[i] != c && line[i] != ' ' && line[i] != '\t') return 0;
    }
    return 1;
}

static int is_fenced_code_start(const char *line, size_t len) {
    if (len < 3) return 0;
    if (line[0] != '`' && line[0] != '~') return 0;
    size_t count = 1;
    while (count < len && line[count] == line[0]) count++;
    return count >= 3;
}

static int is_block_quote(const char *line, size_t len) {
    return len > 0 && line[0] == '>';
}

static int is_indented_code(const char *line, size_t len) {
    if (len < 4) return 0;
    return (line[0] == ' ' && line[1] == ' ' && line[2] == ' ' && line[3] == ' ') || line[0] == '\t';
}

static int is_list_marker(const char *line, size_t len) {
    if (len < 2) return 0;
    if (line[0] == '-' || line[0] == '*' || line[0] == '+') {
        if (len > 1 && (line[1] == ' ' || line[1] == '\t')) return 1;
        if (len > 2 && line[1] == '[' && (line[2] == ' ' || line[2] == 'x' || line[2] == 'X')) return 1;
    }
    if (line[0] >= '0' && line[0] <= '9') {
        for (size_t i = 1; i < len; i++) {
            if (line[i] == '.' || line[i] == ')') {
                if (i + 1 < len && (line[i+1] == ' ' || line[i+1] == '\t')) return 1;
                break;
            }
            if (line[i] < '0' || line[i] > '9') break;
        }
    }
    return 0;
}

static md_node_t *create_block_node(md_node_type_t type, const char *content, size_t len) {
    md_node_t *node = (md_node_t *)malloc(sizeof(md_node_t));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(md_node_t));
    node->type = type;
    
    if (content && len > 0) {
        if (len == SIZE_MAX) {
            free(node);
            return NULL;
        }
        node->content = (char *)malloc(len + 1);
        if (!node->content) {
            free(node);
            return NULL;
        }
        memcpy(node->content, content, len);
        node->content[len] = '\0';
        node->content_len = len;
    }
    
    return node;
}

static char *trim_string(const char *str, size_t len) {
    while (len > 0 && (str[0] == ' ' || str[0] == '\t')) { str++; len--; }
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '\t')) len--;
    
    if (len == SIZE_MAX) {
        return NULL;
    }
    
    char *result = (char *)malloc(len + 1);
    if (result) {
        memcpy(result, str, len);
        result[len] = '\0';
    }
    return result;
}

static void parse_inline_text(md_node_t *parent, const char *text, size_t len) {
    size_t i = 0;
    size_t start = 0;
    
    while (i < len) {
        // Image ![alt](url) or Link [text](url)
        if (text[i] == '!' || text[i] == '[') {
            int is_image = (text[i] == '!');
            size_t bracket_start = is_image ? i + 1 : i;
            
            if (is_image && (i + 1 >= len || text[i+1] != '[')) {
                i++; continue;
            }
            
            size_t bracket_end = bracket_start + 1;
            while (bracket_end < len && text[bracket_end] != ']') bracket_end++;
            
            if (bracket_end < len && bracket_end + 1 < len && text[bracket_end + 1] == '(') {
                size_t paren_start = bracket_end + 1;
                size_t paren_end = paren_start + 1;
                while (paren_end < len && text[paren_end] != ')') paren_end++;
                
                if (paren_end < len) {
                    if (start < i) {
                         md_node_t *txt = create_block_node(MD_NODE_TEXT, text + start, i - start);
                         md_node_add_child(parent, txt);
                    }
                    
                    size_t content_len = bracket_end - bracket_start - 1;
                    size_t url_len = paren_end - paren_start - 1;
                    
                    char *content_str = (char*)malloc(content_len + 1);
                    if (content_len > 0) memcpy(content_str, text + bracket_start + 1, content_len);
                    content_str[content_len] = 0;
                    
                    char *url_str = (char*)malloc(url_len + 1);
                    if (url_len > 0) memcpy(url_str, text + paren_start + 1, url_len);
                    url_str[url_len] = 0;
                    
                    md_node_t *node = create_block_node(is_image ? MD_NODE_IMAGE : MD_NODE_LINK, content_str, content_len);
                    free(content_str);
                    
                    if (is_image) {
                        node->data.inline_.alt = node->content ? strdup(node->content) : NULL;
                        node->data.inline_.url = url_str;
                    } else {
                        node->data.inline_.url = url_str;
                    }
                    
                    md_node_add_child(parent, node);
                    
                    i = paren_end + 1;
                    start = i;
                    continue;
                }
            }
        }
        
        // Code Span `
        if (text[i] == '`') {
             size_t code_end = i + 1;
             while (code_end < len && text[code_end] != '`') code_end++;
             
             if (code_end < len) {
                 if (start < i) {
                     md_node_t *txt = create_block_node(MD_NODE_TEXT, text + start, i - start);
                     md_node_add_child(parent, txt);
                 }
                 
                 size_t code_len = code_end - i - 1;
                 md_node_t *code = create_block_node(MD_NODE_CODE_SPAN, text + i + 1, code_len);
                 md_node_add_child(parent, code);
                 
                 i = code_end + 1;
                 start = i;
                 continue;
             }
        }
        
        i++;
    }
    
    if (start < len) {
        md_node_t *txt = create_block_node(MD_NODE_TEXT, text + start, len - start);
        md_node_add_child(parent, txt);
    }
}

static void parse_inlines_recursive(md_node_t *node) {
    if (!node) return;
    
    if (node->content && node->content_len > 0 && 
        (node->type == MD_NODE_PARAGRAPH || node->type == MD_NODE_HEADING)) {
        
        int has_trigger = 0;
        for (size_t i = 0; i < node->content_len; i++) {
            if (node->content[i] == '[' || node->content[i] == '`' || node->content[i] == '!') {
                has_trigger = 1;
                break;
            }
        }
        
        if (has_trigger) {
            char *orig_content = node->content;
            size_t orig_len = node->content_len;
            
            node->content = NULL;
            node->content_len = 0;
            
            parse_inline_text(node, orig_content, orig_len);
            
            free(orig_content);
        }
    }
    
    md_node_t *child = node->first_child;
    while (child) {
        parse_inlines_recursive(child);
        child = child->next;
    }
}

int md_parser_parse(md_parser_t *parser, const char *md, size_t len, md_document_t **out_doc) {
    if (!parser || !md || !out_doc) return -1;
    
    *out_doc = NULL;
    
    md_document_t *doc = create_document(md, len);
    if (!doc) {
        parser->parse_error = 1;
        md_error_set(&parser->error, MD_ERROR_MEMORY, 1, 1, "Failed to create document");
        return -1;
    }
    
    md_node_t *root = doc->root;
    md_node_t *last_block = NULL;
    
    const char *line_start = md;
    int line_num = 1;
    
    const char *md_end = md + len;
    if (md_end < md) {
        md_document_free(doc);
        parser->parse_error = 1;
        md_error_set(&parser->error, MD_ERROR_MEMORY, 1, 1, "Input too large");
        return -1;
    }
    
    while (line_start < md_end) {
        const char *line_end = line_start;
        while (line_end < md_end && *line_end != '\n' && *line_end != '\r') line_end++;
        size_t line_len = line_end - line_start;
        
        if (line_len == SIZE_MAX) {
            md_document_free(doc);
            parser->parse_error = 1;
            md_error_set(&parser->error, MD_ERROR_MEMORY, line_num, 1, "Line too long");
            return -1;
        }
        
        char *line_copy = (char *)malloc(line_len + 1);
        if (!line_copy) {
            md_document_free(doc);
            parser->parse_error = 1;
            md_error_set(&parser->error, MD_ERROR_MEMORY, line_num, 1, "Failed to allocate line buffer");
            return -1;
        }
        memcpy(line_copy, line_start, line_len);
        line_copy[line_len] = '\0';
        
        int heading_level = is_atx_heading(line_copy, line_len);
        
        if (heading_level > 0) {
            char *title = trim_string(line_copy + heading_level + 1, line_len - heading_level - 1);
            md_node_t *heading = create_block_node(MD_NODE_HEADING, title, strlen(title));
            if (heading) {
                heading->data.block.heading_level = heading_level;
                md_node_add_child(root, heading);
                last_block = heading;
            }
            free(title);
        } else if (is_thematic_break(line_copy, line_len)) {
            md_node_t *hr = create_block_node(MD_NODE_THEMATIC_BREAK, NULL, 0);
            if (hr) {
                md_node_add_child(root, hr);
                last_block = hr;
            }
        } else if (is_setext_underline(line_copy, line_len)) {
            if (last_block && last_block->type == MD_NODE_PARAGRAPH) {
                last_block->type = MD_NODE_HEADING;
                last_block->data.block.heading_level = (line_copy[0] == '=') ? 1 : 2;
            }
            last_block = NULL;
        } else if (is_fenced_code_start(line_copy, line_len)) {
            char fence_char = line_copy[0];
            size_t fence_len = 0;
            while (fence_len < line_len && line_copy[fence_len] == fence_char) fence_len++;
            
            char *lang = NULL;
            char *info = line_copy + fence_len;
            while (*info == ' ' || *info == '\t') info++;
            size_t info_len = line_len - fence_len - (info - line_copy);
            if (info_len > 0) {
                lang = trim_string(info, info_len);
            }
            
            md_node_t *code = create_block_node(MD_NODE_FENCED_CODE, NULL, 0);
            if (code) {
                code->data.block.language = lang;
                md_node_add_child(root, code);
                last_block = code;
                
                char *code_content = NULL;
                size_t code_capacity = 0;
                size_t code_len = 0;
                
                line_start = line_end;
                if (line_end < md + len && *line_end == '\r' && line_end + 1 < md + len && line_end[1] == '\n') {
                    line_start += 2;
                } else if (line_end < md + len && (*line_end == '\n' || *line_end == '\r')) {
                    line_start++;
                }
                line_num++;
                
                while (line_start < md_end) {
                    const char *content_end = line_start;
                    while (content_end < md_end && *content_end != '\n' && *content_end != '\r') content_end++;
                    size_t content_len = content_end - line_start;
                    
                    char *content_line = (char *)malloc(content_len + 1);
                    if (!content_line) {
                        free(code_content);
                        free(line_copy);
                        md_document_free(doc);
                        parser->parse_error = 1;
                        md_error_set(&parser->error, MD_ERROR_MEMORY, line_num, 1, "Failed to allocate code line");
                        return -1;
                    }
                    memcpy(content_line, line_start, content_len);
                    content_line[content_len] = '\0';
                    
                    if (content_len >= fence_len) {
                        int is_closing = 1;
                        for (size_t i = 0; i < fence_len; i++) {
                            if (content_line[i] != fence_char) {
                                is_closing = 0;
                                break;
                            }
                        }
                        if (is_closing) {
                            int only_fence = 1;
                            for (size_t i = fence_len; i < content_len; i++) {
                                if (content_line[i] != ' ' && content_line[i] != '\t') {
                                    only_fence = 0;
                                    break;
                                }
                            }
                            if (only_fence) {
                                free(content_line);
                                line_start = content_end;
                                if (content_end < md_end && *content_end == '\r' && content_end + 1 < md_end && content_end[1] == '\n') {
                                    line_start += 2;
                                } else if (content_end < md_end && (*content_end == '\n' || *content_end == '\r')) {
                                    line_start++;
                                }
                                line_num++;
                                break;
                            }
                        }
                    }
                    
                    size_t new_len = code_len + content_len + 1;
                    if (new_len > code_capacity) {
                        size_t new_capacity = code_capacity ? code_capacity * 2 : 256;
                        if (new_capacity < new_len) new_capacity = new_len;
                        char *new_content = (char *)realloc(code_content, new_capacity);
                        if (!new_content) {
                            free(content_line);
                            free(code_content);
                            free(line_copy);
                            md_document_free(doc);
                            parser->parse_error = 1;
                            md_error_set(&parser->error, MD_ERROR_MEMORY, line_num, 1, "Failed to allocate code content");
                            return -1;
                        }
                        code_content = new_content;
                        code_capacity = new_capacity;
                    }
                    
                    if (code_len > 0) {
                        code_content[code_len] = '\n';
                        code_len++;
                    }
                    memcpy(code_content + code_len, line_start, content_len);
                    code_len += content_len;
                    
                    free(content_line);
                    
                    line_start = content_end;
                    if (content_end < md_end && *content_end == '\r' && content_end + 1 < md_end && content_end[1] == '\n') {
                        line_start += 2;
                    } else if (content_end < md_end && (*content_end == '\n' || *content_end == '\r')) {
                        line_start++;
                    }
                    line_num++;
                }
                
                if (code_content) {
                    code->content = code_content;
                    code->content_len = code_len;
                }
            }
            
            free(line_copy);
            continue;
        } else if (is_block_quote(line_copy, line_len)) {
            char *quote_content = trim_string(line_copy + 1, line_len - 1);
            md_node_t *quote = create_block_node(MD_NODE_BLOCKQUOTE, quote_content, strlen(quote_content));
            if (quote) {
                md_node_add_child(root, quote);
                last_block = quote;
            }
            free(quote_content);
        } else if (is_list_marker(line_copy, line_len)) {
            md_node_type_t list_type = MD_NODE_LIST_ITEM;
            char marker = line_copy[0];
            int is_task = 0;
            int is_checked = 0;
            
            if (line_copy[0] >= '0' && line_copy[0] <= '9') {
                list_type = MD_NODE_LIST_ITEM;
            } else if (line_len > 2 && line_copy[1] == '[' && line_copy[3] == ']') {
                list_type = MD_NODE_TASK_LIST_ITEM;
                is_task = 1;
                if (line_copy[2] == 'x' || line_copy[2] == 'X') is_checked = 1;
            } else {
                char *ptr = line_copy + 1;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                if (*ptr == '[' && ptr[1] != '\0' && ptr[2] == ']') {
                    list_type = MD_NODE_TASK_LIST_ITEM;
                    is_task = 1;
                    if (ptr[1] == 'x' || ptr[1] == 'X') is_checked = 1;
                }
            }
            
            md_node_t *item = create_block_node(list_type, line_copy, line_len);
            if (item) {
                item->data.block.list_marker = marker;
                item->data.block.is_task = is_task;
                item->data.block.is_checked = is_checked;
                
                if (!last_block || last_block->type != MD_NODE_LIST) {
                    md_node_t *list = create_block_node(MD_NODE_LIST, NULL, 0);
                    if (list) {
                        md_node_add_child(root, list);
                        last_block = list;
                    }
                }
                
                if (last_block && last_block->type == MD_NODE_LIST) {
                    md_node_add_child(last_block, item);
                } else {
                    md_node_add_child(root, item);
                }
                last_block = item;
            }
        } else if (is_indented_code(line_copy, line_len)) {
            md_node_t *code;
            if (last_block && last_block->type == MD_NODE_INDENTED_CODE) {
                code = last_block;
                size_t old_len = code->content_len;
                size_t new_len = old_len + 1 + line_len;
                char *new_content = (char *)realloc(code->content, new_len + 1);
                if (new_content) {
                    new_content[old_len] = '\n';
                    memcpy(new_content + old_len + 1, line_copy, line_len);
                    new_content[new_len] = '\0';
                    code->content = new_content;
                    code->content_len = new_len;
                }
            } else {
                char *code_content = trim_string(line_copy, line_len);
                code = create_block_node(MD_NODE_INDENTED_CODE, code_content, strlen(code_content));
                if (code) {
                    md_node_add_child(root, code);
                    last_block = code;
                }
                free(code_content);
            }
        } else if (line_len == 0) {
            last_block = NULL;
        } else {
            md_node_t *para;
            if (last_block && last_block->type == MD_NODE_PARAGRAPH) {
                para = last_block;
                size_t old_len = para->content_len;
                size_t new_len = old_len + 1 + line_len;
                char *new_content = (char *)realloc(para->content, new_len + 1);
                if (new_content) {
                    new_content[old_len] = ' ';
                    memcpy(new_content + old_len + 1, line_copy, line_len);
                    new_content[new_len] = '\0';
                    para->content = new_content;
                    para->content_len = new_len;
                }
            } else {
                char *para_content = trim_string(line_copy, line_len);
                para = create_block_node(MD_NODE_PARAGRAPH, para_content, strlen(para_content));
                if (para) {
                    md_node_add_child(root, para);
                    last_block = para;
                }
                free(para_content);
            }
        }
        
        free(line_copy);
        
        if (line_end < md + len) {
            if (*line_end == '\r' && line_end + 1 < md + len && line_end[1] == '\n') {
                line_end += 2;
            } else if (*line_end == '\n' || *line_end == '\r') {
                line_end++;
            }
        }
        line_start = line_end;
        line_num++;
    }
    
    *out_doc = doc;
    
    // Parse inline elements (links, code spans, etc.)
    parse_inlines_recursive(doc->root);
    
    return 0;
}

int md_parser_parse_file(md_parser_t *parser, const char *filepath, md_document_t **doc) {
    if (!parser || !filepath || !doc) return -1;
    
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        parser->parse_error = 1;
        md_error_set(&parser->error, MD_ERROR_FILE_OPEN, 1, 1, filepath);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (size < 0) {
        fclose(fp);
        parser->parse_error = 1;
        md_error_set(&parser->error, MD_ERROR_FILE_READ, 1, 1, filepath);
        return -1;
    }
    
    char *content = (char *)malloc(size + 1);
    if (!content) {
        fclose(fp);
        parser->parse_error = 1;
        md_error_set(&parser->error, MD_ERROR_MEMORY, 1, 1, NULL);
        return -1;
    }
    
    size_t read = fread(content, 1, size, fp);
    fclose(fp);
    
    content[read] = '\0';
    
    int ret = md_parser_parse(parser, content, read, doc);
    free(content);
    
    return ret;
}

md_document_t *md_parse(const char *md, size_t len) {
    md_parser_options_t opts = MD_PARSER_OPTIONS_DEFAULT;
    md_parser_t *parser = md_parser_create(&opts);
    if (!parser) return NULL;
    
    md_document_t *doc = NULL;
    md_parser_parse(parser, md, len, &doc);
    md_parser_destroy(parser);
    
    return doc;
}

md_document_t *md_parse_file(const char *filepath) {
    md_parser_options_t opts = MD_PARSER_OPTIONS_DEFAULT;
    md_parser_t *parser = md_parser_create(&opts);
    if (!parser) return NULL;
    
    md_document_t *doc = NULL;
    md_parser_parse_file(parser, filepath, &doc);
    md_parser_destroy(parser);
    
    return doc;
}

int md_parser_get_error(const md_parser_t *parser, md_error_info_t *err) {
    if (!parser || !err) return -1;
    *err = parser->error;
    return parser->parse_error ? 0 : -1;
}
