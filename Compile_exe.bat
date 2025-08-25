@echo off

REM 生成 Windows 平台的构建文件（使用 Ninja 或默认的 MSVC/MinGW 工具链）
REM 如果你有 Ninja，可以加上 -G "Ninja" -DCMAKE_MAKE_PROGRAM=...，否则可以省略
@REM cmake -B build_win -S . -DBUILD_AS_SHARED=OFF -G
cmake -B build_win -S . -G "MinGW Makefiles" -DCMAKE_C_COMPILER=D:/MySoftwares/Msys2/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=D:/MySoftwares/Msys2/mingw64/bin/g++.exe

REM 编译生成 .exe 文件
cmake --build build_win

REM 可选：显示输出目录下的 .exe 文件
echo.
echo ===== Build output =====
dir /b build_win\bin\*.exe
echo ========================
pause