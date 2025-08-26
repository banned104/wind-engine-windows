@echo off
REM =================== 清理并设置环境 ===================
REM 只保留 MSYS2 MinGW64、Ninja 和 CMake 的路径
set "PATH=D:\MySoftwares\Msys2\mingw64\bin;D:\MySoftwares\ninja-win;D:\MySoftwares\cmake-4.1.0-windows-x86_64\bin"
set CC=
set CXX=
set CMAKE_TOOLCHAIN_FILE=
REM =================== 清理构建目录 ===================
if exist build_win (
    rmdir /s /q build_win
)

REM =================== 生成构建文件 ===================
cmake -B build_win -S . -G "Ninja" -DCMAKE_C_COMPILER=D:/MySoftwares/Msys2/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=D:/MySoftwares/Msys2/mingw64/bin/g++.exe

REM =================== 编译生成 .exe 文件 ===================
cmake --build build_win

REM =================== 显示输出目录下的 .exe 文件 ===================
echo.
echo ===== Build output =====
dir /b build_win\bin\*.exe
echo ========================
pause