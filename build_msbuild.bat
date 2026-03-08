@echo off
chcp 65001 >nul
echo Building libmarkdown with MSBuild...
echo Note: This script must be run from Visual Studio Developer Command Prompt
echo.

REM Create output directory
if not exist "bin\Release" mkdir "bin\Release"

REM Compile DLL
echo Compiling DLL...
cl /nologo /W4 /WX /D_CRT_SECURE_NO_WARNINGS /DMARKDOWN_BUILD_DLL /DMARKDOWN_DLL /Iinclude /I. /O2 /MD /LD /Fo"bin\Release\markdown.dll" src\error.c src\ast.c src\iterator.c src\document.c src\parser.c src\extractor.c
if errorlevel 1 (
    echo DLL compilation failed.
    echo Make sure you are running from Visual Studio Developer Command Prompt.
    pause
    exit /b 1
)

REM Compile test program
echo Compiling test program...
cl /nologo /W4 /WX /D_CRT_SECURE_NO_WARNINGS /DMARKDOWN_DLL /Iinclude /I. /O2 /MD /Fo"bin\Release\test_markdown.exe" tests\test.c "bin\Release\markdown.lib"
if errorlevel 1 (
    echo Test program compilation failed.
    echo Make sure you are running from Visual Studio Developer Command Prompt.
    pause
    exit /b 1
)

REM Copy DLL to test program directory
copy "bin\Release\markdown.dll" "bin\Release\test_markdown.exe" >nul

REM Compile example program
echo Compiling example program...
cl /nologo /W4 /WX /D_CRT_SECURE_NO_WARNINGS /DMARKDOWN_DLL /Iinclude /I. /O2 /MD /Fo"bin\Release\example.exe" examples\example.c "bin\Release\markdown.lib"
if errorlevel 1 (
    echo Example program compilation failed.
    pause
    exit /b 1
)

REM Run tests
echo Running tests...
cd "bin\Release"
test_markdown.exe
if errorlevel 1 (
    echo Tests failed.
    pause
    exit /b 1
)

echo.
echo Build successful!
echo DLL: bin\Release\markdown.dll
echo Test executable: bin\Release\test_markdown.exe
echo Example program: bin\Release\example.exe
echo Import library: bin\Release\markdown.lib
echo.
echo To use the library:
echo 1. Copy markdown.dll to your application directory
echo 2. Add include/markdown to your include path
echo 3. Link with markdown.lib

cd ..\..
pause
