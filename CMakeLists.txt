cmake_minimum_required(VERSION 3.10)
project(libmarkdown VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

# 库配置选项
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTS "Build tests" ON)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件列表
set(LIB_SOURCES
    src/error.c
    src/ast.c
    src/iterator.c
    src/document.c
    src/parser.c
    src/extractor.c
)

# 设置输出目录到项目根目录的bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 创建库
add_library(markdown ${LIB_SOURCES})
target_include_directories(markdown PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置库输出到项目根目录的bin
set_target_properties(markdown PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

# Windows DLL导出定义
if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(markdown PRIVATE MARKDOWN_BUILD_DLL)
    target_compile_definitions(markdown PUBLIC MARKDOWN_DLL)
endif()

# 安装配置
install(TARGETS markdown
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/markdown
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 测试程序
if(BUILD_TESTS)
    add_executable(test_markdown tests/test.c)
    target_link_libraries(test_markdown markdown)
    
    # 设置测试程序输出到项目根目录的bin
    set_target_properties(test_markdown PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )
    
    # Windows DLL测试配置
    if(WIN32 AND BUILD_SHARED_LIBS)
        target_compile_definitions(test_markdown PRIVATE MARKDOWN_DLL)
    endif()
    
    # 添加测试
    add_test(NAME libmarkdown_tests COMMAND test_markdown)
endif()

# 示例程序
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(mdtools examples/mdtools.c)
    target_link_libraries(mdtools markdown)
    
    # 设置示例程序输出到项目根目录的bin/Release/examples
    set_target_properties(mdtools PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin/RelWithDebInfo
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_SOURCE_DIR}/bin/MinSizeRel
    )
    
    # Windows DLL配置
    if(WIN32 AND BUILD_SHARED_LIBS)
        target_compile_definitions(mdtools PRIVATE MARKDOWN_DLL)
    endif()
endif()

# 导出包配置（简化版本）
if(NOT WIN32)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/markdown-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/markdown-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/markdown-config.cmake
        INSTALL_DESTINATION lib/cmake/markdown
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/markdown-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/markdown-config-version.cmake
        DESTINATION lib/cmake/markdown
    )
endif()