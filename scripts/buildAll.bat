@echo off
setlocal

echo Locating Visual Studio...
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
  set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
  echo Error: Could not find Visual Studio installation.
  pause
  exit /b 1
)

echo Setting up Visual Studio environment...
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat"
if %ERRORLEVEL% neq 0 (
    echo.
    echo Failed to initialize Visual Studio developer environment.
    pause
    exit /b %ERRORLEVEL%
)

set "CMAKE_EXE=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not exist "%CMAKE_EXE%" (
    for /f "usebackq delims=" %%i in (`where cmake 2^>nul`) do (
        set "CMAKE_EXE=%%i"
        goto :cmake_found
    )
)
:cmake_found
if not exist "%CMAKE_EXE%" (
    echo.
    echo Error: cmake.exe was not found.
    pause
    exit /b 1
)

echo.
echo ==============================================
echo Building Backend
echo ==============================================
cd /d "%~dp0backend"

echo.
echo Using CMake: "%CMAKE_EXE%"
echo Generator: Visual Studio 17 2022, x64, toolset v143

echo.
echo Cleaning previous build directory...
if exist build rmdir /s /q build

echo.
echo Configuring CMake...
"%CMAKE_EXE%" -S . -B build -G "Visual Studio 17 2022" -A x64 -T v143
if %ERRORLEVEL% neq 0 (
    echo.
    echo Backend CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Building Project...
"%CMAKE_EXE%" --build build --config Release
if %ERRORLEVEL% neq 0 (
    echo.
    echo Backend Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Backend built successfully!
echo Output is located in: %~dp0backend\build\Release\
pause