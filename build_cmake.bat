@echo off
chcp 65001 >nul
echo Building libmarkdown for Windows...
echo Note: This script must be run from Visual Studio Developer Command Prompt
echo.

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure CMake
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

REM Run tests
echo Running tests...
cd ..\bin\Release
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
echo Tools program: bin\Release\mdtools.exe
echo.
echo To use the library:
echo 1. Copy markdown.dll to your application directory
echo 2. Add include/markdown to your include path
echo 3. Link with markdown.lib

cd ..\..
