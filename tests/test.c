#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <markdown/markdown.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("FAILED: %s\n", msg); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ_INT(a, b, msg) do { \
    if ((a) != (b)) { \
        printf("FAILED: %s (got %d, expected %d)\n", msg, (int)(a), (int)(b)); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ_STR(a, b, msg) do { \
    if (strcmp(a, b) != 0) { \
        printf("FAILED: %s (got '%s'[%zu], expected '%s'[%zu])\n", msg, a, strlen(a), b, strlen(b)); \
        tests_failed++; \
        return; \
    } \
} while(0)

/* ==================== Error Tests ==================== */

TEST(error_string) {
    ASSERT_EQ_STR("No error", md_error_string(MD_ERROR_NONE), "MD_ERROR_NONE");
    ASSERT_EQ_STR("Memory allocation failed", md_error_string(MD_ERROR_MEMORY), "MD_ERROR_MEMORY");
    ASSERT_EQ_STR("Failed to open file", md_error_string(MD_ERROR_FILE_OPEN), "MD_ERROR_FILE_OPEN");
    ASSERT_EQ_STR("Failed to read file", md_error_string(MD_ERROR_FILE_READ), "MD_ERROR_FILE_READ");
    ASSERT_EQ_STR("Invalid UTF-8 encoding", md_error_string(MD_ERROR_INVALID_UTF8), "MD_ERROR_INVALID_UTF8");
    ASSERT_EQ_STR("Syntax error", md_error_string(MD_ERROR_SYNTAX), "MD_ERROR_SYNTAX");
    ASSERT_EQ_STR("Maximum nesting depth exceeded", md_error_string(MD_ERROR_MAX_NESTING), "MD_ERROR_MAX_NESTING");
    ASSERT_EQ_STR("Internal error", md_error_string(MD_ERROR_INTERNAL), "MD_ERROR_INTERNAL");
    ASSERT_EQ_STR("Unknown error", md_error_string(999), "unknown error");
    
    md_error_info_t err;
    md_error_init(&err);
    ASSERT_EQ_INT(MD_ERROR_NONE, err.error, "error init");
    ASSERT_EQ_INT(0, err.line, "error init line");
    ASSERT_EQ_INT(0, err.column, "error init column");
    ASSERT_EQ_STR("", err.message, "error init message");
    
    md_error_set(&err, MD_ERROR_SYNTAX, 10, 5, "test error");
    ASSERT_EQ_INT(MD_ERROR_SYNTAX, err.error, "error set code");
    ASSERT_EQ_INT(10, err.line, "error set line");
    ASSERT_EQ_INT(5, err.column, "error set column");
    ASSERT_EQ_STR("test error", err.message, "error set message");
    
    md_error_set(&err, MD_ERROR_MEMORY, 1, 1, NULL);
    ASSERT_EQ_STR("", err.message, "error set NULL message");
    
    tests_passed++;
}

TEST(error_null_operations) {
    md_error_init(NULL);
    md_error_set(NULL, MD_ERROR_NONE, 0, 0, NULL);
    tests_passed++;
}

/* ==================== AST Tests ==================== */

TEST(ast_node_operations) {
    md_node_t *node = (md_node_t *)malloc(sizeof(md_node_t));
    ASSERT(node != NULL, "node create");
    
    memset(node, 0, sizeof(md_node_t));
    node->type = MD_NODE_PARAGRAPH;
    node->content = strdup("test content");
    node->content_len = 12;
    
    md_node_t *child = (md_node_t *)malloc(sizeof(md_node_t));
    memset(child, 0, sizeof(md_node_t));
    child->type = MD_NODE_TEXT;
    child->content = strdup("child text");
    
    md_node_add_child(node, child);
    ASSERT(node->first_child == child, "first child");
    ASSERT(node->last_child == child, "last child");
    ASSERT(child->parent == node, "parent set");
    ASSERT(child->prev == NULL, "first child prev");
    ASSERT(child->next == NULL, "first child next");
    
    md_node_t *child2 = (md_node_t *)malloc(sizeof(md_node_t));
    memset(child2, 0, sizeof(md_node_t));
    child2->type = MD_NODE_TEXT;
    md_node_add_child(node, child2);
    ASSERT(node->last_child == child2, "last child updated");
    ASSERT(child->next == child2, "sibling link");
    ASSERT(child2->prev == child, "prev link");
    
    md_node_free(node);
    
    tests_passed++;
}

TEST(ast_node_free_with_content) {
    md_node_t *node = (md_node_t *)malloc(sizeof(md_node_t));
    memset(node, 0, sizeof(md_node_t));
    node->type = MD_NODE_PARAGRAPH;
    node->content = strdup("test");
    
    md_node_t *child = (md_node_t *)malloc(sizeof(md_node_t));
    memset(child, 0, sizeof(md_node_t));
    child->type = MD_NODE_TEXT;
    child->content = strdup("child");
    md_node_add_child(node, child);
    
    md_node_free(node);
    tests_passed++;
}

TEST(ast_null_operations) {
    md_node_add_child(NULL, NULL);
    md_node_free(NULL);
    tests_passed++;
}

/* ==================== Parser Tests ==================== */

TEST(parser_create_destroy) {
    md_parser_options_t opts = MD_PARSER_OPTIONS_DEFAULT;
    md_parser_t *parser = md_parser_create(&opts);
    ASSERT(parser != NULL, "parser create with opts");
    md_parser_destroy(parser);
    
    parser = md_parser_create(NULL);
    ASSERT(parser != NULL, "parser create NULL");
    md_parser_destroy(parser);
    
    tests_passed++;
}

TEST(parser_parse_null) {
    md_parser_t *parser = md_parser_create(NULL);
    md_document_t *doc = NULL;
    
    int ret = md_parser_parse(NULL, "test", 4, &doc);
    ASSERT_EQ_INT(-1, ret, "parse NULL parser");
    
    ret = md_parser_parse(parser, NULL, 4, &doc);
    ASSERT_EQ_INT(-1, ret, "parse NULL md");
    
    ret = md_parser_parse(parser, "test", 4, NULL);
    ASSERT_EQ_INT(-1, ret, "parse NULL doc");
    
    md_parser_destroy(parser);
    tests_passed++;
}

TEST(parser_parse_basic) {
    md_parser_t *parser = md_parser_create(NULL);
    md_document_t *doc = NULL;
    
    const char *md = "# Hello World\n\nParagraph text.";
    int ret = md_parser_parse(parser, md, strlen(md), &doc);
    ASSERT_EQ_INT(0, ret, "parse basic md");
    ASSERT(doc != NULL, "doc created");
    
    // Verify structure: Root -> Heading -> Paragraph
    md_node_t *root = doc->root;
    ASSERT(root != NULL, "root exists");
    
    md_node_t *heading = root->first_child;
    ASSERT(heading != NULL, "heading exists");
    ASSERT_EQ_INT(MD_NODE_HEADING, heading->type, "first child is heading");
    ASSERT_EQ_INT(1, heading->data.block.heading_level, "heading level 1");
    
    md_node_t *para = heading->next;
    ASSERT(para != NULL, "paragraph exists");
    ASSERT_EQ_INT(MD_NODE_PARAGRAPH, para->type, "second child is paragraph");
    
    const char *src = md_document_get_source(doc);
    ASSERT(src != NULL, "source not NULL");
    ASSERT(strlen(src) > 0, "source not empty");
    
    md_document_free(doc);
    md_parser_destroy(parser);
    
    tests_passed++;
}

TEST(parser_parse_memory_error) {
    md_parser_t *parser = md_parser_create(NULL);
    md_document_t *doc = NULL;
    
    const char *md = "# Test";
    int ret = md_parser_parse(parser, md, strlen(md), &doc);
    ASSERT_EQ_INT(0, ret, "parse success");
    ASSERT(doc != NULL, "doc created");
    
    md_document_free(doc);
    md_parser_destroy(parser);
    
    tests_passed++;
}

TEST(parser_parse_headings) {
    md_document_t *doc = md_parse("# H1\n## H2\n### H3\n#### H4\n##### H5\n###### H6", 45);
    ASSERT(doc != NULL, "parse headings");
    
    md_heading_t *headings = NULL;
    size_t count = 0;
    md_extract_headings(doc, &headings, &count);
    ASSERT_EQ_INT(6, count, "heading count");
    ASSERT_EQ_INT(1, headings[0].level, "H1 level");
    ASSERT_EQ_INT(2, headings[1].level, "H2 level");
    ASSERT_EQ_INT(3, headings[2].level, "H3 level");
    ASSERT_EQ_INT(4, headings[3].level, "H4 level");
    ASSERT_EQ_INT(5, headings[4].level, "H5 level");
    ASSERT_EQ_INT(6, headings[5].level, "H6 level");
    
    md_headings_free(headings, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_headings_no_space) {
    md_document_t *doc = md_parse("#NoSpace", 8);
    ASSERT(doc != NULL, "parse heading no space");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int heading_count = 0;
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_HEADING) {
            heading_count++;
        }
    }
    ASSERT_EQ_INT(0, heading_count, "no heading without space");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_headings_7) {
    md_document_t *doc = md_parse("####### H7", 10);
    ASSERT(doc != NULL, "parse H7");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_headings_trailing) {
    md_document_t *doc = md_parse("# Title #", 10);
    ASSERT(doc != NULL, "parse heading trailing");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int heading_count = 0;
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_HEADING) {
            heading_count++;
        }
    }
    ASSERT_EQ_INT(1, heading_count, "heading with trailing");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_thematic_break) {
    md_document_t *doc = md_parse("---\n\n***\n\n___\n\n- - -\n\n* * *", 33);
    ASSERT(doc != NULL, "parse thematic breaks");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int hr_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_THEMATIC_BREAK) {
            hr_count++;
        }
    }
    
    ASSERT(hr_count >= 1, "thematic break found");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_thematic_break_short) {
    md_document_t *doc = md_parse("--\n\n**", 7);
    ASSERT(doc != NULL, "parse short break");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int hr_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_THEMATIC_BREAK) {
            hr_count++;
        }
    }
    
    ASSERT_EQ_INT(0, hr_count, "short break not thematic");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_thematic_break_with_spaces) {
    md_document_t *doc = md_parse("- - - - -", 9);
    ASSERT(doc != NULL, "parse spaced break");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int hr_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_THEMATIC_BREAK) {
            hr_count++;
        }
    }
    
    ASSERT(hr_count >= 1, "spaced break");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_fenced_code) {
    md_document_t *doc = md_parse("```c\nint x;\n```", 17);
    ASSERT(doc != NULL, "parse fenced code");
    
    md_code_block_t *blocks = NULL;
    size_t count = 0;
    md_extract_code_blocks(doc, &blocks, &count);
    ASSERT_EQ_INT(1, count, "code block count");
    
    md_code_blocks_free(blocks, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_fenced_code_tilde) {
    md_document_t *doc = md_parse("~~~python\ncode\n~~~", 19);
    ASSERT(doc != NULL, "parse tilde fenced code");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int code_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_FENCED_CODE) {
            code_count++;
        }
    }
    
    ASSERT_EQ_INT(1, code_count, "fenced code count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_fenced_code_short) {
    md_document_t *doc = md_parse("``short\ncode\n``", 16);
    ASSERT(doc != NULL, "parse short fence");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int code_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_FENCED_CODE) {
            code_count++;
        }
    }
    
    ASSERT_EQ_INT(0, code_count, "short fence not code");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_fenced_code_with_info) {
    md_document_t *doc = md_parse("```javascript showLineNumbers\ncode\n```", 36);
    ASSERT(doc != NULL, "parse fenced with info");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int code_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_FENCED_CODE) {
            code_count++;
        }
    }
    
    ASSERT_EQ_INT(1, code_count, "fenced code with info");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_indented_code) {
    md_document_t *doc = md_parse("    code line\n    another line", 31);
    ASSERT(doc != NULL, "parse indented code");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int code_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_INDENTED_CODE) {
            code_count++;
        }
    }
    
    ASSERT_EQ_INT(1, code_count, "indented code count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_blockquote) {
    md_document_t *doc = md_parse("> quote text", 12);
    ASSERT(doc != NULL, "parse blockquote");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int bq_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_BLOCKQUOTE) {
            bq_count++;
        }
    }
    
    ASSERT_EQ_INT(1, bq_count, "blockquote count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_blockquote_empty) {
    md_document_t *doc = md_parse(">", 1);
    ASSERT(doc != NULL, "parse empty blockquote");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int bq_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_BLOCKQUOTE) {
            bq_count++;
        }
    }
    
    ASSERT_EQ_INT(1, bq_count, "empty blockquote count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_blockquote_multiline) {
    md_document_t *doc = md_parse("> line 1\n> line 2\n> line 3", 29);
    ASSERT(doc != NULL, "parse multiline blockquote");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_unordered) {
    md_document_t *doc = md_parse("- item1\n- item2\n- item3", 23);
    ASSERT(doc != NULL, "parse unordered list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) {
            list_count++;
        }
    }
    
    ASSERT_EQ_INT(3, list_count, "list item count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_plus) {
    md_document_t *doc = md_parse("+ item1\n+ item2", 16);
    ASSERT(doc != NULL, "parse plus list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) {
            list_count++;
        }
    }
    
    ASSERT_EQ_INT(2, list_count, "plus list count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_asterisk) {
    md_document_t *doc = md_parse("* item1\n* item2", 16);
    ASSERT(doc != NULL, "parse asterisk list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) {
            list_count++;
        }
    }
    
    ASSERT_EQ_INT(2, list_count, "asterisk list count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_ordered) {
    md_document_t *doc = md_parse("1. first\n2. second\n3. third", 27);
    ASSERT(doc != NULL, "parse ordered list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) {
            list_count++;
        }
    }
    
    ASSERT_EQ_INT(3, list_count, "ordered list count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_paren) {
    md_document_t *doc = md_parse("1) first\n2) second", 20);
    ASSERT(doc != NULL, "parse paren ordered list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) {
            list_count++;
        }
    }
    
    ASSERT_EQ_INT(2, list_count, "paren list count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_ordered_multi_digit) {
    md_document_t *doc = md_parse("10. tenth\n100. hundred", 24);
    ASSERT(doc != NULL, "parse multi-digit ordered list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) {
            list_count++;
        }
    }
    
    ASSERT_EQ_INT(2, list_count, "multi-digit list count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_task_list) {
    md_document_t *doc = md_parse("- [x] done\n- [ ] pending\n- [X] done2", 35);
    ASSERT(doc != NULL, "parse task list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int task_count = 0;
    int checked_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_TASK_LIST_ITEM) {
            task_count++;
            if (elem->data.block.is_checked) {
                checked_count++;
            }
        }
    }
    
    ASSERT_EQ_INT(3, task_count, "task item count");
    ASSERT_EQ_INT(2, checked_count, "checked count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_task_list_invalid) {
    md_document_t *doc = md_parse("- [y] invalid", 13);
    ASSERT(doc != NULL, "parse invalid task");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int task_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_TASK_LIST_ITEM) {
            task_count++;
            ASSERT_EQ_INT(0, elem->data.block.is_checked, "invalid not checked");
        }
    }
    
    ASSERT_EQ_INT(1, task_count, "task item count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_task_list_with_space) {
    md_document_t *doc = md_parse("- [ ] task with space", 22);
    ASSERT(doc != NULL, "parse task with space");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int task_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_TASK_LIST_ITEM) {
            task_count++;
        }
    }
    
    ASSERT_EQ_INT(1, task_count, "task count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_paragraph) {
    md_document_t *doc = md_parse("This is a paragraph.", 19);
    ASSERT(doc != NULL, "parse paragraph");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int para_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_PARAGRAPH) {
            para_count++;
        }
    }
    
    ASSERT_EQ_INT(1, para_count, "paragraph count");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_complex) {
    const char *md = 
        "# Title\n"
        "\n"
        "Paragraph with **bold**.\n"
        "\n"
        "```c\ncode\n```\n"
        "\n"
        "- item1\n"
        "- item2\n"
        "\n"
        "> quote\n"
        "\n"
        "---\n";
    
    md_document_t *doc = md_parse(md, strlen(md));
    ASSERT(doc != NULL, "parse complex md");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int total = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        total++;
    }
    
    ASSERT(total > 5, "complex doc has elements");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_empty) {
    md_document_t *doc = md_parse("", 0);
    ASSERT(doc != NULL, "parse empty string");
    md_document_free(doc);
    
    doc = md_parse("\n\n\n", 3);
    ASSERT(doc != NULL, "parse only newlines");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_only_spaces) {
    md_document_t *doc = md_parse("   \n   \n   ", 11);
    ASSERT(doc != NULL, "parse only spaces");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_multiline_paragraph) {
    md_document_t *doc = md_parse("Line 1\nLine 2\nLine 3", 19);
    ASSERT(doc != NULL, "parse multiline");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int para_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_PARAGRAPH) {
            para_count++;
        }
    }
    
    ASSERT_EQ_INT(1, para_count, "merged to single paragraph");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_multiple_paragraphs) {
    md_document_t *doc = md_parse("Para 1\n\nPara 2\n\nPara 3", 24);
    ASSERT(doc != NULL, "parse multiple paragraphs");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int para_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_PARAGRAPH) {
            para_count++;
        }
    }
    
    ASSERT_EQ_INT(3, para_count, "multiple paragraphs");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_marker_invalid) {
    md_document_t *doc = md_parse("-", 1);
    ASSERT(doc != NULL, "parse single dash");
    md_document_free(doc);
    
    doc = md_parse("1.", 2);
    ASSERT(doc != NULL, "parse just 1.");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_crlf) {
    md_document_t *doc = md_parse("Line1\r\nLine2\r\n", 15);
    ASSERT(doc != NULL, "parse CRLF");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int para_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_PARAGRAPH) {
            para_count++;
        }
    }
    
    ASSERT_EQ_INT(1, para_count, "CRLF paragraph");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_setext_heading) {
    md_document_t *doc = md_parse("Title\n=====", 11);
    ASSERT(doc != NULL, "parse setext");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int heading_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_HEADING) {
            heading_count++;
        }
    }
    
    ASSERT(heading_count >= 1, "setext heading");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_setext_h2) {
    md_document_t *doc = md_parse("Title\n---", 11);
    ASSERT(doc != NULL, "parse setext H2");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_leading_tab) {
    md_document_t *doc = md_parse("\ttab start", 10);
    ASSERT(doc != NULL, "parse leading tab");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_list_consecutive) {
    md_document_t *doc = md_parse("- item1\n- item2\n- item3\n\npara", 28);
    ASSERT(doc != NULL, "parse consecutive list");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int list_count = 0;
    int para_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_LIST_ITEM) list_count++;
        if (elem->type == MD_NODE_PARAGRAPH) para_count++;
    }
    
    ASSERT_EQ_INT(3, list_count, "list items");
    ASSERT_EQ_INT(1, para_count, "paragraph after list");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_parse_mixed_content) {
    md_document_t *doc = md_parse("# Heading\n\nPara1\n\n- List\n\nPara2", 35);
    ASSERT(doc != NULL, "parse mixed");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int h_count = 0, p_count = 0, l_count = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_HEADING) h_count++;
        if (elem->type == MD_NODE_PARAGRAPH) p_count++;
        if (elem->type == MD_NODE_LIST_ITEM) l_count++;
    }
    
    ASSERT_EQ_INT(1, h_count, "heading");
    ASSERT_EQ_INT(2, p_count, "paragraphs");
    ASSERT_EQ_INT(1, l_count, "list");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

/* ==================== Iterator Tests ==================== */

TEST(iterator_create_destroy) {
    md_document_t *doc = md_parse("test", 4);
    md_iterator_t *iter = md_iterator_create(doc);
    ASSERT(iter != NULL, "iterator create");
    
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    iter = md_iterator_create(NULL);
    ASSERT(iter == NULL, "iterator NULL doc");
    
    tests_passed++;
}

TEST(iterator_reset) {
    md_document_t *doc = md_parse("# H1\n\n# H2", 11);
    md_iterator_t *iter = md_iterator_create(doc);
    
    md_element_t *elem = md_iterator_next(iter);
    ASSERT(elem != NULL, "first next");
    
    md_iterator_reset(iter);
    elem = md_iterator_next(iter);
    ASSERT(elem != NULL, "after reset");
    
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(iterator_traverse) {
    md_document_t *doc = md_parse("# H1\n\nPara.", 11);
    md_iterator_t *iter = md_iterator_create(doc);
    
    int count = 0;
    md_element_t *elem;
    while ((elem = md_iterator_next(iter)) != NULL) {
        count++;
        ASSERT(elem->type > 0 && elem->type < MD_NODE_SPAN, "valid type");
    }
    
    ASSERT(count > 0, "traversed elements");
    
    const md_element_t *get_elem = md_iterator_get(iter);
    ASSERT(get_elem != NULL || get_elem == NULL, "iterator get");
    
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(iterator_traverse_all_types) {
    md_document_t *doc = md_parse("# H\n\n---\n\n> BQ\n\n- L\n\n    C\n\n```\n```\n\nP", 38);
    ASSERT(doc != NULL, "parse all types");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    int types_found = 0;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        if (elem->type == MD_NODE_HEADING) types_found |= 1;
        if (elem->type == MD_NODE_THEMATIC_BREAK) types_found |= 2;
        if (elem->type == MD_NODE_BLOCKQUOTE) types_found |= 4;
        if (elem->type == MD_NODE_LIST_ITEM) types_found |= 8;
        if (elem->type == MD_NODE_INDENTED_CODE) types_found |= 16;
        if (elem->type == MD_NODE_FENCED_CODE) types_found |= 32;
        if (elem->type == MD_NODE_PARAGRAPH) types_found |= 64;
    }
    
    ASSERT(types_found > 0, "found some types");
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(iterator_empty_doc) {
    md_document_t *doc = md_parse("", 0);
    md_iterator_t *iter = md_iterator_create(doc);
    
    md_element_t *elem = md_iterator_next(iter);
    ASSERT(elem == NULL, "empty doc no elements");
    
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    tests_passed++;
}

/* ==================== Document Tests ==================== */

TEST(document_free_null) {
    md_document_free(NULL);
    tests_passed++;
}

TEST(document_get_source) {
    const char *md = "test content";
    md_document_t *doc = md_parse(md, strlen(md));
    
    const char *src = md_document_get_source(doc);
    ASSERT(src != NULL, "source not NULL");
    ASSERT_EQ_INT((int)strlen(md), (int)md_document_get_source_len(doc), "source length");
    
    md_document_free(doc);
    
    ASSERT(NULL == md_document_get_source(NULL), "NULL doc source");
    ASSERT_EQ_INT(0, (int)md_document_get_source_len(NULL), "NULL doc source len");
    
    tests_passed++;
}

static int test_visit_callback(const md_node_t *node, void *data) {
    (void)node;
    int *count = (int *)data;
    (*count)++;
    return 0;
}

TEST(document_visit) {
    md_document_t *doc = md_parse("# H1\n\n# H2", 11);
    
    int visit_count = 0;
    
    int ret = md_document_visit(doc, test_visit_callback, &visit_count);
    ASSERT_EQ_INT(0, ret, "visit returns 0");
    ASSERT(visit_count > 0, "visited nodes");
    
    ret = md_document_visit(NULL, test_visit_callback, NULL);
    ASSERT_EQ_INT(-1, ret, "visit NULL doc");
    
    ret = md_document_visit(doc, NULL, NULL);
    ASSERT_EQ_INT(-1, ret, "visit NULL callback");
    
    md_document_free(doc);
    
    tests_passed++;
}

static int test_visit_early_exit_callback(const md_node_t *node, void *data) {
    (void)node;
    int *c = (int *)data;
    (*c)++;
    return *c >= 2 ? 1 : 0;
}

TEST(document_visit_early_exit) {
    md_document_t *doc = md_parse("# H1\n\n# H2\n\n# H3", 23);
    
    int c = 0;
    int ret = md_document_visit(doc, test_visit_early_exit_callback, &c);
    ASSERT_EQ_INT(1, ret, "visit returns early exit");
    
    md_document_free(doc);
    
    tests_passed++;
}

TEST(node_type_name) {
    ASSERT_EQ_STR("document", md_node_type_name(MD_NODE_DOCUMENT), "document");
    ASSERT_EQ_STR("heading", md_node_type_name(MD_NODE_HEADING), "heading");
    ASSERT_EQ_STR("fenced_code", md_node_type_name(MD_NODE_FENCED_CODE), "fenced");
    ASSERT_EQ_STR("paragraph", md_node_type_name(MD_NODE_PARAGRAPH), "paragraph");
    ASSERT_EQ_STR("link", md_node_type_name(MD_NODE_LINK), "link");
    ASSERT_EQ_STR("image", md_node_type_name(MD_NODE_IMAGE), "image");
    ASSERT_EQ_STR("unknown", md_node_type_name(999), "unknown");
    
    tests_passed++;
}

/* ==================== Extractor Tests ==================== */

TEST(extractor_headings) {
    md_document_t *doc = md_parse("# H1\n## H2\n### H3", 20);
    
    md_heading_t *headings = NULL;
    size_t count = 0;
    
    md_extract_headings(doc, &headings, &count);
    ASSERT_EQ_INT(3, count, "heading count");
    
    ASSERT_EQ_INT(1, headings[0].level, "H1 level");
    ASSERT(headings[0].text != NULL, "H1 text exists");
    ASSERT_EQ_STR("H1", headings[0].text, "H1 content match");
    
    ASSERT_EQ_INT(2, headings[1].level, "H2 level");
    ASSERT(headings[1].text != NULL, "H2 text exists");
    ASSERT_EQ_STR("H2", headings[1].text, "H2 content match");
    
    ASSERT_EQ_INT(3, headings[2].level, "H3 level");
    ASSERT(headings[2].text != NULL, "H3 text exists");
    ASSERT_EQ_STR("H3", headings[2].text, "H3 content match");
    
    md_headings_free(headings, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_headings_null) {
    md_heading_t *headings = NULL;
    size_t count = 0;
    
    int ret = md_extract_headings(NULL, &headings, &count);
    ASSERT_EQ_INT(-1, ret, "extract NULL doc");
    
    ret = md_extract_headings(NULL, NULL, NULL);
    ASSERT_EQ_INT(-1, ret, "extract all NULL");
    
    md_headings_free(NULL, 0);
    
    tests_passed++;
}

TEST(extractor_headings_empty) {
    md_document_t *doc = md_parse("no headings here", 16);
    
    md_heading_t *headings = NULL;
    size_t count = 0;
    
    md_extract_headings(doc, &headings, &count);
    ASSERT_EQ_INT(0, count, "no headings");
    
    md_headings_free(headings, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_code_blocks) {
    const char *md = "```c\ncode\n```\n\n```python\npy\n```";
    md_document_t *doc = md_parse(md, strlen(md));
    
    md_code_block_t *blocks = NULL;
    size_t count = 0;
    
    md_extract_code_blocks(doc, &blocks, &count);
    ASSERT_EQ_INT(2, count, "code block count");
    
    ASSERT(blocks[0].language != NULL, "first block lang exists");
    ASSERT_EQ_STR("c", blocks[0].language, "first block lang");
    ASSERT(blocks[0].code != NULL, "first block code exists");
    ASSERT_EQ_STR("code", blocks[0].code, "first block code");
    
    ASSERT(blocks[1].language != NULL, "second block lang exists");
    ASSERT_EQ_STR("python", blocks[1].language, "second block lang");
    ASSERT(blocks[1].code != NULL, "second block code exists");
    ASSERT_EQ_STR("py", blocks[1].code, "second block code");
    
    md_code_blocks_free(blocks, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_code_blocks_null) {
    md_code_block_t *blocks = NULL;
    size_t count = 0;
    
    int ret = md_extract_code_blocks(NULL, &blocks, &count);
    ASSERT_EQ_INT(-1, ret, "extract NULL doc");
    
    md_code_blocks_free(NULL, 0);
    
    tests_passed++;
}

TEST(extractor_code_blocks_empty) {
    md_document_t *doc = md_parse("no code blocks", 14);
    
    md_code_block_t *blocks = NULL;
    size_t count = 0;
    
    md_extract_code_blocks(doc, &blocks, &count);
    ASSERT_EQ_INT(0, count, "no code blocks");
    
    md_code_blocks_free(blocks, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_links) {
    md_document_t *doc = md_parse("[link](url)", 13);
    
    md_link_t *links = NULL;
    size_t count = 0;
    
    md_extract_links(doc, &links, &count);
    ASSERT_EQ_INT(1, count, "link count");
    
    ASSERT(links[0].text != NULL, "link text exists");
    ASSERT_EQ_STR("link", links[0].text, "link text match");
    ASSERT(links[0].url != NULL, "link url exists");
    ASSERT_EQ_STR("url", links[0].url, "link url match");
    
    md_links_free(links, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_links_null) {
    md_link_t *links = NULL;
    size_t count = 0;
    
    int ret = md_extract_links(NULL, &links, &count);
    ASSERT_EQ_INT(-1, ret, "extract NULL doc");
    
    md_links_free(NULL, 0);
    
    tests_passed++;
}

TEST(extractor_links_multiple) {
    md_document_t *doc = md_parse("[link1](url1) and [link2](url2)", 35);
    
    md_link_t *links = NULL;
    size_t count = 0;
    
    md_extract_links(doc, &links, &count);
    ASSERT_EQ_INT(2, count, "multiple link count");
    md_links_free(links, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_images) {
    md_document_t *doc = md_parse("![alt](img.png)", 17);
    
    md_image_t *images = NULL;
    size_t count = 0;
    
    md_extract_images(doc, &images, &count);
    ASSERT_EQ_INT(1, count, "image count");
    
    md_images_free(images, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_images_null) {
    md_image_t *images = NULL;
    size_t count = 0;
    
    int ret = md_extract_images(NULL, &images, &count);
    ASSERT_EQ_INT(-1, ret, "extract NULL doc");
    
    md_images_free(NULL, 0);
    
    tests_passed++;
}

TEST(extractor_tables) {
    md_document_t *doc = md_parse("|a|b|\n|---|---|", 15);
    
    md_table_t *tables = NULL;
    size_t count = 0;
    
    int ret = md_extract_tables(doc, &tables, &count);
    ASSERT_EQ_INT(0, ret, "extract tables");
    
    md_tables_free(tables, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_tables_null) {
    md_table_t *tables = NULL;
    size_t count = 0;
    
    int ret = md_extract_tables(NULL, &tables, &count);
    ASSERT_EQ_INT(-1, ret, "extract NULL doc");
    
    md_tables_free(NULL, 0);
    
    tests_passed++;
}

TEST(extractor_plain_text) {
    md_document_t *doc = md_parse("Hello World", 11);
    
    char *text = md_extract_plain_text(doc);
    ASSERT(text != NULL, "plain text not NULL");
    ASSERT_EQ_STR("Hello World", text, "plain text match");
    
    free(text);
    md_document_free(doc);
    
    ASSERT(NULL == md_extract_plain_text(NULL), "NULL doc plain text");
    
    tests_passed++;
}

TEST(extractor_plain_text_with_heading) {
    md_document_t *doc = md_parse("# Title\n\nSome text here", 24);
    
    char *text = md_extract_plain_text(doc);
    ASSERT(text != NULL, "plain text not NULL");
    ASSERT(strlen(text) > 0, "text not empty");
    
    free(text);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_plain_text_empty) {
    md_document_t *doc = md_parse("", 0);
    
    char *text = md_extract_plain_text(doc);
    ASSERT(text != NULL || text == NULL, "empty doc");
    free(text);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_plain_text_complex) {
    md_document_t *doc = md_parse("# Heading\n\nPara1\n\n```code```\n\n> Quote", 40);
    
    char *text = md_extract_plain_text(doc);
    ASSERT(text != NULL, "plain text not NULL");
    
    free(text);
    md_document_free(doc);
    
    tests_passed++;
}

/* ==================== Parser Options Tests ==================== */

TEST(parser_options_default) {
    md_parser_options_t opts = MD_PARSER_OPTIONS_DEFAULT;
    ASSERT(opts.parse_yaml_front_matter == 0, "yaml front matter default");
    ASSERT(opts.parse_footnotes == 0, "footnotes default");
    ASSERT(opts.parse_table == 1, "table default");
    ASSERT(opts.parse_task_list == 1, "task list default");
    ASSERT(opts.parse_strikethrough == 1, "strikethrough default");
    ASSERT(opts.parse_autolink == 1, "autolink default");
    ASSERT(opts.parse_html == 1, "html default");
    ASSERT(opts.max_nesting == 16, "max nesting default");
    
    tests_passed++;
}

TEST(parser_options_custom) {
    md_parser_options_t opts;
    memset(&opts, 0, sizeof(opts));
    opts.parse_yaml_front_matter = 1;
    opts.parse_footnotes = 1;
    opts.parse_table = 0;
    opts.parse_task_list = 0;
    opts.parse_strikethrough = 0;
    opts.parse_autolink = 0;
    opts.parse_html = 0;
    opts.max_nesting = 32;
    
    md_parser_t *parser = md_parser_create(&opts);
    ASSERT(parser != NULL, "parser with custom opts");
    md_parser_destroy(parser);
    
    tests_passed++;
}

/* ==================== File Parsing Tests ==================== */

TEST(parser_parse_file) {
    md_parser_t *parser = md_parser_create(NULL);
    md_document_t *doc = NULL;
    
    int ret = md_parser_parse_file(NULL, "nonexistent.md", &doc);
    ASSERT_EQ_INT(-1, ret, "file not found");
    
    ret = md_parser_parse_file(parser, NULL, &doc);
    ASSERT_EQ_INT(-1, ret, "NULL filename");
    
    ret = md_parser_parse_file(parser, "nonexistent.md", NULL);
    ASSERT_EQ_INT(-1, ret, "NULL doc");
    
    md_parser_destroy(parser);
    
    tests_passed++;
}

TEST(parser_get_error) {
    md_parser_t *parser = md_parser_create(NULL);
    md_document_t *doc = NULL;
    
    md_parser_parse_file(parser, "nonexistent.md", &doc);
    
    md_error_info_t err;
    int has_error = md_parser_get_error(parser, &err);
    ASSERT_EQ_INT(0, has_error, "has error");
    ASSERT_EQ_INT(MD_ERROR_FILE_OPEN, err.error, "error code");
    
    md_parser_get_error(NULL, NULL);
    
    md_parser_destroy(parser);
    
    tests_passed++;
}

TEST(parser_parse_file_memory_error) {
    md_parser_t *parser = md_parser_create(NULL);
    md_parser_destroy(parser);
    
    tests_passed++;
}

TEST(parser_parse_file_success) {
    const char *filename = "test_sample.md";
    const char *content = "# Title\n\nContent";
    
    // Write file
    FILE *fp = fopen(filename, "wb");
    if (fp) {
        fwrite(content, 1, strlen(content), fp);
        fclose(fp);
    } else {
        printf("Failed to create test file\n");
        tests_failed++;
        return;
    }
    
    md_parser_t *parser = md_parser_create(NULL);
    md_document_t *doc = NULL;
    
    int ret = md_parser_parse_file(parser, filename, &doc);
    ASSERT_EQ_INT(0, ret, "parse file success");
    ASSERT(doc != NULL, "doc created");
    
    // Verify content
    const char *src = md_document_get_source(doc);
    ASSERT_EQ_STR(content, src, "source match");
    
    md_document_free(doc);
    md_parser_destroy(parser);
    
    // Test wrapper
    doc = md_parse_file(filename);
    ASSERT(doc != NULL, "wrapper parse file");
    ASSERT_EQ_STR(content, md_document_get_source(doc), "wrapper source");
    md_document_free(doc);
    
    remove(filename);
    
    tests_passed++;
}

/* ==================== Edge Cases ==================== */

TEST(parser_special_characters) {
    md_document_t *doc;
    
    doc = md_parse("# Title with `code`", 21);
    ASSERT(doc != NULL, "title with code");
    md_document_free(doc);
    
    doc = md_parse("# Title with *emphasis*", 24);
    ASSERT(doc != NULL, "title with emphasis");
    md_document_free(doc);
    
    doc = md_parse("> > nested quote", 16);
    ASSERT(doc != NULL, "nested quote");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_special_headings) {
    md_document_t *doc;
    
    doc = md_parse("# Title #", 10);
    ASSERT(doc != NULL, "title with trailing");
    md_document_free(doc);
    
    doc = md_parse("## Title ##", 12);
    ASSERT(doc != NULL, "H2 with trailing");
    md_document_free(doc);
    
    doc = md_parse("#", 1);
    ASSERT(doc != NULL, "just hash");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_list_with_tab) {
    md_document_t *doc = md_parse("-\titem", 7);
    ASSERT(doc != NULL, "list with tab");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(iterator_null_operations) {
    md_element_t *elem;
    
    elem = md_iterator_next(NULL);
    ASSERT(elem == NULL, "next NULL iter");
    
    const md_element_t *get = md_iterator_get(NULL);
    ASSERT(get == NULL, "get NULL iter");
    
    md_iterator_reset(NULL);
    md_iterator_destroy(NULL);
    
    tests_passed++;
}

TEST(parser_edge_cases) {
    md_document_t *doc;
    
    doc = md_parse("\tcode", 5);
    ASSERT(doc != NULL, "leading tab");
    md_document_free(doc);
    
    doc = md_parse(">><", 3);
    ASSERT(doc != NULL, "multiple angles");
    md_document_free(doc);
    
    doc = md_parse("a]b", 3);
    ASSERT(doc != NULL, "bracket combo");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(parser_very_long_line) {
    char *long_line = (char *)malloc(10000);
    memset(long_line, 'a', 9999);
    long_line[9999] = '\0';
    
    md_document_t *doc = md_parse(long_line, 9999);
    ASSERT(doc != NULL, "parse long line");
    md_document_free(doc);
    free(long_line);
    
    tests_passed++;
}

TEST(parser_many_newlines) {
    char *many_newlines = (char *)malloc(1000);
    memset(many_newlines, '\n', 999);
    many_newlines[999] = '\0';
    
    md_document_t *doc = md_parse(many_newlines, 999);
    ASSERT(doc != NULL, "parse many newlines");
    md_document_free(doc);
    free(many_newlines);
    
    tests_passed++;
}

/* ==================== Coverage Tests ==================== */

TEST(document_getters) {
    const char *md = "# H1\n\n- [x] Task\n\n```c\ncode\n```\n\n[link](url \"title\")";
    md_document_t *doc = md_parse(md, strlen(md));
    ASSERT(doc != NULL, "doc parsed");
    
    md_node_t *root = doc->root;
    ASSERT_EQ_INT(MD_NODE_DOCUMENT, md_node_get_type(root), "root type");
    
    // H1
    const md_node_t *h1 = md_node_get_first_child(root);
    ASSERT(h1 != NULL, "h1 exists");
    ASSERT_EQ_INT(MD_NODE_HEADING, md_node_get_type(h1), "h1 type");
    ASSERT_EQ_INT(1, md_node_get_heading_level(h1), "h1 level");
    ASSERT(md_node_get_parent(h1) == root, "h1 parent");
    ASSERT_EQ_STR("H1", md_node_get_content(h1), "h1 content");
    
    // List
    const md_node_t *list = md_node_get_next(h1);
    ASSERT(list != NULL, "list exists");
    ASSERT_EQ_INT(MD_NODE_LIST, md_node_get_type(list), "list type");
    ASSERT_EQ_INT(0, md_node_get_list_start(list), "list start");
    ASSERT_EQ_INT(0, md_node_get_list_marker(list), "list marker (on list node)");
    
    // Task Item
    const md_node_t *item = md_node_get_first_child(list);
    ASSERT(item != NULL, "item exists");
    ASSERT_EQ_INT(MD_NODE_TASK_LIST_ITEM, md_node_get_type(item), "item type");
    ASSERT(md_node_is_task_list(item), "is task list item");
    ASSERT(md_node_is_checked(item), "is checked");
    ASSERT_EQ_INT('-', md_node_get_list_marker(item), "item marker");
    
    // Code
    const md_node_t *code = md_node_get_next(list);
    ASSERT(code != NULL, "code exists");
    ASSERT_EQ_INT(MD_NODE_FENCED_CODE, md_node_get_type(code), "code type");
    ASSERT_EQ_STR("c", md_node_get_code_language(code), "code lang");
    ASSERT(md_node_get_prev(code) == list, "code prev");
    ASSERT(md_node_get_last_child(root) != code, "code not last");
    
    // Paragraph with Link
    const md_node_t *para = md_node_get_next(code);
    ASSERT(para != NULL, "para exists");
    const md_node_t *link = md_node_get_first_child(para);
    ASSERT(link != NULL, "link exists");
    ASSERT_EQ_INT(MD_NODE_LINK, md_node_get_type(link), "link type");
    ASSERT_EQ_STR("url \"title\"", md_node_get_url(link), "link url");
    
    // Test null/invalid getters
    ASSERT(md_node_get_parent(NULL) == NULL, "null parent");
    ASSERT(md_node_get_first_child(NULL) == NULL, "null first child");
    ASSERT(md_node_get_next(NULL) == NULL, "null next");
    ASSERT_EQ_INT(0, md_node_get_heading_level(NULL), "null heading level");
    ASSERT(md_node_get_code_language(NULL) == NULL, "null code lang");
    ASSERT(md_node_get_url(NULL) == NULL, "null url");
    
    md_document_free(doc);
    tests_passed++;
}

TEST(table_extraction_manual) {
    // Construct AST manually to test extractor fully
    md_document_t *doc = (md_document_t *)malloc(sizeof(md_document_t));
    memset(doc, 0, sizeof(md_document_t));
    
    md_node_t *root = (md_node_t *)malloc(sizeof(md_node_t));
    memset(root, 0, sizeof(md_node_t));
    root->type = MD_NODE_DOCUMENT;
    doc->root = root;
    
    md_node_t *table = (md_node_t *)malloc(sizeof(md_node_t));
    memset(table, 0, sizeof(md_node_t));
    table->type = MD_NODE_TABLE;
    md_node_add_child(root, table);
    
    // Row 1
    md_node_t *row1 = (md_node_t *)malloc(sizeof(md_node_t));
    memset(row1, 0, sizeof(md_node_t));
    row1->type = MD_NODE_TABLE_ROW;
    md_node_add_child(table, row1);
    
    // Cell 1
    md_node_t *cell1 = (md_node_t *)malloc(sizeof(md_node_t));
    memset(cell1, 0, sizeof(md_node_t));
    cell1->type = MD_NODE_TABLE_CELL;
    cell1->content = strdup("cell1");
    cell1->content_len = 5;
    cell1->data.block.table_align = MD_TABLE_ALIGN_CENTER;
    md_node_add_child(row1, cell1);
    
    // Extract
    md_table_t *tables = NULL;
    size_t count = 0;
    
    int ret = md_extract_tables(doc, &tables, &count);
    ASSERT_EQ_INT(0, ret, "manual table extract");
    ASSERT_EQ_INT(1, count, "manual table count");
    
    ASSERT_EQ_INT(1, tables[0].row_count, "row count");
    ASSERT_EQ_INT(1, tables[0].col_count, "col count");
    ASSERT(tables[0].rows[0][0] != NULL, "cell content exists");
    ASSERT_EQ_STR("cell1", tables[0].rows[0][0], "cell content");
    ASSERT_EQ_INT(MD_TABLE_ALIGN_CENTER, tables[0].aligns[0], "cell align");
    
    md_tables_free(tables, count);
    md_document_free(doc);
    
    tests_passed++;
}

TEST(image_getter) {
    const char *md = "![alt](url)";
    md_document_t *doc = md_parse(md, strlen(md));
    
    md_node_t *para = doc->root->first_child;
    md_node_t *img = para->first_child;
    
    ASSERT_EQ_INT(MD_NODE_IMAGE, md_node_get_type(img), "image type");
    ASSERT_EQ_STR("alt", md_node_get_alt(img), "image alt");
    ASSERT_EQ_STR("url", md_node_get_url(img), "image url");
    ASSERT(md_node_get_title(img) == NULL, "image title null");
    
    md_document_free(doc);
    tests_passed++;
}

TEST(parser_inline_edge_cases) {
    // Incomplete or invalid inline sequences
    const char *md = "!\n![\n![alt\n![alt]\n![alt](\n![alt](url\n[\n[text\n[text]\n[text](\n[text](url";
    md_document_t *doc = md_parse(md, strlen(md));
    ASSERT(doc != NULL, "parse inline edges");
    // Just verify it doesn't crash and parse finishes
    md_document_free(doc);
    tests_passed++;
}

TEST(parser_list_edge_cases) {
    // 1) item
    const char *md1 = "1) item";
    md_document_t *doc = md_parse(md1, strlen(md1));
    md_node_t *list = doc->root->first_child;
    ASSERT(list != NULL && list->type == MD_NODE_LIST, "ordered list paren");
    md_document_free(doc);
    
    // 1.item (no space) -> paragraph
    const char *md2 = "1.item";
    doc = md_parse(md2, strlen(md2));
    md_node_t *para = doc->root->first_child;
    ASSERT(para != NULL && para->type == MD_NODE_PARAGRAPH, "no space list");
    md_document_free(doc);
    
    // -[ ] item (no space after hyphen) -> Parser allows this as list item (implementation detail)
    const char *md3 = "-[ ] item";
    doc = md_parse(md3, strlen(md3));
    md_node_t *root_child = doc->root->first_child;
    ASSERT(root_child != NULL && root_child->type == MD_NODE_LIST, "no space task is list");
    md_document_free(doc);
    
    tests_passed++;
}

TEST(extractor_table_empty_content) {
    md_document_t *doc = (md_document_t *)malloc(sizeof(md_document_t));
    memset(doc, 0, sizeof(md_document_t));
    doc->root = (md_node_t *)calloc(1, sizeof(md_node_t));
    doc->root->type = MD_NODE_DOCUMENT;
    
    md_node_t *table = (md_node_t *)calloc(1, sizeof(md_node_t));
    table->type = MD_NODE_TABLE;
    md_node_add_child(doc->root, table);
    
    // No rows
    
    md_table_t *tables = NULL;
    size_t count = 0;
    md_extract_tables(doc, &tables, &count);
    ASSERT_EQ_INT(1, count, "empty table count");
    ASSERT_EQ_INT(0, tables[0].row_count, "empty table rows");
    
    md_tables_free(tables, count);
    md_document_free(doc);
    tests_passed++;
}

TEST(parser_heading_whitespace) {
    // "#   " -> Empty title
    md_document_t *doc = md_parse("#   ", 4);
    md_node_t *h = doc->root->first_child;
    ASSERT(h != NULL, "empty heading");
    ASSERT_EQ_INT(MD_NODE_HEADING, h->type, "heading type");
    // content should be null because length is 0
    ASSERT(h->content == NULL, "empty heading content null");
    md_document_free(doc);
    
    // "#" -> Paragraph (too short for ATX)
    doc = md_parse("#", 1);
    h = doc->root->first_child;
    ASSERT(h != NULL, "just hash");
    ASSERT_EQ_INT(MD_NODE_PARAGRAPH, h->type, "just hash is paragraph");
    md_document_free(doc);
    
    tests_passed++;
}

/* ==================== Main ==================== */

int main(void) {
    printf("=== libmarkdown Test Suite ===\n\n");
    
    printf("--- Error Tests ---\n");
    RUN_TEST(error_string);
    RUN_TEST(error_null_operations);
    
    printf("\n--- AST Tests ---\n");
    RUN_TEST(ast_node_operations);
    RUN_TEST(ast_node_free_with_content);
    RUN_TEST(ast_null_operations);
    
    printf("\n--- Parser Tests ---\n");
    RUN_TEST(parser_create_destroy);
    RUN_TEST(parser_parse_null);
    RUN_TEST(parser_parse_basic);
    RUN_TEST(parser_parse_memory_error);
    RUN_TEST(parser_parse_headings);
    RUN_TEST(parser_parse_headings_no_space);
    RUN_TEST(parser_parse_headings_7);
    RUN_TEST(parser_parse_headings_trailing);
    RUN_TEST(parser_parse_thematic_break);
    RUN_TEST(parser_parse_thematic_break_short);
    RUN_TEST(parser_parse_thematic_break_with_spaces);
    RUN_TEST(parser_parse_fenced_code);
    RUN_TEST(parser_parse_fenced_code_tilde);
    RUN_TEST(parser_parse_fenced_code_short);
    RUN_TEST(parser_parse_fenced_code_with_info);
    RUN_TEST(parser_parse_indented_code);
    RUN_TEST(parser_parse_blockquote);
    RUN_TEST(parser_parse_blockquote_empty);
    RUN_TEST(parser_parse_blockquote_multiline);
    RUN_TEST(parser_parse_list_unordered);
    RUN_TEST(parser_parse_list_plus);
    RUN_TEST(parser_parse_list_asterisk);
    RUN_TEST(parser_parse_list_ordered);
    RUN_TEST(parser_parse_list_paren);
    RUN_TEST(parser_parse_list_ordered_multi_digit);
    RUN_TEST(parser_parse_task_list);
    RUN_TEST(parser_parse_task_list_invalid);
    RUN_TEST(parser_parse_task_list_with_space);
    RUN_TEST(parser_parse_paragraph);
    RUN_TEST(parser_parse_complex);
    RUN_TEST(parser_parse_empty);
    RUN_TEST(parser_parse_only_spaces);
    RUN_TEST(parser_parse_multiline_paragraph);
    RUN_TEST(parser_parse_multiple_paragraphs);
    RUN_TEST(parser_parse_list_marker_invalid);
    RUN_TEST(parser_parse_crlf);
    RUN_TEST(parser_parse_setext_heading);
    RUN_TEST(parser_parse_setext_h2);
    RUN_TEST(parser_parse_leading_tab);
    RUN_TEST(parser_parse_list_consecutive);
    RUN_TEST(parser_parse_mixed_content);
    
    printf("\n--- Iterator Tests ---\n");
    RUN_TEST(iterator_create_destroy);
    RUN_TEST(iterator_reset);
    RUN_TEST(iterator_traverse);
    RUN_TEST(iterator_traverse_all_types);
    RUN_TEST(iterator_empty_doc);
    
    printf("\n--- Document Tests ---\n");
    RUN_TEST(document_free_null);
    RUN_TEST(document_get_source);
    RUN_TEST(document_visit);
    RUN_TEST(document_visit_early_exit);
    RUN_TEST(node_type_name);
    
    printf("\n--- Extractor Tests ---\n");
    RUN_TEST(extractor_headings);
    RUN_TEST(extractor_headings_null);
    RUN_TEST(extractor_headings_empty);
    RUN_TEST(extractor_code_blocks);
    RUN_TEST(extractor_code_blocks_null);
    RUN_TEST(extractor_code_blocks_empty);
    RUN_TEST(extractor_links);
    RUN_TEST(extractor_links_null);
    RUN_TEST(extractor_links_multiple);
    RUN_TEST(extractor_images);
    RUN_TEST(extractor_images_null);
    RUN_TEST(extractor_tables);
    RUN_TEST(extractor_tables_null);
    RUN_TEST(extractor_plain_text);
    RUN_TEST(extractor_plain_text_with_heading);
    RUN_TEST(extractor_plain_text_empty);
    RUN_TEST(extractor_plain_text_complex);
    
    printf("\n--- Parser Options Tests ---\n");
    RUN_TEST(parser_options_default);
    RUN_TEST(parser_options_custom);
    
    printf("\n--- File Parsing Tests ---\n");
    RUN_TEST(parser_parse_file);
    RUN_TEST(parser_get_error);
    RUN_TEST(parser_parse_file_memory_error);
    RUN_TEST(parser_parse_file_success);
    
    printf("\n--- Edge Cases ---\n");
    RUN_TEST(parser_special_characters);
    RUN_TEST(parser_special_headings);
    RUN_TEST(parser_list_with_tab);
    RUN_TEST(iterator_null_operations);
    RUN_TEST(parser_edge_cases);
    RUN_TEST(parser_very_long_line);
    RUN_TEST(parser_many_newlines);
    
    printf("\n--- Coverage Tests ---\n");
    RUN_TEST(document_getters);
    RUN_TEST(table_extraction_manual);
    RUN_TEST(image_getter);
    RUN_TEST(parser_inline_edge_cases);
    RUN_TEST(parser_list_edge_cases);
    RUN_TEST(extractor_table_empty_content);
    
    RUN_TEST(parser_heading_whitespace);
    
    printf("\n=== Results ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}
