#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GLSL to C++ Header Converter
将GLSL着色器文件转换为C++头文件，包含着色器代码字符串
"""

import sys
import os
import re

def remove_bom(content):
    """移除UTF-8 BOM字符"""
    # UTF-8 BOM是 \ufeff
    if content.startswith('\ufeff'):
        content = content[1:]
    # 也处理字节形式的BOM
    if content.startswith('\xef\xbb\xbf'):
        content = content[3:]
    return content

def remove_comments(content):
    """删除GLSL代码中的所有注释，包括单行和多行注释"""
    # 删除多行注释 /* ... */
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # 删除单行注释 // ...
    content = re.sub(r'//.*', '', content)
    return content

def clean_whitespace(content):
    """清理多余的空白字符和空行"""
    # 分割成行
    lines = content.split('\n')
    # 移除每行末尾的空白字符，但保留有内容的行
    cleaned_lines = []
    for line in lines:
        stripped = line.rstrip()
        # 保留非空行和只包含空白字符的行（转换为空行）
        if stripped or (not stripped and line.strip() == ''):
            cleaned_lines.append(stripped)
    
    # 移除连续的空行，只保留单个空行
    result_lines = []
    prev_empty = False
    for line in cleaned_lines:
        if line.strip() == '':
            if not prev_empty:
                result_lines.append('')
            prev_empty = True
        else:
            result_lines.append(line)
            prev_empty = False
    
    # 移除开头和结尾的空行
    while result_lines and result_lines[0] == '':
        result_lines.pop(0)
    while result_lines and result_lines[-1] == '':
        result_lines.pop()
    
    return '\n'.join(result_lines)

def escape_string_for_cpp(content):
    """将字符串中的特殊字符转义，以便在C++代码中使用"""
    # 转义反斜杠
    content = content.replace('\\', '\\\\')
    # 转义双引号
    content = content.replace('"', '\\"')
    # 将换行符转换为\\n字符串
    content = content.replace('\n', '\\n')
    return content

def normalize_line_endings(content):
    """标准化行结束符为Unix风格(\n)"""
    # 先将\r\n转换为\n，然后将单独的\r转换为\n
    content = content.replace('\r\n', '\n')
    content = content.replace('\r', '\n')
    return content

def generate_variable_name(filename):
    """根据文件名生成变量名"""
    base_name = os.path.basename(filename)
    file_name_without_ext = os.path.splitext(base_name)[0]
    
    # 根据文件类型生成变量名
    if "vert" in file_name_without_ext:
        return "WIND_VERTEX_SHADER"
    elif "frag" in file_name_without_ext:
        return "WIND_FRAGMENT_SHADER"
    else:
        # 通用命名规则
        return file_name_without_ext.upper().replace('.', '_').replace('-', '_') + "_SHADER"

def convert_version_for_android(content):
    """将GLSL版本转换为Android兼容的版本"""
    # 将 #version xxx core 替换为 #version 310 es
    content = re.sub(r'#version\s+\d+\s+core', '#version 310 es', content)
    
    # 删除OpenGL特有的扩展指令（OpenGL ES不需要）
    content = re.sub(r'#extension\s+GL_ARB_separate_shader_objects\s*:\s*enable\s*\n?', '', content)
    content = re.sub(r'#extension\s+GL_ARB_shading_language_420pack\s*:\s*enable\s*\n?', '', content)
    
    # 在版本声明后添加精度声明（OpenGL ES必需）
    content = re.sub(r'(#version\s+310\s+es\s*\n)', r'\1\nprecision highp float;\n', content)
    
    return content

def convert_version_for_pc(content):
    """为PC模式处理GLSL版本，保持OpenGL Core版本"""
    # PC模式保持原有的OpenGL Core版本，不做特殊转换
    # 如果版本行没有core，则添加core；如果已有core，则保持不变
    if re.search(r'#version\s+\d+\s+core', content):
        # 已经有core，不需要修改
        return content
    else:
        # 没有core，添加core
        content = re.sub(r'#version\s+(\d+)(?!\s+core)', r'#version \1 core', content)
        return content

def generate_pc_output_filename(input_file):
    """为PC模式生成输出文件名"""
    base_name = os.path.basename(input_file)
    name_without_ext = os.path.splitext(base_name)[0]
    
    # 将 .vert.glsl 转换为 .vert.core.h，.frag.glsl 转换为 .frag.core.h
    if name_without_ext.endswith('.vert'):
        return name_without_ext + '.core.h'
    elif name_without_ext.endswith('.frag'):
        return name_without_ext + '.core.h'
    else:
        return name_without_ext + '.core.h'

def main():
    # 检查命令行参数
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("用法: python Convert_GLSL_to_h.py <输入GLSL文件> <输出头文件> [--android|--pc]")
        print("示例: python Convert_GLSL_to_h.py wind.vert.glsl wind.vert.h")
        print("示例: python Convert_GLSL_to_h.py wind.vert.glsl wind.vert.h --android")
        print("示例: python Convert_GLSL_to_h.py wind.vert.glsl auto --pc")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    # 检查模式参数
    is_android = False
    is_pc = False
    if len(sys.argv) == 4:
        if sys.argv[3] == '--android':
            is_android = True
        elif sys.argv[3] == '--pc':
            is_pc = True
        else:
            print(f"错误: 不支持的模式参数 '{sys.argv[3]}'，支持的参数: --android, --pc")
            sys.exit(1)
    
    # 如果是PC模式且输出文件名为auto，则自动生成文件名
    if is_pc and output_file == 'auto':
        output_file = generate_pc_output_filename(input_file)
    
    # 检查输入文件是否存在
    if not os.path.exists(input_file):
        print(f"错误: 输入文件 '{input_file}' 不存在")
        sys.exit(1)
    
    try:
        # 使用UTF-8编码读取文件内容，并指定错误处理方式
        with open(input_file, 'r', encoding='utf-8-sig', errors='replace') as f:
            content = f.read()
        
        # 移除BOM字符（如果存在）
        content = remove_bom(content)
        
        # 标准化行结束符
        content = normalize_line_endings(content)
        
        # 删除所有注释
        content = remove_comments(content)
        
        # 清理空白字符
        content = clean_whitespace(content)
        
        # 根据模式转换版本号
        if is_android:
            content = convert_version_for_android(content)
        elif is_pc:
            content = convert_version_for_pc(content)
        
        # 确保内容不为空
        if not content.strip():
            print(f"警告: 处理后的shader内容为空")
        
        # 转义内容
        escaped_content = escape_string_for_cpp(content)
        
        # 生成变量名
        var_name = generate_variable_name(input_file)
        
        # 确保输出目录存在
        output_dir = os.path.dirname(output_file)
        if output_dir and not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        # 使用UTF-8编码写入输出文件（不带BOM）
        with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
            f.write("#pragma once\n\n")
            f.write(f"// Auto-generated from {os.path.basename(input_file)}\n")
            f.write(f"// Do not edit this file manually\n\n")
            f.write(f"const char* const {var_name} = \"{escaped_content}\";\n")
        
        print(f"成功转换: {input_file} -> {output_file}")
        print(f"变量名: {var_name}")
        print(f"内容长度: {len(content)} 字符")
        if is_android:
            print("已转换为Android版本(#version 310 es)")
        elif is_pc:
            print("已转换为PC版本(OpenGL Core)")
        else:
            print("默认版本转换")
        
    except Exception as e:
        print(f"错误: 转换失败 - {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()