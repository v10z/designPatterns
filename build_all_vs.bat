@echo off
setlocal enabledelayedexpansion

echo =====================================
echo Building C++ Design Patterns with Visual Studio
echo =====================================

REM Check for Visual Studio compiler
where cl >nul 2>nul
if !errorlevel! neq 0 (
    echo Error: Visual Studio C++ compiler (cl) not found.
    echo Please run this script from a Visual Studio Developer Command Prompt.
    echo.
    echo You can open it from:
    echo   Start Menu -^> Visual Studio 2022 -^> Developer Command Prompt
    echo.
    echo Or run vcvarsall.bat first:
    echo   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    exit /b 1
)

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
        echo.
        echo Building %%D...
        
        REM Extract pattern name without number prefix
        set "dirname=%%D"
        for /f "tokens=1,* delims=-" %%a in ("!dirname!") do set "pattern_name=%%b"
        
        REM Replace hyphens with underscores
        set "filename=!pattern_name:-=_!"
        
        REM Try to compile with C++20 first
        cl /std:c++20 /EHsc /O2 /Fe:"build\%%D.exe" "%%D\!filename!.cpp" /nologo >nul 2>&1
        if !errorlevel! equ 0 (
            echo [OK] %%D built successfully
            set /a success+=1
        ) else (
            REM Try C++17
            cl /std:c++17 /EHsc /O2 /Fe:"build\%%D.exe" "%%D\!filename!.cpp" /nologo >nul 2>&1
            if !errorlevel! equ 0 (
                echo [OK] %%D built successfully with C++17
                set /a success+=1
            ) else (
                REM Try C++14
                cl /std:c++14 /EHsc /O2 /Fe:"build\%%D.exe" "%%D\!filename!.cpp" /nologo >nul 2>&1
                if !errorlevel! equ 0 (
                    echo [OK] %%D built successfully with C++14
                    set /a success+=1
                ) else (
                    echo [FAILED] %%D build failed
                    set /a failed+=1
                )
            )
        )
    )
)

echo.
echo =====================================
echo Build Summary:
echo Successful: !success! / !total!
echo Failed: !failed!
echo =====================================

if !failed! gtr 0 (
    echo.
    echo Some builds failed. To see error details, run individual builds.
)

echo.
echo Built executables are in: %cd%\build\
echo.

endlocal