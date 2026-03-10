# Agent Guidelines for libmarkdown

This document provides essential information for AI agents working on the libmarkdown project, a C library for parsing and analyzing Markdown documents.

## Build Commands

### Using Make (Linux/Unix)

```bash
# Build the static library and example tool
make

# Run all tests (compiles test executable and runs it)
make test

# Run coverage tests (requires gcov)
make coverage

# Build the example CLI tool (mdtools)
make mdtools

# Clean build artifacts
make clean
```

### Using CMake (Cross-platform)

```bash
# Configure and build with default options
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build . --config Release

# Run tests via ctest (after building)
ctest

# Or run the test executable directly
./bin/Release/test_markdown.exe  # Windows
./bin/test_markdown              # Unix
```

**Windows**: Use `build_cmake.bat` from Visual Studio Developer Command Prompt for a complete build and test.

## Test Commands

### Running All Tests

```bash
# With Make
make test

# With CMake (after building)
ctest

# Direct executable (after build)
./bin/test                 # Make build
./bin/Release/test_markdown.exe  # CMake build
```

### Running a Single Test

The test suite is monolithic; there is no built-in mechanism to run individual tests. However, you can:

1. **Compile and run a custom test program** that calls specific test functions from `tests/test.c`
2. **Use `ctest` with test filtering** (if CMake's `add_test` was used per test, but currently there's only one test target)
3. **Modify the test suite** to accept command-line arguments (not currently implemented)

For rapid iteration, consider creating a separate test file that links against the library.

### Coverage Analysis

```bash
make coverage
```

This generates a code coverage report showing line-by-line execution percentages. The target requires `gcov` and `lcov` (optional).

## Linting and Formatting

No automated linting or formatting tools are currently configured for this project. However, the following conventions should be followed:

- **Compiler warnings**: Treat warnings as errors (`-Werror` in CMake for non-Windows)
- **Static analysis**: Consider running `cppcheck` or `clang-tidy` manually if desired

## Code Style Guidelines

### Naming Conventions

- **Functions**: `snake_case` with `md_` prefix for public API, e.g., `md_parse_file`
- **Types**: `snake_case_t` suffix for typedefs, e.g., `md_node_t`, `md_iterator_t`
- **Enumerations**: `UPPER_SNAKE_CASE` for enumeration constants, prefixed with module name, e.g., `MD_NODE_PARAGRAPH`, `MD_ERROR_NONE`
- **Macros**: `UPPER_SNAKE_CASE`, prefixed with `MARKDOWN_` for public macros, e.g., `MARKDOWN_API`
- **Variables**: `snake_case`
- **Struct members**: `snake_case`

### Types and Structures

- Use `typedef` to hide struct implementation details for opaque types (e.g., `md_iterator_t`)
- Public structures that are part of the API should be defined in headers with complete member definitions
- Use `stdint.h` types (`uint8_t`, `size_t`, etc.) where appropriate
- Boolean values use `int` (0 for false, non-zero for true)

### Error Handling

- Functions that can fail return an `int` where 0 indicates success and non-zero indicates error
- Error details are stored in `md_error_info_t` structures passed by reference
- Memory allocation failures should return `NULL` or an appropriate error code
- Public API functions should validate input parameters and set error information
- Error messages should be concise and actionable

### Memory Management

- Allocation/deallocation functions follow the pattern `md_*_create` / `md_*_destroy` or `md_*_alloc` / `md_*_free`
- The library does not use automatic memory management; caller must free resources
- Functions that return newly allocated strings or structures document ownership transfer
- Use `malloc`/`calloc`/`realloc`/`free` directly (no custom allocator support yet)

### Header Files

- Header guards follow pattern `MARKDOWN_MODULE_H` (e.g., `MARKDOWN_AST_H`)
- Include order: system headers, library headers, local headers
- Headers should be self-contained and include all necessary dependencies
- Public API functions are marked with `MARKDOWN_API` macro for DLL support
- Document public functions with clear comments about parameters, return values, and ownership

### Source Files

- Start with `#define _POSIX_C_SOURCE 200809L` for POSIX extensions
- Include local headers with relative path `../include/markdown/...`
- Group related functions together with descriptive comments
- Keep functions focused and relatively small (under 100 lines where possible)
- Use static functions for internal implementation details

### Documentation

- Public API functions must be documented in header files
- Use descriptive variable and function names to reduce need for comments
- Complex algorithms should include explanatory comments
- Chinese documentation exists in `README.zhCN.md` and `doc/` directory

## Project Structure

```
libmarkdown/
├── include/markdown/     # Public headers
├── src/                  # Implementation files
├── tests/                # Test suite (monolithic)
├── examples/             # Example programs
├── doc/                  # Design documentation
├── cmake/                # CMake configuration
└── bin/                  # Build output directory
```

## Additional Notes

- **C Standard**: C11 with POSIX extensions
- **Dependencies**: None (pure C library)
- **Platform Support**: Linux, macOS, Windows (DLL support)
- **License**: MIT (see LICENSE file)

## Cursor and Copilot Rules

No Cursor rules (`.cursor/rules/` or `.cursorrules`) or Copilot instructions (`.github/copilot-instructions.md`) were found in the repository.

## Common Tasks for Agents

- **Adding new Markdown element support**: Update `ast.h`, parser, iterator, extractor, and add tests
- **Fixing memory leaks**: Ensure allocation/free pairs, use valgrind/AddressSanitizer, update tests
- **Improving performance**: Profile hot paths, focus on parser, consider caching
- **Extending API**: Follow naming conventions, add error handling, update documentation and tests
- **Cross-platform fixes**: Test Windows DLL macros, handle path separators and line endings

---

*This file is intended for AI agents working on the libmarkdown project. Update it as conventions evolve.*