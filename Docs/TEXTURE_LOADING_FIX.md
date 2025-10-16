# 纹理加载系统修复报告

## 修复日期
2025年10月16日

## 问题概述
在Wind Engine的3D渲染系统中发现了严重的纹理加载和绑定问题，导致多层草地效果渲染异常。问题主要集中在纹理单元分配、GLSL动态索引限制和纹理绑定时机等方面。

## 发现的关键问题

### 1. 纹理单元分配混乱 ❌

**问题描述**：
- 每个mesh都从`GL_TEXTURE0`开始绑定纹理
- 后渲染的mesh会覆盖前面mesh的纹理绑定
- Fragment Shader期望3个固定的纹理单元，但C++代码绑定逻辑错误

**原始代码**：
```cpp
// DrawInstancedWind - 原始版本
for (size_t meshIndex = 0; meshIndex < m_meshes.size(); ++meshIndex) {
    for (unsigned int i = 0; i < mesh.textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);  // ❌ 每个mesh都从0开始
        // ...
        if (name == "texture_diffuse") {
            if (meshIndex < 3) {
                uniformName = "material.texture_diffuse" + std::to_string(meshIndex + 1);
            }
        }
        glUniform1i(glGetUniformLocation(program, uniformName.c_str()), i); // ❌ i始终从0开始
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
    }
}
```

**问题影响**：
- Mesh 0 → `texture_diffuse1` → 绑定到纹理单元 0
- Mesh 1 → `texture_diffuse2` → 绑定到纹理单元 0 (覆盖！)
- Mesh 2 → `texture_diffuse3` → 绑定到纹理单元 0 (覆盖！)

### 2. GLSL动态索引限制 ❌

**问题描述**：
- GLSL不支持使用运行时变量索引数组
- GPU硬件要求数组索引必须是编译时常量

**原始代码**：
```glsl
// Fragment Shader - 原始版本
float opa_param_array[3] = { 0.0, 0.0, 0.8 };
// ...
FragColor = vec4(windColor, 
    (texColor.r + texColor.g + texColor.b) * 0.33333 * 
    opa_param_array[int(layerIndex)]  // ❌ layerIndex是运行时变量！
);
```

**编译错误**：
```
Error: Dynamic indexing of arrays not supported in this GLSL version
```

### 3. 纹理绑定时机错误 ❌

**问题描述**：
- 在每个mesh绘制时重新绑定纹理，导致状态混乱
- 纹理清理逻辑不完整
- 性能问题：在渲染循环中频繁调用`glGetUniformLocation`

## 修复方案

### 1. 重构纹理单元分配逻辑 ✅

**修复后代码**：
```cpp
void Model::DrawInstancedWind( GLuint program, GLuint instanceCount ) const {
    if ( !m_hasInstanceData ) {
        Draw( program );
        return;
    }

    // 🔧 修复：首先绑定所有diffuse纹理到正确的纹理单元
    int textureUnit = 0;
    for (size_t meshIndex = 0; meshIndex < m_meshes.size() && meshIndex < 3; ++meshIndex) {
        const Mesh& mesh = m_meshes[meshIndex];
        
        // 查找该mesh的diffuse纹理
        for (unsigned int i = 0; i < mesh.textures.size(); ++i) {
            if (mesh.textures[i].type == "texture_diffuse") {
                glActiveTexture(GL_TEXTURE0 + textureUnit);  // ✅ 正确的纹理单元
                glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
                
                std::string uniformName = "material.texture_diffuse" + std::to_string(textureUnit + 1);
                glUniform1i(glGetUniformLocation(program, uniformName.c_str()), textureUnit);  // ✅ 正确的单元号
                
                textureUnit++;
                break; // 每个mesh只取第一个diffuse纹理
            }
        }
    }

    // 🔧 修复：然后绘制所有mesh（不重复绑定纹理）
    for (size_t meshIndex = 0; meshIndex < m_meshes.size(); ++meshIndex) {
        const Mesh& mesh = m_meshes[meshIndex];
        const_cast<Mesh&>(mesh).DrawInstanced(instanceCount);
    }
    
    // 🔧 修复：正确清理纹理绑定
    for (int i = 0; i < textureUnit; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}
```

**修复效果**：
- Mesh 0 → `texture_diffuse1` → 纹理单元 0 ✅
- Mesh 1 → `texture_diffuse2` → 纹理单元 1 ✅  
- Mesh 2 → `texture_diffuse3` → 纹理单元 2 ✅

### 2. 修复GLSL数组索引问题 ✅

**修复后代码**：
```glsl
void main() {
    vec4 texColor;
    float timeOffset = uTime * 0.1;
    vec2 moving_coords = vec2(TexCoords.x - timeOffset, TexCoords.y);
    
    // 🔧 修复：使用分支直接赋值，避免动态数组索引
    float opacity;
    if (layerIndex < 0.05) {
        texColor = texture(material.texture_diffuse1, moving_coords);
        opacity = 0.0;  // 第0层完全透明
    } else if (layerIndex - 1.0 < 0.05 ) {
        texColor = texture(material.texture_diffuse2, moving_coords);
        opacity = 0.0;  // 第1层完全透明
    } else {
        texColor = texture(material.texture_diffuse3, vec2( TexCoords.y - timeOffset, TexCoords.x));
        opacity = 0.8;  // 第2层有透明度
    }

    vec3 windColor = vec3( 1. ) * 0.8;

    // 优化alpha裁剪判断，减少计算量
    if ((texColor.r + texColor.g + texColor.b)*0.3333 < 0.15) {
        discard;
    } else {
        FragColor = vec4( windColor, (texColor.r + texColor.g + texColor.b) * 0.33333 * opacity );  // ✅ 使用变量
    }
}
```

### 3. 添加PC模式支持到Shader转换工具 ✅

**新增功能**：
```python
# Convert_GLSL_to_h.py - 新增PC模式
def generate_pc_output_filename(input_file):
    """为PC模式生成输出文件名"""
    base_name = os.path.basename(input_file)
    name_without_ext = os.path.splitext(base_name)[0]
    
    if name_without_ext.endswith('.vert'):
        return name_without_ext + '.core.h'
    elif name_without_ext.endswith('.frag'):
        return name_without_ext + '.core.h'
    else:
        return name_without_ext + '.core.h'

def convert_version_for_pc(content):
    """为PC模式处理GLSL版本，保持OpenGL Core版本"""
    if re.search(r'#version\s+\d+\s+core', content):
        return content  # 已经有core，不需要修改
    else:
        content = re.sub(r'#version\s+(\d+)(?!\s+core)', r'#version \1 core', content)
        return content
```

**使用方式**：
```bash
# PC模式 - 自动生成文件名
python Convert_GLSL_to_h.py wind.vert.glsl auto --pc
python Convert_GLSL_to_h.py wind.frag.glsl auto --pc

# Android模式（原有功能）
python Convert_GLSL_to_h.py wind.vert.glsl wind.vert.h --android
```

## 技术细节分析

### OpenGL纹理单元机制
```cpp
// 正确的纹理绑定流程：
glActiveTexture(GL_TEXTURE0);           // 激活纹理单元0
glBindTexture(GL_TEXTURE_2D, texture1); // 绑定纹理到当前单元
glUniform1i(location, 0);               // 告诉shader uniform变量使用单元0

glActiveTexture(GL_TEXTURE1);           // 激活纹理单元1  
glBindTexture(GL_TEXTURE_2D, texture2); // 绑定不同纹理到单元1
glUniform1i(location, 1);               // 告诉shader uniform变量使用单元1
```

### GLSL限制说明
```glsl
// ❌ 错误：动态索引
float array[3] = {1.0, 2.0, 3.0};
int index = int(varying_var);
float result = array[index];  // 编译错误

// ✅ 正确：使用分支
float result;
if (varying_var < 0.5) {
    result = 1.0;
} else if (varying_var < 1.5) {
    result = 2.0;
} else {
    result = 3.0;
}
```

## 性能优化建议

### 1. 缓存Uniform Locations
```cpp
// 建议：在初始化时缓存locations
struct ShaderUniforms {
    GLint texture_diffuse1;
    GLint texture_diffuse2; 
    GLint texture_diffuse3;
};

// 在shader链接后调用一次
void cacheUniformLocations(GLuint program, ShaderUniforms& uniforms) {
    uniforms.texture_diffuse1 = glGetUniformLocation(program, "material.texture_diffuse1");
    uniforms.texture_diffuse2 = glGetUniformLocation(program, "material.texture_diffuse2");
    uniforms.texture_diffuse3 = glGetUniformLocation(program, "material.texture_diffuse3");
}
```

### 2. 减少状态切换
```cpp
// 当前实现：一次性绑定所有纹理，然后绘制所有mesh
// 优势：减少纹理状态切换，提高渲染效率
```

## 测试验证

### 修复前症状
- [ ] 草地层次渲染异常
- [ ] 纹理显示错误或黑色
- [ ] OpenGL错误：GL_INVALID_OPERATION
- [ ] Shader编译失败

### 修复后预期
- [x] 3层草地正确显示不同纹理
- [x] 透明度效果正常工作
- [x] 无OpenGL错误
- [x] Shader编译成功

## 文件修改清单

### 修改的文件
1. `EGL_Component/ModelLoader/ModelLoader_Universal_Instancing.cpp`
   - 重构 `DrawInstancedWind()` 方法
   - 修复纹理单元分配逻辑

2. `EGL_Component/Component_Shader_Blinn_Phong/wind.frag.glsl`
   - 移除动态数组索引
   - 使用分支处理不同层的opacity

3. `EGL_Component/Component_Shader_Blinn_Phong/Convert_GLSL_to_h.py`
   - 添加PC模式支持
   - 自动文件名生成功能

### 生成的文件
1. `wind.vert.core.h` - PC版本vertex shader
2. `wind.frag.core.h` - PC版本fragment shader

## 总结

这次修复解决了Wind Engine纹理系统的核心问题：

1. **架构层面**：修正了纹理单元分配策略，确保多层纹理正确绑定
2. **兼容性层面**：解决了GLSL动态索引限制，提高了跨平台兼容性  
3. **工具链层面**：完善了shader转换工具，支持PC和Android双平台
4. **性能层面**：优化了纹理绑定流程，减少不必要的状态切换

修复后的系统应该能够正确渲染多层草地效果，为后续的风场动画和实例化渲染打下坚实基础。

---
**作者**: GitHub Copilot  
**引擎**: Wind Engine v1.0  
**平台**: Windows + OpenGL Core / Android + OpenGL ES