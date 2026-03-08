#ifndef MARKDOWN_EXPORT_H
#define MARKDOWN_EXPORT_H

#if defined(_WIN32) && defined(MARKDOWN_DLL)
    #ifdef MARKDOWN_BUILD_DLL
        #define MARKDOWN_API __declspec(dllexport)
    #else
        #define MARKDOWN_API __declspec(dllimport)
    #endif
#else
    #define MARKDOWN_API
#endif

#endif