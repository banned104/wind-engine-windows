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

# Find Python for GLSL conversion
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Define GLSL shader files and their outputs
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/EGL_Component/Component_Shader_Blinn_Phong")
set(GLSL_CONVERTER "${SHADER_DIR}/Convert_GLSL_to_h.py")

# Define shader files to convert
set(SHADER_FILES
    "${SHADER_DIR}/wind.vert.glsl"
    "${SHADER_DIR}/wind.frag.glsl"
)

# Generate header files for each shader
set(GENERATED_HEADERS)

foreach(SHADER_FILE ${SHADER_FILES})
    # Get the full filename without path and extension
    get_filename_component(SHADER_BASENAME ${SHADER_FILE} NAME)
    # Remove the .glsl extension to get the proper name (e.g., wind.vert, wind.frag)
    string(REGEX REPLACE "\\.glsl$" "" SHADER_NAME "${SHADER_BASENAME}")
    set(HEADER_FILE "${SHADER_DIR}/${SHADER_NAME}.h")
    
    # Add to list
    list(APPEND GENERATED_HEADERS ${HEADER_FILE})
    
    # Create custom command to generate header file
    add_custom_command(
        OUTPUT ${HEADER_FILE}
        COMMAND ${Python3_EXECUTABLE} ${GLSL_CONVERTER} ${SHADER_FILE} ${HEADER_FILE}
        DEPENDS ${SHADER_FILE} ${GLSL_CONVERTER}
        COMMENT "Converting ${SHADER_FILE} to C++ header"
        VERBATIM
    )
endforeach()

# Create a custom target for shader generation
add_custom_target(generate_shaders
    DEPENDS ${GENERATED_HEADERS}
    COMMENT "Generating all shader header files"
)

# Add EGL_Component library
add_subdirectory(EGL_Component)

# Make sure shaders are generated before building EGL_Component
add_dependencies(EGL_Component generate_shaders)

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
