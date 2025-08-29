@echo off
setlocal enabledelayedexpansion

echo =====================================
echo Building C++ Design Patterns
echo =====================================

REM Try to find a compiler
set COMPILER=
set COMPILER_NAME=

REM Check for Visual Studio cl
where cl >nul 2>nul
if !errorlevel! equ 0 (
    set COMPILER=cl
    set COMPILER_NAME=Visual Studio
    goto :compiler_found
)

REM Check for g++
where g++ >nul 2>nul
if !errorlevel! equ 0 (
    set COMPILER=g++
    set COMPILER_NAME=GCC
    goto :compiler_found
)

REM Check for clang++
where clang++ >nul 2>nul
if !errorlevel! equ 0 (
    set COMPILER=clang++
    set COMPILER_NAME=Clang
    goto :compiler_found
)

echo Error: No C++ compiler found (cl, g++, or clang++)
echo Please install one of:
echo   - Visual Studio: https://visualstudio.microsoft.com/
echo   - MinGW-w64: https://www.mingw-w64.org/
echo   - LLVM/Clang: https://releases.llvm.org/
exit /b 1

:compiler_found
echo Using compiler: !COMPILER_NAME!
echo.

REM Create build directory
if not exist build mkdir build

REM Counter for successful builds
set success=0
set failed=0
set total=0

REM Build each pattern
for /d %%D in (*-*) do (
    if exist "%%D\*.cpp" (
        set /a total+=1
        echo Building %%D...
        
        REM Extract pattern name without number prefix
        set "dirname=%%D"
        for /f "tokens=1,* delims=-" %%a in ("!dirname!") do set "pattern_name=%%b"
        
        REM Replace hyphens with underscores
        set "filename=!pattern_name:-=_!"
        
        if "!COMPILER!"=="cl" (
            REM Visual Studio compilation
            cl /std:c++20 /EHsc /O2 /Fe:"build\%%D.exe" "%%D\!filename!.cpp" /nologo >nul 2>&1
            if !errorlevel! neq 0 (
                cl /std:c++17 /EHsc /O2 /Fe:"build\%%D.exe" "%%D\!filename!.cpp" /nologo >nul 2>&1
                if !errorlevel! neq 0 (
                    cl /std:c++14 /EHsc /O2 /Fe:"build\%%D.exe" "%%D\!filename!.cpp" /nologo >nul 2>&1
                )
            )
        ) else (
            REM GCC/Clang compilation
            !COMPILER! -std=c++20 -Wall -O2 "%%D\!filename!.cpp" -o "build\%%D.exe" >nul 2>&1
            if !errorlevel! neq 0 (
                !COMPILER! -std=c++17 -Wall -O2 "%%D\!filename!.cpp" -o "build\%%D.exe" >nul 2>&1
                if !errorlevel! neq 0 (
                    !COMPILER! -std=c++14 -Wall -O2 "%%D\!filename!.cpp" -o "build\%%D.exe" >nul 2>&1
                    if !errorlevel! neq 0 (
                        !COMPILER! -std=c++11 -Wall -O2 "%%D\!filename!.cpp" -o "build\%%D.exe" >nul 2>&1
                    )
                )
            )
        )
        
        if !errorlevel! equ 0 (
            echo   [OK] %%D
            set /a success+=1
        ) else (
            echo   [FAILED] %%D
            set /a failed+=1
        )
    )
)

echo.
echo =====================================
echo Build Summary:
echo Successful: !success! / !total!
echo Failed: !failed!
echo =====================================

if !success! gtr 0 (
    echo.
    echo Built executables are in: %cd%\build\
)

endlocal