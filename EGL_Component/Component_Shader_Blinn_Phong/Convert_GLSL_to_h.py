# generate_header.py
import sys
import os

# 获取输入文件路径和输出文件路径
input_file = sys.argv[1]
output_file = sys.argv[2]

# 从文件名中提取变量名
base_name = os.path.basename(input_file)
file_name_without_ext = os.path.splitext(base_name)[0]

# 根据文件类型生成变量名
if "vert" in file_name_without_ext:
    var_name = "WIND_VERTEX_SHADER"
elif "frag" in file_name_without_ext:
    var_name = "WIND_FRAGMENT_SHADER"
else:
    var_name = file_name_without_ext.upper() + "_CONTENT"

# 使用UTF-8编码读取文件内容
with open(input_file, 'r', encoding='utf-8') as f:
    content = f.read()

# 使用UTF-8编码写入输出文件
with open(output_file, 'w', encoding='utf-8') as f:
    f.write("#pragma once\n")
    f.write(f"inline const char* {var_name} = \"")
    f.write(content.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n'))
    f.write("\";\n")