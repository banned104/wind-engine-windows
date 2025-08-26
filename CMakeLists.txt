cmake_minimum_required(VERSION 3.16)

# 设置MSVC编译器工具集为14.3（VS2022 17.8版本） platform.c : fatal error C1001: 内部编译器错误 
if(MSVC)
    set(CMAKE_GENERATOR_TOOLSET "v143,version=14.38")
endif()




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

# 禁用C4034警告
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4034")

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