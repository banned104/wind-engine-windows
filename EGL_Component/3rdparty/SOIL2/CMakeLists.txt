# SOIL2 Library CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# 创建SOIL2静态库
set(SOIL2_SOURCES
    SOIL2.c
    image_DXT.c
    image_helper.c
    wfETC.c
)

set(SOIL2_HEADERS
    SOIL2.h
    image_DXT.h
    image_helper.h
    pkm_helper.h
    pvr_helper.h
    stb_image.h
    stb_image_write.h
    stbi_DDS.h
    stbi_DDS_c.h
    stbi_ext.h
    stbi_ext_c.h
    stbi_pkm.h
    stbi_pkm_c.h
    stbi_pvr.h
    stbi_pvr_c.h
    stbi_qoi.h
    stbi_qoi_c.h
    stbi_qoi_write.h
    wfETC.h
)

# 创建静态库
add_library(SOIL2 STATIC ${SOIL2_SOURCES} ${SOIL2_HEADERS})

# 设置包含目录
target_include_directories(SOIL2 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置编译定义
target_compile_definitions(SOIL2 
    PRIVATE
        SOIL2_STATIC
)

# 设置C标准
target_compile_features(SOIL2 PRIVATE c_std_99)

# 设置输出目录
set_target_properties(SOIL2 PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则（可选）
install(TARGETS SOIL2
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${SOIL2_HEADERS}
    DESTINATION include/SOIL2
)