#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <markdown/markdown.h>

static int get_depth(const md_node_t *node) {
    int depth = 0;
    while (node && (node = md_node_get_parent(node)) != NULL) {
        depth++;
    }
    return depth;
}

static int print_node(const md_node_t *node, void *data) {
    (void)data;
    int depth = get_depth(node);
    
    if (depth > 0) {
        for (int i = 0; i < depth - 1; i++) {
            printf("  ");
        }
        printf("- ");
    }
    
    md_node_type_t type = md_node_get_type(node);
    printf("%s", md_node_type_name(type));
    
    if (type == MD_NODE_HEADING) {
        printf(" [Level %d]", md_node_get_heading_level(node));
    } else if (type == MD_NODE_FENCED_CODE) {
        const char *lang = md_node_get_code_language(node);
        if (lang) printf(" [Lang: %s]", lang);
    } else if (type == MD_NODE_LINK || type == MD_NODE_IMAGE) {
        const char *url = md_node_get_url(node);
        if (url) printf(" [URL: %s]", url);
    }
    
    const char *content = md_node_get_content(node);
    if (content) {
        printf(" : \"");
        int len = 0;
        while (content[len] && len < 40 && content[len] != '\n' && content[len] != '\r') len++;
        printf("%.*s", len, content);
        if (content[len]) printf("...");
        printf("\"");
    }
    
    printf("\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <markdown_file>\n", argv[0]);
        return 1;
    }
    
    const char *filepath = argv[1];
    printf("Parsing file: %s\n\n", filepath);
    
    md_document_t *doc = md_parse_file(filepath);
    if (!doc) {
        fprintf(stderr, "Failed to parse markdown file.\n");
        return 1;
    }
    
    printf("=== Document AST ===\n");
    md_document_visit(doc, print_node, NULL);
    
    printf("\n=== Extracted Information ===\n");
    
    md_heading_t *headings = NULL;
    size_t h_count = 0;
    md_extract_headings(doc, &headings, &h_count);
    printf("Headings: %zu\n", h_count);
    md_headings_free(headings, h_count);
    
    md_link_t *links = NULL;
    size_t l_count = 0;
    md_extract_links(doc, &links, &l_count);
    printf("Links: %zu\n", l_count);
    md_links_free(links, l_count);
    
    md_image_t *images = NULL;
    size_t i_count = 0;
    md_extract_images(doc, &images, &i_count);
    printf("Images: %zu\n", i_count);
    md_images_free(images, i_count);
    
    md_code_block_t *blocks = NULL;
    size_t c_count = 0;
    md_extract_code_blocks(doc, &blocks, &c_count);
    printf("Code Blocks: %zu\n", c_count);
    md_code_blocks_free(blocks, c_count);
    
    md_table_t *tables = NULL;
    size_t t_count = 0;
    md_extract_tables(doc, &tables, &t_count);
    printf("Tables: %zu\n", t_count);
    md_tables_free(tables, t_count);
    
    char *text = md_extract_plain_text(doc);
    if (text) {
        size_t text_len = strlen(text);
        printf("\n=== Plain Text Preview (%zu bytes) ===\n", text_len);
        int preview_len = text_len > 200 ? 200 : (int)text_len;
        printf("%.*s", preview_len, text);
        if (text_len > 200) printf("...\n");
        else printf("\n");
        free(text);
    }
    
    md_document_free(doc);
    return 0;
}
