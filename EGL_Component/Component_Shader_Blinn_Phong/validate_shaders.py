import os
import sys
import subprocess
import glob
import tempfile
import shutil

def convert_shader_version(shader_path):
    """
    将GLSL着色器从300 es版本转换为310 es版本，用于验证
    返回临时文件路径
    """
    with open(shader_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 替换版本声明
    if '#version 300 es' in content:
        # 将300 es替换为310 es
        content = content.replace('#version 300 es', '#version 310 es')
        
        # 创建临时文件
        temp_fd, temp_path = tempfile.mkstemp(suffix='.glsl', text=True)
        try:
            with os.fdopen(temp_fd, 'w', encoding='utf-8') as temp_file:
                temp_file.write(content)
            return temp_path
        except:
            os.close(temp_fd)
            if os.path.exists(temp_path):
                os.unlink(temp_path)
            raise
    else:
        # 如果不是300 es版本，直接返回原文件路径
        return shader_path

def validate_shader(shader_path):
    # 确定着色器类型
    if "vert" in shader_path:
        shader_type = "vert"
    elif "frag" in shader_path:
        shader_type = "frag"
    else:
        print(f"无法确定着色器类型: {shader_path}")
        return False
    
    # 设置glslangValidator路径
    glslang_validator = r"D:\\MySoftwares\\Vulkan\\Bin\\glslangValidator.exe"
    
    # 转换着色器版本用于验证
    temp_shader_path = None
    try:
        temp_shader_path = convert_shader_version(shader_path)
        
        # 构建验证命令
        cmd = [glslang_validator, "-S", shader_type, "--target-env", "opengl", temp_shader_path]
        
        result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
        if result.returncode != 0:
            print(f"着色器验证失败: {shader_path}")
            print("返回码:", result.returncode)
            if result.stdout:
                print("标准输出:")
                print(result.stdout)
            if result.stderr:
                print("错误输出:")
                print(result.stderr)
            if not result.stdout and not result.stderr:
                print("没有输出信息")
            return False
        else:
            print(f"着色器验证成功: {shader_path}")
            return True
    except FileNotFoundError:
        print(f"找不到glslangValidator: {glslang_validator}")
        return False
    except Exception as e:
        print(f"验证过程出错: {e}")
        return False
    finally:
        # 清理临时文件
        if temp_shader_path and temp_shader_path != shader_path:
            try:
                os.unlink(temp_shader_path)
            except:
                pass

def main():
    # 指定着色器目录
    shader_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 查找所有.glsl文件
    shader_files = glob.glob(os.path.join(shader_dir, "*.glsl"))
    
    if not shader_files:
        print("没有找到.glsl文件")
        return
    
    print(f"找到 {len(shader_files)} 个着色器文件")
    print("注意: 300 es版本的着色器将临时转换为310 es版本进行验证")
    
    # 验证所有着色器
    failed = False
    for shader_file in shader_files:
        print(f"\n正在验证: {shader_file}")
        if not validate_shader(shader_file):
            failed = True
        else:
            print("着色器编译成功: " + shader_file)
    
    # 如果有任何着色器验证失败，返回非零退出码
    if failed:
        print("\n有着色器验证失败")
        sys.exit(1)
    else:
        print("\n所有着色器验证成功")

if __name__ == "__main__":
    main()