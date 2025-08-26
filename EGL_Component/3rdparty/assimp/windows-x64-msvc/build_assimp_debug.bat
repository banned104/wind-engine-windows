@echo off
REM Build Assimp with MSVC - Debug version

echo Cleaning and creating build directory...
if exist build rmdir /s /q build
mkdir build
cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_NO_EXPORT=TRUE -DASSIMP_BUILD_ASSIMP_TOOLS=FALSE -DASSIMP_BUILD_SAMPLES=FALSE -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=FALSE -DASSIMP_BUILD_FBX_IMPORTER=TRUE -DASSIMP_BUILD_OBJ_IMPORTER=TRUE -DASSIMP_BUILD_GLTF_IMPORTER=TRUE

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building Debug version...
cmake --build . --config Debug --parallel 8

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo Debug build complete! Library files are in: build/lib/Debug/
pause