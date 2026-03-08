# libmarkdown

一个用纯 C 语言编写的 Markdown 文档解析库，基于 CommonMark 0.31.2 + GFM (GitHub Flavored Markdown) 规范。

## 特性

- **纯 C 实现** - 无外部依赖
- **基于 AST 解析** - 从 Markdown 构建抽象语法树
- **迭代器 API** - 轻松遍历所有文档元素
- **内容抽取** - 提取标题、代码块、链接、图片等
- **CommonMark + GFM** - 支持大多数 CommonMark 和 GitHub 扩展

## 快速开始

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

## 编译

```bash
# 编译静态库
make

# 运行测试
make test
# 或
./bin/test
```

## 项目结构

```
libmarkdown/
├── include/markdown/    # 头文件
│   └── markdown.h      # 主头文件
├── src/                # 源代码
├── tests/              # 测试用例
├── doc/                # 设计文档
├── bin/                # 编译输出
│   ├── libmarkdown.a  # 静态库
│   └── test          # 测试程序
└── Makefile
```

## 使用方法

### 提取标题

```c
md_heading_t *headings = NULL;
size_t count = 0;

md_extract_headings(doc, &headings, &count);

for (size_t i = 0; i < count; i++) {
    printf("H%d: %s\n", headings[i].level, headings[i].text);
}

md_headings_free(headings, count);
```

### 提取代码块

```c
md_code_block_t *codes = NULL;
size_t count = 0;

md_extract_code_blocks(doc, &codes, &count);

for (size_t i = 0; i < count; i++) {
    printf("语言: %s\n", codes[i].language);
    printf("代码: %s\n", codes[i].code);
}

md_code_blocks_free(codes, count);
```

### 编译你的程序

```bash
gcc -o myapp myapp.c -I include -L bin -lmarkdown
```

## API 参考

详细 API 文档请参阅 [doc/design.md](doc/design.md)（中文）。

## 支持的元素

**块级元素：**
- ATX / Setext 标题
- 围栏 / 缩进代码块
- 无序 / 有序 / 任务列表
- 引用块
- 表格 (GFM)
- 水平线
- HTML 块
- 段落

**行内元素：**
- 强调（粗体/斜体）
- 行内代码
- 链接 / 图片
- 换行
- 删除线 (GFM)

## 许可证

MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎贡献！请随时提交 Issue 或 Pull Request。
