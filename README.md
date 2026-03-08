# libmarkdown

[中文版](./README.zhCN.md)

A C library for parsing and analyzing Markdown documents, built on CommonMark 0.31.2 + GFM (GitHub Flavored Markdown) specification.

## Features

- **Pure C implementation** - No external dependencies
- **AST-based parsing** - Build abstract syntax tree from Markdown
- **Iterator API** - Traverse all document elements easily
- **Content extraction** - Extract headings, code blocks, links, images, etc.
- **CommonMark + GFM** - Supports most CommonMark and GitHub extensions

## Quick Start

```c
#include <markdown/markdown.h>

int main() {
    md_document_t *doc = md_parse_file("readme.md");
    
    md_iterator_t *iter = md_iterator_create(doc);
    md_element_t *elem;
    
    while ((elem = md_iterator_next(iter)) != NULL) {
        printf("[%s] %s\n", md_node_type_name(elem->type), elem->content);
    }
    
    md_iterator_destroy(iter);
    md_document_free(doc);
    return 0;
}
```

## Build

```bash
# Compile the library
make

# Run tests
make test
# or
./bin/test
```

## Project Structure

```
libmarkdown/
├── include/markdown/    # Header files
│   └── markdown.h      # Main header
├── src/               # Source code
├── tests/             # Test cases
├── doc/               # Design documentation
├── bin/               # Build output
│   ├── libmarkdown.a # Static library
│   └── test          # Test program
└── Makefile
```

## Usage

### Extract Headings

```c
md_heading_t *headings = NULL;
size_t count = 0;

md_extract_headings(doc, &headings, &count);

for (size_t i = 0; i < count; i++) {
    printf("H%d: %s\n", headings[i].level, headings[i].text);
}

md_headings_free(headings, count);
```

### Extract Code Blocks

```c
md_code_block_t *codes = NULL;
size_t count = 0;

md_extract_code_blocks(doc, &codes, &count);

for (size_t i = 0; i < count; i++) {
    printf("Language: %s\n", codes[i].language);
    printf("Code: %s\n", codes[i].code);
}

md_code_blocks_free(codes, count);
```

### Compile Your Program

```bash
gcc -o myapp myapp.c -I include -L bin -lmarkdown
```

## API Reference

See [doc/design.md](doc/design.md) for detailed API documentation.

## Supported Elements

**Block Elements:**
- ATX / Setext Headings
- Fenced / Indented Code Blocks
- Unordered / Ordered / Task Lists
- Block Quotes
- Tables (GFM)
- Thematic Breaks
- HTML Blocks
- Paragraphs

**Inline Elements:**
- Emphasis (Bold/Italic)
- Code Spans
- Links / Images
- Line Breaks
- Strikethrough (GFM)

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.
