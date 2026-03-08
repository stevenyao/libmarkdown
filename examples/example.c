#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <markdown/markdown.h>

int main() {
    const char *markdown_text = 
        "# Hello World\n"
        "\n"
        "This is a **simple** example of using libmarkdown.\n"
        "\n"
        "## Features\n"
        "\n"
        "* Fast parsing\n"
        "* Iterator API\n"
        "* Content extraction\n"
        "\n"
        "```c\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n"
        "```\n";
    
    printf("Parsing markdown...\n");
    
    // Parse markdown document
    md_document_t *doc = md_parse(markdown_text, strlen(markdown_text));
    if (!doc) {
        fprintf(stderr, "Failed to parse markdown\n");
        return 1;
    }
    
    // Use iterator to traverse document
    md_iterator_t *iter = md_iterator_create(doc);
    if (!iter) {
        fprintf(stderr, "Failed to create iterator\n");
        md_document_free(doc);
        return 1;
    }
    
    printf("\nDocument structure:\n");
    printf("==================\n");
    
    md_element_t *elem;
    while ((elem = md_iterator_next(iter)) != NULL) {
        switch (elem->type) {
            case MD_NODE_HEADING:
                printf("Heading level %d: %.*s\n", 
                       elem->data.block.heading_level,
                       (int)elem->content_len, elem->content);
                break;
                
            case MD_NODE_PARAGRAPH:
                printf("Paragraph: %.*s\n", 
                       (int)elem->content_len, elem->content);
                break;
                
            case MD_NODE_LIST_ITEM:
                printf("List item: %.*s\n", 
                       (int)elem->content_len, elem->content);
                break;
                
            case MD_NODE_FENCED_CODE:
                printf("Code block (language: %s)\n", 
                       elem->data.block.language ? elem->data.block.language : "none");
                break;
                
            default:
                printf("Node type: %s\n", md_node_type_name(elem->type));
                break;
        }
    }
    
    // Extract headings
    md_heading_t *headings = NULL;
    size_t heading_count = 0;
    
    if (md_extract_headings(doc, &headings, &heading_count) == 0) {
        printf("\nExtracted headings:\n");
        printf("===================\n");
        for (size_t i = 0; i < heading_count; i++) {
            printf("Level %d: %s\n", headings[i].level, headings[i].text);
        }
        md_headings_free(headings, heading_count);
    }
    
    // Extract plain text
    char *plain_text = md_extract_plain_text(doc);
    if (plain_text) {
        printf("\nPlain text preview (first 200 chars):\n");
        printf("=====================================\n");
        printf("%.200s...\n", plain_text);
        free(plain_text);
    }
    
    // Cleanup
    md_iterator_destroy(iter);
    md_document_free(doc);
    
    printf("\nExample completed successfully!\n");
    return 0;
}