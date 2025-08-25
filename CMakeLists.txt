cmake_minimum_required(VERSION 3.16)
project(thread_cpp_glfw)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找OpenGL
find_package(OpenGL REQUIRED)

# 设置 GLFW 为静态库
set(GLFW_BUILD_SHARED_LIBS OFF)

# 添加EGL_Component库
add_subdirectory(EGL_Component)

# 创建可执行文件
add_executable(${PROJECT_NAME} main.cpp)

# 链接库
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    EGL_Component
    glfw
    glad
    OpenGL::GL
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/EGL_Component
    ${CMAKE_SOURCE_DIR}/EGL_Component/3rdparty/glad/include
    ${CMAKE_SOURCE_DIR}/EGL_Component/3rdparty/glfw/include
)
