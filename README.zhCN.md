# libmarkdown

一个用纯 C 语言编写的 Markdown 文档解析库，基于 CommonMark 0.31.2 + GFM (GitHub Flavored Markdown) 规范。

## 特性

- **纯 C 实现** - 无外部依赖，C11 标准
- **基于 AST 解析** - 从 Markdown 构建抽象语法树
- **迭代器 API** - 带位置信息遍历所有文档元素
- **内容抽取** - 提取标题、代码块、链接、图片等
- **CommonMark + GFM** - 支持 CommonMark 0.31.2 和 GitHub Flavored Markdown
- **完善的错误处理** - 带行号/列号的详细错误信息
- **代码覆盖率** - 内置覆盖率测试支持
- **模块化设计** - 分离的解析器、迭代器、抽取器模块
- **跨平台支持** - 支持 Linux、macOS、Windows（DLL 支持）
- **170+ 测试** - 完整的测试套件，通过率 100%

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

### Linux / Unix
```bash
# 编译静态库
make

# 运行测试
make test

# 运行覆盖率测试
make coverage

# 或直接运行测试程序
./bin/test
```

### Windows (VC++ DLL)

**前提条件:** 安装 Visual Studio 2022 或更高版本

```bash
# 方法一：使用 CMake（推荐）
# 从 Visual Studio Developer Command Prompt 运行
build_cmake.bat

# 方法二：使用 MSBuild 直接编译
# 从 Visual Studio Developer Command Prompt 运行
build_msbuild.bat

# 运行测试程序（170 个测试）
# 从 Visual Studio Developer Command Prompt 运行
run_example.bat
```

### CMake（跨平台）
```bash
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=ON
cmake --build . --config Release
```

## 项目结构

```
libmarkdown/
├── include/markdown/          # 头文件
│   ├── markdown.h            # 主头文件
│   ├── ast.h                 # AST 节点定义
│   ├── iterator.h            # 迭代器 API
│   ├── parser.h              # 解析器 API
│   ├── document.h            # 文档操作
│   ├── extractor.h           # 内容抽取
│   ├── error.h               # 错误处理
│   └── export.h              # Windows DLL 导出宏
├── src/                      # 源代码
│   ├── parser.c              # 解析器实现
│   ├── ast.c                 # AST 操作
│   ├── iterator.c            # 迭代器实现
│   ├── document.c            # 文档 API
│   ├── extractor.c           # 内容抽取
│   └── error.c               # 错误处理
├── tests/                    # 测试用例
│   ├── test.c                # 测试套件（170+ 测试）
│   └── sample.md             # 测试文档
├── examples/                 # 示例程序
│   └── example.c             # 简单使用示例
├── doc/                      # 设计文档
│   ├── design.md             # 设计文档
│   └── prompt.md             # 开发提示
├── cmake/                    # CMake 配置
│   └── markdown-config.cmake.in
├── bin/                      # 编译输出目录
│   └── Release/
│       ├── markdown.dll      # Windows DLL
│       ├── markdown.lib      # 导入库
│       ├── markdown.exp      # 导出文件
│       ├── test_markdown.exe # 测试程序（170 个测试）
│       └── examples/
│           └── example.exe        # 示例程序
├── CMakeLists.txt           # CMake 配置文件
├── Makefile                 # Linux Makefile
├── build_cmake.bat        # Windows CMake 构建脚本
├── build_msbuild.bat      # Windows MSBuild 构建脚本
└── run_example.bat        # 运行测试示例
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
# 确保 markdown.dll 与可执行文件在同一目录
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
- 链接引用定义
- YAML 前置元数据

**行内元素：**
- 强调（斜体、粗体、粗斜体）
- 行内代码
- 链接 / 图片
- 硬换行 / 软换行
- 删除线 (GFM)
- 自动链接（URL/邮箱）
- 行内 HTML
- 脚注
- 标记 / 下标 / 上标

## 许可证

MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎贡献！请随时提交 Issue 或 Pull Request。
