# libmarkdown 设计文档

## 1. 项目概述

**项目名称**: libmarkdown  
**项目类型**: C 语言静态库  
**核心功能**: 读取并解析 Markdown 格式文件，构建 AST（抽象语法树），提供 API 访问文档结构  
**目标标准**: CommonMark 0.31.2 + GFM (GitHub Flavored Markdown)

### 设计目标

- 提供纯 C 实现的 Markdown 解析库
- 支持 CommonMark 规范和 GitHub 扩展
- 提供清晰的 AST 和遍历 API
- 简单易用的接口设计
- 80%+ 代码测试覆盖率

---

## 2. 架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────┐
│                    Application                           │
└─────────────────────┬───────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────┐
│              markdown.h (Public API)                     │
│  #include <markdown/markdown.h>                         │
└─────────────────────┬───────────────────────────────────┘
                      │
      ┌───────────────┼───────────────┐
      ▼               ▼               ▼
┌──────────┐   ┌──────────┐   ┌──────────┐
│ Parser   │   │Iterator  │   │Extractor │
└──────────┘   └──────────┘   └──────────┘
      │               │               │
      └───────────────┼───────────────┘
                      ▼
┌─────────────────────────────────────────────────────────┐
│                    AST (md_node_t)                       │
│  Document → Heading → Paragraph → ...                    │
└─────────────────────────────────────────────────────────┘
```

### 2.2 模块设计

| 模块 | 文件 | 职责 |
|------|------|------|
| **Parser** | parser.c | 解析 Markdown 文本，构建 AST |
| **Iterator** | iterator.c | 遍历文档元素 |
| **Extractor** | extractor.c | 抽取特定类型内容 |
| **AST** | ast.c, ast.h | 节点数据结构定义 |
| **Document** | document.c | 文档操作 API |
| **Error** | error.c, error.h | 错误处理 |

---

## 3. API 设计

### 3.1 快速开始

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

### 3.2 解析 API

```c
// 解析器选项
typedef struct md_parser_options {
    int parse_yaml_front_matter;
    int parse_footnotes;
    int parse_table;
    int parse_task_list;
    int parse_strikethrough;
    int parse_autolink;
    int parse_html;
    size_t max_nesting;
} md_parser_options_t;

// 使用默认选项解析
md_document_t *md_parse(const char *md, size_t len);
md_document_t *md_parse_file(const char *filepath);

// 使用自定义选项解析
md_parser_t *md_parser_create(const md_parser_options_t *opts);
int md_parser_parse(md_parser_t *parser, const char *md, size_t len, md_document_t **doc);
int md_parser_parse_file(md_parser_t *parser, const char *filepath, md_document_t **doc);
void md_parser_destroy(md_parser_t *parser);

// 获取解析错误
int md_parser_get_error(const md_parser_t *parser, md_error_info_t *err);
```

### 3.3 迭代器 API

```c
// 元素结构
typedef struct md_element {
    md_node_type_t type;
    int line;
    int column;
    char *content;
    size_t content_len;
    
    union {
        struct {
            int heading_level;
            char *language;
            int list_start;
            char list_marker;
            int is_task;
            int is_checked;
        } block;
        
        struct {
            char *url;
            char *title;
            char *alt;
            int emphasis_level;
        } inline_;
    } data;
    
    md_node_type_t parent_type;
    int depth;
} md_element_t;

// 创建迭代器
md_iterator_t *md_iterator_create(const md_document_t *doc);

// 遍历元素（返回当前元素并移动到下一个）
md_element_t *md_iterator_next(md_iterator_t *iter);

// 获取当前元素
const md_element_t *md_iterator_get(const md_iterator_t *iter);

// 重置迭代器
void md_iterator_reset(md_iterator_t *iter);

// 销毁迭代器
void md_iterator_destroy(md_iterator_t *iter);
```

### 3.4 内容抽取 API

```c
// 抽取标题
int md_extract_headings(const md_document_t *doc, md_heading_t **headings, size_t *count);
void md_headings_free(md_heading_t *headings, size_t count);

// 抽取代码块
int md_extract_code_blocks(const md_document_t *doc, md_code_block_t **blocks, size_t *count);
void md_code_blocks_free(md_code_block_t *blocks, size_t count);

// 抽取链接
int md_extract_links(const md_document_t *doc, md_link_t **links, size_t *count);
void md_links_free(md_link_t *links, size_t count);

// 抽取图片
int md_extract_images(const md_document_t *doc, md_image_t **images, size_t *count);
void md_images_free(md_image_t *images, size_t count);

// 抽取纯文本
char *md_extract_plain_text(const md_document_t *doc);
```

---

## 4. 数据结构

### 4.1 节点类型定义

```c
typedef enum md_node_type {
    // 块级元素
    MD_NODE_DOCUMENT,           // 文档根节点
    MD_NODE_HEADING,            // 标题 (# H1 - ###### H6)
    MD_NODE_FENCED_CODE,        // 围栏代码块
    MD_NODE_INDENTED_CODE,      // 缩进代码块
    MD_NODE_LIST,               // 列表容器
    MD_NODE_LIST_ITEM,          // 列表项
    MD_NODE_TASK_LIST_ITEM,     // 任务列表项 (GFM)
    MD_NODE_BLOCKQUOTE,         // 引用块
    MD_NODE_TABLE,              // 表格 (GFM)
    MD_NODE_TABLE_HEAD,         // 表格头部
    MD_NODE_TABLE_ROW,          // 表格行
    MD_NODE_TABLE_CELL,         // 表格单元格
    MD_NODE_THEMATIC_BREAK,     // 水平线
    MD_NODE_HTML_BLOCK,         // HTML 块
    MD_NODE_PARAGRAPH,          // 段落
    MD_NODE_LINK_REF_DEF,       // 链接引用定义
    MD_NODE_FRONT_MATTER,       // 前置元数据 (YAML)
    
    // 行内元素
    MD_NODE_TEXT,               // 普通文本
    MD_NODE_EMPHASIS,           // 斜体
    MD_NODE_STRONG,             // 粗体
    MD_NODE_STRONG_EMPHASIS,    // 粗斜体
    MD_NODE_STRIKETHROUGH,      // 删除线 (GFM)
    MD_NODE_CODE_SPAN,          // 行内代码
    MD_NODE_LINK,               // 链接
    MD_NODE_IMAGE,              // 图片
    MD_NODE_AUTOLINK,           // 自动链接
    MD_NODE_EMAIL_AUTOLINK,     // 邮箱自动链接
    MD_NODE_LINE_BREAK,         // 硬换行
    MD_NODE_SOFT_LINE_BREAK,    // 软换行
    MD_NODE_HTML_INLINE,        // 行内 HTML
    MD_NODE_FOOTNOTE_REF,       // 脚注引用
    MD_NODE_FOOTNOTE_DEF,       // 脚注定义
    MD_NODE_MARK,               // 标记
    MD_NODE_SUBSCRIPT,          // 下标
    MD_NODE_SUPERSCRIPT,        // 上标
    MD_NODE_SPAN,               // 通用行内容器
} md_node_type_t;
```

### 4.2 节点结构

```c
typedef struct md_node {
    md_node_type_t type;        // 节点类型
    int flags;                  // 标志位
    
    union {
        struct {
            int heading_level;  // 标题级别 (1-6)
            char *language;     // 代码语言
            char *info_string;  // 信息字符串
            char *raw_html;     // 原始 HTML
            int list_start;     // 列表起始编号
            char list_marker;   // 列表标记字符
            md_list_type_t list_type;      // 列表类型
            md_list_delim_t list_delim;    // 列表分隔符
            int is_task;        // 是否为任务列表
            int is_checked;     // 任务是否完成
            md_table_align_t table_align;  // 表格对齐方式
        } block;
        
        struct {
            char *url;          // 链接/图片 URL
            char *title;        // 链接标题
            char *alt;          // 图片 alt 文本
            int emphasis_level; // 强调级别
        } inline_;
    } data;
    
    char *content;              // 文本内容
    size_t content_len;         // 内容长度
    struct md_node *parent;     // 父节点
    struct md_node *first_child;// 第一个子节点
    struct md_node *last_child; // 最后一个子节点
    struct md_node *prev;       // 前一个兄弟节点
    struct md_node *next;       // 后一个兄弟节点
} md_node_t;

// 文档结构
typedef struct {
    md_node_t *root;            // AST 根节点
    char *source;               // 原始源代码
    size_t source_len;          // 源代码长度
    md_node_t *link_refs;       // 链接引用定义链表
} md_document_t;
```

---

## 5. Markdown 支持

### 5.1 支持的元素

**块级元素 (Block Elements)**:

| 元素 | 语法示例 | 支持 |
|------|---------|------|
| ATX 标题 | `# H1`, `## H2` | ✅ |
| Setext 标题 | `Title\n===` | ✅ |
| 围栏代码块 | ` ```c\ncode\n``` ` | ✅ |
| 缩进代码块 | `    code` | ✅ |
| 无序列表 | `- item`, `* item` | ✅ |
| 有序列表 | `1. item` | ✅ |
| 任务列表 | `- [x] done` (GFM) | ✅ |
| 引用块 | `> quote` | ✅ |
| 表格 | `\|a\|b\|` (GFM) | ✅ |
| 水平线 | `---`, `***` | ✅ |
| HTML 块 | `<div>html</div>` | ✅ |
| 链接引用定义 | `[id]: url "title"` | ✅ |
| YAML 前置元数据 | `---\ntitle: ...\n---` | ✅ |

**行内元素 (Inline Elements)**:

| 元素 | 语法示例 | 支持 |
|------|---------|------|
| 行内代码 | `` `code` `` | ✅ |
| 强调 | `*italic*`, `**bold**`, `***bold italic***` | ✅ |
| 链接 | `[text](url)` | ✅ |
| 图片 | `![alt](url)` | ✅ |
| 删除线 | `~~text~~` (GFM) | ✅ |
| 自动链接 | `<https://url>` (GFM) | ✅ |
| 邮箱自动链接 | `<email@example.com>` | ✅ |
| 硬换行 | `line  \nline` | ✅ |
| 软换行 | `line\nline` | ✅ |
| 行内 HTML | `<span>text</span>` | ✅ |
| 脚注 | `[^1]` | ✅ |
| 标记 | `==text==` | ✅ |
| 下标 | `~text~` | ✅ |
| 上标 | `^text^` | ✅ |

---

## 6. 编译和使用

### 6.1 编译库

```bash
# 编译静态库
make

# 清理
make clean
```

### 6.2 编译应用程序

```bash
gcc -o myapp myapp.c -I include -L . -lmarkdown
```

### 6.3 运行测试和覆盖率

```bash
# 运行测试
make test

# 运行覆盖率测试
make coverage

# 或直接运行测试程序
./bin/test
```

---

## 7. 项目结构

```
libmarkdown/
├── include/markdown/           # 公共头文件
│   ├── markdown.h             # 主头文件
│   ├── ast.h                  # 节点类型定义
│   ├── iterator.h             # 迭代器 API
│   ├── parser.h               # 解析器 API
│   ├── document.h             # 文档操作
│   ├── extractor.h            # 内容抽取
│   └── error.h                # 错误处理
├── src/                       # 源代码
│   ├── parser.c               # 解析器实现
│   ├── parser_debug.c         # 解析器调试工具
│   ├── ast.c                  # AST 操作
│   ├── iterator.c             # 迭代器实现
│   ├── document.c             # 文档 API
│   ├── extractor.c            # 内容抽取
│   └── error.c                # 错误处理
├── tests/                     # 测试代码
│   ├── test.c                 # 测试用例
│   └── sample.md              # 测试文档
├── doc/                       # 文档
│   ├── design.md              # 本设计文档
│   └── prompt.md              # 开发提示
├── bin/                       # 编译输出
│   ├── libmarkdown.a          # 静态库
│   └── test                   # 测试程序
├── Makefile                   # 构建文件
├── test_*.c                   # 测试示例程序
└── .gitignore
```

---

## 8. 错误处理

```c
// 获取解析错误
md_error_info_t err;
int has_error = md_parser_get_error(parser, &err);

if (has_error == 0) {
    printf("Error: %s at line %d, column %d\n", 
           md_error_string(err.error), err.line, err.column);
}
```

错误类型:

- `MD_ERROR_NONE` - 无错误
- `MD_ERROR_MEMORY` - 内存分配失败
- `MD_ERROR_FILE_OPEN` - 文件打开失败
- `MD_ERROR_FILE_READ` - 文件读取失败
- `MD_ERROR_INVALID_UTF8` - 无效 UTF-8 编码
- `MD_ERROR_SYNTAX` - 语法错误
- `MD_ERROR_MAX_NESTING` - 超出最大嵌套深度
- `MD_ERROR_INTERNAL` - 内部错误
- `MD_ERROR_UNEXPECTED_EOF` - 意外的文件结束
- `MD_ERROR_INVALID_CHAR` - 无效字符
- `MD_ERROR_INVALID_STATE` - 无效状态

---

## 9. 示例

### 9.1 提取所有标题

```c
md_heading_t *headings = NULL;
size_t count = 0;

md_extract_headings(doc, &headings, &count);

for (size_t i = 0; i < count; i++) {
    printf("H%d: %s\n", headings[i].level, headings[i].text);
}

md_headings_free(headings, count);
```

### 9.2 提取代码块

```c
md_code_block_t *codes = NULL;
size_t count = 0;

md_extract_code_blocks(doc, &codes, &count);

for (size_t i = 0; i < count; i++) {
    printf("Language: %s\n", codes[i].language);
    printf("Code:\n%s\n", codes[i].code);
}

md_code_blocks_free(codes, count);
```

### 9.3 自定义遍历

```c
md_iterator_t *iter = md_iterator_create(doc);
md_element_t *elem;

while ((elem = md_iterator_next(iter)) != NULL) {
    // 处理每个元素
    switch (elem->type) {
        case MD_NODE_HEADING:
            printf("标题 H%d: %s\n", 
                   elem->data.block.heading_level, 
                   elem->content);
            break;
        case MD_NODE_PARAGRAPH:
            printf("段落: %s\n", elem->content);
            break;
        case MD_NODE_FENCED_CODE:
            printf("代码 [%s]: %s\n", 
                   elem->data.block.language,
                   elem->content);
            break;
    }
}

md_iterator_destroy(iter);
```

---

## 10. 注意事项

1. **内存管理**: 使用 `md_document_free()` 释放文档，使用 `free()` 释放抽取的内容
2. **线程安全**: 当前实现非线程安全，需自行加锁
3. **UTF-8**: 输入应为 UTF-8 编码
4. **行内解析**: 行内元素（链接、图片、强调）解析正在完善中

---

## 11. 未来计划

- [x] 完善行内元素解析
- [x] 支持表格完整解析
- [ ] 添加 AST 修改 API
- [ ] 支持输出 HTML
- [ ] 性能优化
- [ ] 内存池优化
- [ ] 支持更多 GFM 扩展
- [ ] 添加序列化/反序列化 API
