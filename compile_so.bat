@echo off
chcp 65001 >nul

python EGL_Component\Component_Shader_Blinn_Phong\Convert_GLSL_to_h.py EGL_Component\Component_Shader_Blinn_Phong\wind.frag.glsl   EGL_Component\Component_Shader_Blinn_Phong\wind.frag.h
python EGL_Component\Component_Shader_Blinn_Phong\Convert_GLSL_to_h.py EGL_Component\Component_Shader_Blinn_Phong\wind.vert.glsl   EGL_Component\Component_Shader_Blinn_Phong\wind.vert.h

python EGL_Component\Component_Shader_Blinn_Phong\validate_shaders.py
if errorlevel 1 (
    echo 着色器验证失败，终止构建！
    pause
    exit /b 1
)

echo "Changed glsl to hpp."
REM 生成构建文件; DCMAKE_MAKE_PROGRAM 配置本地Ninja工具路径 没有Ninja就到Github下载
REM Android NDK路径 DCMAKE_TOOLCHAIN_FILE
if not exist build_android mkdir build_android
cmake -S . -B build_android -G "Ninja" ^
    -DCMAKE_MAKE_PROGRAM=D:/MySoftwares/ninja-win/ninja.exe ^
    -DCMAKE_TOOLCHAIN_FILE=D:/MySoftwares/AndroidStudio_main/ndk/23.1.7779620/build/cmake/android.toolchain.cmake ^
    -DANDROID_ABI=arm64-v8a ^
    -DANDROID_PLATFORM=android-21 ^
    -DBUILD_AS_SHARED=ON
cd build_android
ninja

REM 编译生成 .so 文件
@REM cmake --build build_android

REM 可选：显示输出目录下的 .so 文件
echo.
echo ===== Build output =====
dir /b build_android\*.so
echo ========================

REM 复制生成的 .so 文件到多个目标目录
set TARGET_DIRS="D:\AndroidCode\P05_LearnKotlin\app\src\main\jniLibs\arm64-v8a"

if not exist %TARGET_DIRS% (
    echo %TARGET_DIRS% does not exist.
) ELSE (
    echo Copying .so to %TARGET_DIRS%
    copy /Y "build_android\*.so" %TARGET_DIRS%
)

pause

