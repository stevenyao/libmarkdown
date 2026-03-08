# libmarkdown

[中文版](./README.zhCN.md)

A C library for parsing and analyzing Markdown documents, built on CommonMark 0.31.2 + GFM (GitHub Flavored Markdown) specification.

## Features

- **Pure C implementation** - No external dependencies, C11 standard
- **AST-based parsing** - Build abstract syntax tree from Markdown
- **Iterator API** - Traverse all document elements with position information
- **Content extraction** - Extract headings, code blocks, links, images, etc.
- **CommonMark + GFM** - Supports CommonMark 0.31.2 and GitHub Flavored Markdown
- **Comprehensive error handling** - Detailed error messages with line/column info
- **Code coverage** - Built-in coverage testing support
- **Modular design** - Separate parser, iterator, extractor modules
- **Cross-platform** - Works on Linux, macOS, Windows (DLL support)
- **170+ tests** - Comprehensive test suite with 100% pass rate

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

### Linux / Unix
```bash
# Compile the library
make

# Run tests
make test

# Run coverage tests
make coverage

# Or run test directly
./bin/test
```

### Windows (VC++ DLL)

**Prerequisites:** Install Visual Studio 2022 or later

```bash
# Method 1: Using CMake (Recommended)
# Run from Visual Studio Developer Command Prompt
build_cmake.bat

# Method 2: Using MSBuild directly
# Run from Visual Studio Developer Command Prompt
build_msbuild.bat

# Run test program (170 tests)
# Run from Visual Studio Developer Command Prompt
run_example.bat
```

### CMake (Cross-platform)
```bash
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=ON
cmake --build . --config Release
```

## Project Structure

```
libmarkdown/
├── include/markdown/          # Header files
│   ├── markdown.h            # Main header
│   ├── ast.h                 # AST node definitions
│   ├── iterator.h            # Iterator API
│   ├── parser.h              # Parser API
│   ├── document.h            # Document operations
│   ├── extractor.h           # Content extraction
│   ├── error.h               # Error handling
│   └── export.h              # Windows DLL export macros
├── src/                      # Source code
│   ├── parser.c              # Parser implementation
│   ├── ast.c                 # AST operations
│   ├── iterator.c            # Iterator implementation
│   ├── document.c            # Document API
│   ├── extractor.c           # Content extraction
│   └── error.c               # Error handling
├── tests/                    # Test cases
│   ├── test.c                # Test suite (170+ tests)
│   └── sample.md             # Test document
├── examples/                 # Example programs
│   └── example.c             # Simple usage example
├── doc/                      # Documentation
│   ├── design.md             # Design documentation
│   └── prompt.md             # Development prompts
├── cmake/                    # CMake configuration
│   └── markdown-config.cmake.in
├── bin/                      # Build output directory
│   └── Release/
│       ├── markdown.dll      # Windows DLL
│       ├── markdown.lib      # Import library
│       ├── markdown.exp      # Export file
│       ├── test_markdown.exe # Test program (170 tests)
│       └── examples/
│           └── example.exe        # Example program
├── CMakeLists.txt           # CMake configuration
├── Makefile                 # Linux Makefile
├── build_cmake.bat        # Windows CMake build script
├── build_msbuild.bat      # Windows MSBuild build script
└── run_example.bat        # Run test example
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

#### Linux / Unix
```bash
gcc -o myapp myapp.c -I include -L bin -lmarkdown -std=c11
```

#### Windows (VC++ DLL)
```c
#include <markdown/markdown.h>
```
```bash
cl /DMARKDOWN_DLL /Iinclude myapp.c markdown.lib
# Ensure markdown.dll is in the same directory as your executable
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
- Link Reference Definitions
- YAML Front Matter

**Inline Elements:**
- Emphasis (Italic, Bold, Bold-Italic)
- Code Spans
- Links / Images
- Hard / Soft Line Breaks
- Strikethrough (GFM)
- Autolinks (URL/Email)
- Inline HTML
- Footnotes
- Mark / Subscript / Superscript

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.
