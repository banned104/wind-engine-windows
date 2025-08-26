cmake_minimum_required(VERSION 3.16)

# Set MSVC compiler toolset to 14.3 (VS2022 17.8 version) platform.c : fatal error C1001: internal compiler error 
if(MSVC)
    set(CMAKE_GENERATOR_TOOLSET "v143,version=14.38")
    # Disable C4034 warning for MSVC
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4034")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4034")
elseif(MINGW)
    # MinGW/GCC specific flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
endif()

project(thread_cpp_glfw)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Set GLFW as static library
set(GLFW_BUILD_SHARED_LIBS OFF)

# Add EGL_Component library
add_subdirectory(EGL_Component)

# Create executable
add_executable(${PROJECT_NAME} main.cpp)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    EGL_Component
    glfw
    glad
    OpenGL::GL
)

# Set include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/EGL_Component
    ${CMAKE_SOURCE_DIR}/EGL_Component/3rdparty/glad/include
    ${CMAKE_SOURCE_DIR}/EGL_Component/3rdparty/glfw/include
)

# Copy resource files to output folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/models"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/models"
    COMMENT "Copying models directory to output folder"
)
