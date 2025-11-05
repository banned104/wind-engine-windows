# OpenGL API 参考文档

本文档详细记录了 EGL_Component 项目中使用的所有 OpenGL 和 GLFW API 函数。

## 目录

1. [项目模块概览](#项目模块概览)
2. [缓冲区对象管理](#缓冲区对象管理)
3. [纹理管理](#纹理管理)
4. [着色器程序](#着色器程序)
5. [帧缓冲对象-FBO](#帧缓冲对象-fbo)
6. [顶点数组对象-VAO](#顶点数组对象-vao)
7. [渲染状态管理](#渲染状态管理)
8. [统一缓冲对象-UBO](#统一缓冲对象-ubo)
9. [绘制命令](#绘制命令)
10. [GLFW窗口管理](#glfw窗口管理)
11. [EGL上下文管理](#egl上下文管理)
12. [常用OpenGL常量](#常用opengl常量)
13. [项目特定使用模式](#项目特定使用模式)
14. [性能优化建议](#性能优化建议)
15. [调试技巧](#调试技巧)

---

## 项目模块概览

本项目包含以下主要组件模块：

- **Component_3DModels**: 3D 模型渲染器
- **Component_Camera**: 相机系统
- **Component_FBO**: 帧缓冲对象封装
- **Component_TextureManager**: 纹理管理器
- **Component_UBO**: 统一缓冲对象管理
- **Component_Offscreen**: 离屏渲染
- **Component_SkyBox**: 天空盒渲染
- **Component_Box**: 包围盒渲染
- **Component_Shader_Blinn_Phong**: Blinn-Phong 光照着色器
- **Component_Silhouette**: 轮廓渲染和拾取
- **Component_Mouse**: 相机交互控制
- **ModelLoader**: 模型加载器（支持 OBJ、GLTF、GLB、FBX 等格式）
- **Shader**: 着色器加载和编译

---

## 缓冲区对象管理

### glGenBuffers

```cpp
void glGenBuffers(GLsizei n, GLuint* buffers);
```

**功能**: 生成缓冲区对象名称（ID）

**参数**:
- `n`: 要生成的缓冲区对象数量
- `buffers`: 存储生成的缓冲区对象 ID 的数组

**使用场景**: 
- 创建 VBO（顶点缓冲对象）
- 创建 EBO（元素缓冲对象）
- 创建 UBO（统一缓冲对象）

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::setupMesh()
glGenBuffers(1, &VBO);
glGenBuffers(1, &EBO);

// OffscreenRenderer.cpp - initScreenRender()
glGenBuffers(1, &mScreenVbo);

// UniformBuffer.cpp - 构造函数
glGenBuffers(1, &m_uboId);
```

---

### glBindBuffer

```cpp
void glBindBuffer(GLenum target, GLuint buffer);
```

**功能**: 将缓冲区对象绑定到指定的目标

**参数**:
- `target`: 缓冲区目标类型
  - `GL_ARRAY_BUFFER`: 顶点属性数据
  - `GL_ELEMENT_ARRAY_BUFFER`: 顶点索引数据
  - `GL_UNIFORM_BUFFER`: 统一缓冲对象数据
- `buffer`: 要绑定的缓冲区对象 ID

**使用场景**: 在操作缓冲区之前必须先绑定

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

// UniformBuffer.cpp
glBindBuffer(GL_UNIFORM_BUFFER, m_uboId);
```

---

### glBufferData

```cpp
void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
```

**功能**: 创建并初始化缓冲区对象的数据存储

**参数**:
- `target`: 缓冲区目标类型
- `size`: 数据大小（字节）
- `data`: 指向数据的指针，如果为 NULL 则只分配空间
- `usage`: 数据使用模式
  - `GL_STATIC_DRAW`: 数据不会或几乎不会改变
  - `GL_DYNAMIC_DRAW`: 数据会被改变很多次
  - `GL_STREAM_DRAW`: 数据每次绘制时都会改变

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::setupMesh()
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), 
             &vertices[0], GL_STATIC_DRAW);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
             &indices[0], GL_STATIC_DRAW);

// UniformBuffer.cpp
glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
```

---

### glBufferSubData

```cpp
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
```

**功能**: 更新缓冲区对象的部分数据

**参数**:
- `target`: 缓冲区目标类型
- `offset`: 数据起始偏移量（字节）
- `size`: 要更新的数据大小（字节）
- `data`: 指向新数据的指针

**使用场景**: 用于更新已存在的缓冲区数据，比 glBufferData 更高效

**项目中的使用**:
```cpp
// UniformBuffer.cpp - SetSubData()
glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
```

---

### glDeleteBuffers

```cpp
void glDeleteBuffers(GLsizei n, const GLuint* buffers);
```

**功能**: 删除缓冲区对象

**参数**:
- `n`: 要删除的缓冲区对象数量
- `buffers`: 包含要删除的缓冲区对象 ID 的数组

**使用场景**: 释放不再使用的缓冲区资源

**项目中的使用**:
```cpp
// OffscreenRenderer.cpp - 析构函数
glDeleteBuffers(1, &mScreenVbo);

// UniformBuffer.cpp - 析构函数
glDeleteBuffers(1, &m_uboId);
```

---

### glBindBufferBase

```cpp
void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
```

**功能**: 将缓冲区对象绑定到索引绑定点

**参数**:
- `target`: 必须是 `GL_UNIFORM_BUFFER`
- `index`: 绑定点索引
- `buffer`: 缓冲区对象 ID

**使用场景**: 用于 UBO（统一缓冲对象）的全局绑定点设置

**项目中的使用**:
```cpp
// UniformBuffer.cpp - 构造函数
glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_uboId);
```

---

## 纹理管理

### glGenTextures

```cpp
void glGenTextures(GLsizei n, GLuint* textures);
```

**功能**: 生成纹理对象名称（ID）

**参数**:
- `n`: 要生成的纹理对象数量
- `textures`: 存储生成的纹理对象 ID 的数组

**项目中的使用**:
```cpp
// LyFBO.cpp
glGenTextures(1, &tex);

// TextureManager.cpp - createGLTexture()
glGenTextures(1, &textureId);

// ModelLoader_Universal.cpp - textureFromMemory()
glGenTextures(1, &textureID);
```

---

### glBindTexture

```cpp
void glBindTexture(GLenum target, GLuint texture);
```

**功能**: 将纹理对象绑定到纹理目标

**参数**:
- `target`: 纹理目标类型
  - `GL_TEXTURE_2D`: 2D 纹理
  - `GL_TEXTURE_CUBE_MAP`: 立方体贴图纹理
- `texture`: 要绑定的纹理对象 ID

**使用场景**: 在操作纹理之前必须先绑定

**项目中的使用**:
```cpp
// LyFBO.cpp
glBindTexture(GL_TEXTURE_2D, tex);

// SkyBox.hpp - loadCubemap()
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

// ModelLoader_Universal.cpp - Draw()
glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
```

---

### glTexImage2D

```cpp
void glTexImage2D(GLenum target, GLint level, GLint internalFormat, 
                  GLsizei width, GLsizei height, GLint border, 
                  GLenum format, GLenum type, const void* data);
```

**功能**: 指定二维纹理图像

**参数**:
- `target`: 纹理目标
  - `GL_TEXTURE_2D`: 2D 纹理
  - `GL_TEXTURE_CUBE_MAP_POSITIVE_X` 等: 立方体贴图的各个面
- `level`: Mipmap 级别（0 为基础级别）
- `internalFormat`: 纹理内部格式（`GL_RGB`, `GL_RGBA` 等）
- `width`, `height`: 纹理宽度和高度
- `border`: 边框宽度（必须为 0）
- `format`: 像素数据格式
- `type`: 像素数据类型（通常为 `GL_UNSIGNED_BYTE`）
- `data`: 指向图像数据的指针

**使用场景**: 上传纹理数据到 GPU

**项目中的使用**:
```cpp
// LyFBO.cpp
glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 
             0, format, type, NULL);

// SkyBox.hpp - loadCubemap()
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 
             width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

// ModelLoader_Universal.cpp - textureFromMemory()
glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, 
             GL_UNSIGNED_BYTE, data);
```

---

### glTexParameteri

```cpp
void glTexParameteri(GLenum target, GLenum pname, GLint param);
```

**功能**: 设置纹理参数

**参数**:
- `target`: 纹理目标
- `pname`: 参数名称
  - `GL_TEXTURE_WRAP_S`: S 坐标（U）的环绕方式
  - `GL_TEXTURE_WRAP_T`: T 坐标（V）的环绕方式
  - `GL_TEXTURE_WRAP_R`: R 坐标的环绕方式（3D 纹理）
  - `GL_TEXTURE_MIN_FILTER`: 缩小过滤
  - `GL_TEXTURE_MAG_FILTER`: 放大过滤
- `param`: 参数值
  - 环绕方式: `GL_REPEAT`, `GL_CLAMP_TO_EDGE`, `GL_MIRRORED_REPEAT`
  - 过滤方式: `GL_LINEAR`, `GL_NEAREST`, `GL_LINEAR_MIPMAP_LINEAR` 等

**项目中的使用**:
```cpp
// LyFBO.cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// SkyBox.hpp
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// ModelLoader_Universal.cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
```

---

### glActiveTexture

```cpp
void glActiveTexture(GLenum texture);
```

**功能**: 激活指定的纹理单元

**参数**:
- `texture`: 纹理单元标识符（`GL_TEXTURE0` 到 `GL_TEXTURE31`）

**使用场景**: 在绑定纹理之前选择要使用的纹理单元

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Draw()
glActiveTexture(GL_TEXTURE0 + i);

// OffscreenRenderer.cpp - drawToScreen()
glActiveTexture(GL_TEXTURE0);

// SkyBox.hpp - Draw()
glActiveTexture(GL_TEXTURE0);
```

---

### glGenerateMipmap

```cpp
void glGenerateMipmap(GLenum target);
```

**功能**: 为纹理生成 Mipmap

**参数**:
- `target`: 纹理目标（`GL_TEXTURE_2D` 等）

**使用场景**: 自动生成纹理的多级渐远纹理，提高渲染性能和质量

**项目中的使用**:
```cpp
// TextureManager.cpp - createGLTexture()
if (generateMipmap) {
    glGenerateMipmap(GL_TEXTURE_2D);
}

// ModelLoader_Universal.cpp - textureFromMemory()
glGenerateMipmap(GL_TEXTURE_2D);
```

---

### glDeleteTextures

```cpp
void glDeleteTextures(GLsizei n, const GLuint* textures);
```

**功能**: 删除纹理对象

**参数**:
- `n`: 要删除的纹理对象数量
- `textures`: 包含要删除的纹理对象 ID 的数组

**项目中的使用**:
```cpp
// LyFBO.cpp - 析构函数
glDeleteTextures(1, &tex);

// TextureManager.cpp - removeTextureInternal()
glDeleteTextures(1, &it->second->textureId);
```

---

## 着色器程序

### glCreateShader

```cpp
GLuint glCreateShader(GLenum shaderType);
```

**功能**: 创建着色器对象

**参数**:
- `shaderType`: 着色器类型
  - `GL_VERTEX_SHADER`: 顶点着色器
  - `GL_FRAGMENT_SHADER`: 片段着色器
  - `GL_GEOMETRY_SHADER`: 几何着色器（如果支持）

**返回值**: 着色器对象 ID，失败返回 0

**项目中的使用**:
```cpp
// ShaderLoader.cpp - initShader()
GLint sh = glCreateShader(type);
```

---

### glShaderSource

```cpp
void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
```

**功能**: 替换着色器对象中的源代码

**参数**:
- `shader`: 着色器对象 ID
- `count`: 字符串数量
- `string`: 指向源代码字符串数组的指针
- `length`: 每个字符串的长度数组，传 0 则读到字符串结尾

**项目中的使用**:
```cpp
// ShaderLoader.cpp - initShader()
glShaderSource(sh, 1, &source, 0);
```

---

### glCompileShader

```cpp
void glCompileShader(GLuint shader);
```

**功能**: 编译着色器对象

**参数**:
- `shader`: 要编译的着色器对象 ID

**使用场景**: 在链接着色器程序之前必须先编译

**项目中的使用**:
```cpp
// ShaderLoader.cpp - initShader()
glCompileShader(sh);
```

---

### glGetShaderiv

```cpp
void glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
```

**功能**: 获取着色器对象的参数

**参数**:
- `shader`: 着色器对象 ID
- `pname`: 参数名称
  - `GL_COMPILE_STATUS`: 编译状态
  - `GL_INFO_LOG_LENGTH`: 信息日志长度
  - `GL_SHADER_TYPE`: 着色器类型
- `params`: 存储返回值的指针

**项目中的使用**:
```cpp
// ShaderLoader.cpp - initShader()
GLint status;
glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
if (status == 0) {
    // 编译失败
}
```

---

### glGetShaderInfoLog

```cpp
void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
```

**功能**: 获取着色器对象的信息日志

**参数**:
- `shader`: 着色器对象 ID
- `maxLength`: 信息日志缓冲区的最大长度
- `length`: 返回的实际长度
- `infoLog`: 存储信息日志的缓冲区

**使用场景**: 用于调试着色器编译错误

**项目中的使用**:
```cpp
// ShaderLoader.cpp - initShader()
auto *infoLog = new GLchar[2048];
GLsizei length;
glGetShaderInfoLog(sh, 2048, &length, infoLog);
```

---

### glCreateProgram

```cpp
GLuint glCreateProgram(void);
```

**功能**: 创建着色器程序对象

**返回值**: 程序对象 ID，失败返回 0

**使用场景**: 用于链接多个着色器对象

**项目中的使用**:
```cpp
// ShaderLoader.cpp - use()
program = glCreateProgram();
```

---

### glAttachShader

```cpp
void glAttachShader(GLuint program, GLuint shader);
```

**功能**: 将着色器对象附加到程序对象

**参数**:
- `program`: 程序对象 ID
- `shader`: 着色器对象 ID

**使用场景**: 在链接程序之前附加所有需要的着色器

**项目中的使用**:
```cpp
// ShaderLoader.cpp - use()
glAttachShader(program, vsh);
glAttachShader(program, fsh);
```

---

### glLinkProgram

```cpp
void glLinkProgram(GLuint program);
```

**功能**: 链接程序对象

**参数**:
- `program`: 要链接的程序对象 ID

**使用场景**: 将附加的着色器链接成可执行的着色器程序

**项目中的使用**:
```cpp
// ShaderLoader.cpp - use()
glLinkProgram(program);
```

---

### glGetProgramiv

```cpp
void glGetProgramiv(GLuint program, GLenum pname, GLint* params);
```

**功能**: 获取程序对象的参数

**参数**:
- `program`: 程序对象 ID
- `pname`: 参数名称
  - `GL_LINK_STATUS`: 链接状态
  - `GL_ACTIVE_UNIFORMS`: 活动 uniform 变量数量
  - `GL_ACTIVE_ATTRIBUTES`: 活动属性数量
- `params`: 存储返回值的指针

**项目中的使用**:
```cpp
// ShaderLoader.cpp - use()
GLint status = 0;
glGetProgramiv(program, GL_LINK_STATUS, &status);

// TextureManager.cpp - bindToShader()
GLint uniformCount = 0;
glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
```

---

### glUseProgram

```cpp
void glUseProgram(GLuint program);
```

**功能**: 安装程序对象作为当前渲染状态的一部分

**参数**:
- `program`: 要使用的程序对象 ID，传 0 则取消使用

**使用场景**: 在绘制之前激活着色器程序

**项目中的使用**:
```cpp
// ShaderLoader.cpp - use()
glUseProgram(program);

// OffscreenRenderer.cpp - drawToScreen()
mScreenShader->use();
```

---

### glDeleteShader

```cpp
void glDeleteShader(GLuint shader);
```

**功能**: 删除着色器对象

**参数**:
- `shader`: 要删除的着色器对象 ID

**使用场景**: 着色器链接到程序后可以删除

**项目中的使用**:
```cpp
// ShaderLoader.cpp - use()
glDeleteShader(vsh);
glDeleteShader(fsh);
```

---

### glDeleteProgram

```cpp
void glDeleteProgram(GLuint program);
```

**功能**: 删除程序对象

**参数**:
- `program`: 要删除的程序对象 ID

**项目中的使用**:
```cpp
// ShaderLoader.cpp - release()
glDeleteProgram(program);
```

---

### glGetUniformLocation

```cpp
GLint glGetUniformLocation(GLuint program, const GLchar* name);
```

**功能**: 获取 uniform 变量的位置

**参数**:
- `program`: 程序对象 ID
- `name`: uniform 变量名称

**返回值**: uniform 变量的位置，失败返回 -1

**使用场景**: 在设置 uniform 值之前获取其位置

**项目中的使用**:
```cpp
// TextureManager.cpp - bindToShader()
GLint uniformLocation = glGetUniformLocation(programId, uniformName.c_str());

// ModelLoader_Universal.cpp - Draw()
glUniform1i(glGetUniformLocation(program, uniformName.c_str()), i);

// OffscreenRenderer.cpp - drawToScreen()
glUniform1i(glGetUniformLocation(mScreenShader->handle(), "screenTexture"), 0);
```

---

### glUniform1i

```cpp
void glUniform1i(GLint location, GLint v0);
```

**功能**: 设置整型 uniform 变量的值

**参数**:
- `location`: uniform 变量的位置
- `v0`: 要设置的整数值

**使用场景**: 通常用于设置纹理单元索引

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Draw()
glUniform1i(glGetUniformLocation(program, uniformName.c_str()), i);

// OffscreenRenderer.cpp - drawToScreen()
glUniform1i(glGetUniformLocation(mScreenShader->handle(), "screenTexture"), 0);
```

---

### glGetUniformBlockIndex

```cpp
GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
```

**功能**: 获取 uniform 块的索引

**参数**:
- `program`: 程序对象 ID
- `uniformBlockName`: uniform 块的名称

**返回值**: uniform 块的索引，失败返回 `GL_INVALID_INDEX`

**使用场景**: 用于 UBO（统一缓冲对象）的绑定

**项目中的使用**:
```cpp
// UniformBuffer.cpp - BindShaderBolckToGlobalBindingPoint()
// 注释中的示例：
GLuint blockIndex = glGetUniformBlockIndex(handle(), "Globals");
```

---

### glUniformBlockBinding

```cpp
void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
```

**功能**: 将 uniform 块绑定到绑定点

**参数**:
- `program`: 程序对象 ID
- `uniformBlockIndex`: uniform 块的索引
- `uniformBlockBinding`: 绑定点索引

**使用场景**: 将着色器中的 uniform 块与 UBO 绑定点关联

**项目中的使用**:
```cpp
// UniformBuffer.cpp - BindShaderBolckToGlobalBindingPoint()
glUniformBlockBinding(program, shader_block_pos, m_bindingPoint);
```

---

### glGetActiveUniform

```cpp
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, 
                        GLsizei* length, GLint* size, GLenum* type, GLchar* name);
```

**功能**: 获取活动 uniform 变量的信息

**参数**:
- `program`: 程序对象 ID
- `index`: uniform 变量的索引
- `bufSize`: 名称缓冲区的大小
- `length`: 返回的名称长度
- `size`: uniform 变量的大小
- `type`: uniform 变量的类型
- `name`: 存储 uniform 变量名称的缓冲区

**使用场景**: 用于调试和枚举着色器中的所有 uniform 变量

**项目中的使用**:
```cpp
// TextureManager.cpp - bindToShader()
for (GLint i = 0; i < uniformCount; i++) {
    char name[256];
    GLsizei length;
    GLint size;
    GLenum type;
    glGetActiveUniform(programId, i, sizeof(name), &length, &size, &type, name);
}
```

---

## 帧缓冲对象-FBO

### glGenFramebuffers

```cpp
void glGenFramebuffers(GLsizei n, GLuint* framebuffers);
```

**功能**: 生成帧缓冲对象名称（ID）

**参数**:
- `n`: 要生成的帧缓冲对象数量
- `framebuffers`: 存储生成的帧缓冲对象 ID 的数组

**使用场景**: 创建离屏渲染目标

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
glGenFramebuffers(1, &fbo);
```

---

### glBindFramebuffer

```cpp
void glBindFramebuffer(GLenum target, GLuint framebuffer);
```

**功能**: 将帧缓冲对象绑定到目标

**参数**:
- `target`: 帧缓冲目标
  - `GL_FRAMEBUFFER`: 读写帧缓冲
  - `GL_READ_FRAMEBUFFER`: 只读帧缓冲
  - `GL_DRAW_FRAMEBUFFER`: 只写帧缓冲
- `framebuffer`: 要绑定的帧缓冲对象 ID，传 0 则绑定到默认帧缓冲

**使用场景**: 在渲染到 FBO 之前绑定

**项目中的使用**:
```cpp
// LyFBO.cpp - bind()
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

// LyFBO.cpp - unbind()
glBindFramebuffer(GL_FRAMEBUFFER, 0);

// OffscreenRenderer.cpp - drawToScreen()
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

---

### glFramebufferTexture2D

```cpp
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, 
                            GLuint texture, GLint level);
```

**功能**: 将纹理图像附加到帧缓冲对象

**参数**:
- `target`: 帧缓冲目标（`GL_FRAMEBUFFER` 等）
- `attachment`: 附加点
  - `GL_COLOR_ATTACHMENT0`: 颜色附加点 0
  - `GL_DEPTH_ATTACHMENT`: 深度附加点
  - `GL_STENCIL_ATTACHMENT`: 模板附加点
  - `GL_DEPTH_STENCIL_ATTACHMENT`: 深度模板附加点
- `textarget`: 纹理目标（`GL_TEXTURE_2D` 等）
- `texture`: 纹理对象 ID
- `level`: Mipmap 级别

**使用场景**: 将纹理作为 FBO 的渲染目标

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                       GL_TEXTURE_2D, tex, 0);
```

---

### glGenRenderbuffers

```cpp
void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers);
```

**功能**: 生成渲染缓冲对象名称（ID）

**参数**:
- `n`: 要生成的渲染缓冲对象数量
- `renderbuffers`: 存储生成的渲染缓冲对象 ID 的数组

**使用场景**: 创建深度/模板缓冲

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
glGenRenderbuffers(1, &rbo);
```

---

### glBindRenderbuffer

```cpp
void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
```

**功能**: 将渲染缓冲对象绑定到目标

**参数**:
- `target`: 必须是 `GL_RENDERBUFFER`
- `renderbuffer`: 要绑定的渲染缓冲对象 ID

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
glBindRenderbuffer(GL_RENDERBUFFER, rbo);

// LyFBO.cpp - 析构函数
glBindRenderbuffer(GL_RENDERBUFFER, 0);
```

---

### glRenderbufferStorage

```cpp
void glRenderbufferStorage(GLenum target, GLenum internalformat, 
                           GLsizei width, GLsizei height);
```

**功能**: 为渲染缓冲对象分配存储空间

**参数**:
- `target`: 必须是 `GL_RENDERBUFFER`
- `internalformat`: 内部格式
  - `GL_DEPTH24_STENCIL8`: 24 位深度 + 8 位模板
  - `GL_DEPTH_COMPONENT16`: 16 位深度
  - `GL_RGBA8`: 8 位 RGBA
- `width`, `height`: 渲染缓冲的宽度和高度

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
```

---

### glFramebufferRenderbuffer

```cpp
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, 
                               GLenum renderbuffertarget, GLuint renderbuffer);
```

**功能**: 将渲染缓冲对象附加到帧缓冲对象

**参数**:
- `target`: 帧缓冲目标（`GL_FRAMEBUFFER` 等）
- `attachment`: 附加点（`GL_DEPTH_STENCIL_ATTACHMENT` 等）
- `renderbuffertarget`: 必须是 `GL_RENDERBUFFER`
- `renderbuffer`: 渲染缓冲对象 ID

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                          GL_RENDERBUFFER, rbo);
```

---

### glCheckFramebufferStatus

```cpp
GLenum glCheckFramebufferStatus(GLenum target);
```

**功能**: 检查帧缓冲对象的完整性状态

**参数**:
- `target`: 帧缓冲目标（`GL_FRAMEBUFFER` 等）

**返回值**: 
- `GL_FRAMEBUFFER_COMPLETE`: 帧缓冲完整
- 其他值表示不完整的原因

**使用场景**: 验证 FBO 配置是否正确

**项目中的使用**:
```cpp
// LyFBO.cpp - 构造函数
GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
if (status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FrameBuffer initialization failed. Error: 0x%x", status);
}
```

---

### glDeleteFramebuffers

```cpp
void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
```

**功能**: 删除帧缓冲对象

**参数**:
- `n`: 要删除的帧缓冲对象数量
- `framebuffers`: 包含要删除的帧缓冲对象 ID 的数组

**项目中的使用**:
```cpp
// LyFBO.cpp - 析构函数
glDeleteFramebuffers(1, &fbo);
```

---

### glDeleteRenderbuffers

```cpp
void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
```

**功能**: 删除渲染缓冲对象

**参数**:
- `n`: 要删除的渲染缓冲对象数量
- `renderbuffers`: 包含要删除的渲染缓冲对象 ID 的数组

**项目中的使用**:
```cpp
// LyFBO.cpp - 析构函数
glDeleteRenderbuffers(1, &rbo);
```

---

## 顶点数组对象-VAO

### glGenVertexArrays

```cpp
void glGenVertexArrays(GLsizei n, GLuint* arrays);
```

**功能**: 生成顶点数组对象名称（ID）

**参数**:
- `n`: 要生成的顶点数组对象数量
- `arrays`: 存储生成的顶点数组对象 ID 的数组

**使用场景**: VAO 用于存储顶点属性配置状态

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::setupMesh()
glGenVertexArrays(1, &VAO);

// OffscreenRenderer.cpp - initScreenRender()
glGenVertexArrays(1, &mScreenVao);

// SkyBox.hpp - setupMesh()
glGenVertexArrays(1, &skyboxVAO);
```

---

### glBindVertexArray

```cpp
void glBindVertexArray(GLuint array);
```

**功能**: 绑定顶点数组对象

**参数**:
- `array`: 要绑定的顶点数组对象 ID，传 0 则解绑

**使用场景**: 在配置顶点属性或绘制之前绑定 VAO

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::setupMesh()
glBindVertexArray(VAO);

// ModelLoader_Universal.cpp - Mesh::Draw()
glBindVertexArray(VAO);
glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
glBindVertexArray(0);

// OffscreenRenderer.cpp - drawToScreen()
glBindVertexArray(mScreenVao);
glDrawArrays(GL_TRIANGLES, 0, 6);
glBindVertexArray(0);
```

---

### glDeleteVertexArrays

```cpp
void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);
```

**功能**: 删除顶点数组对象

**参数**:
- `n`: 要删除的顶点数组对象数量
- `arrays`: 包含要删除的顶点数组对象 ID 的数组

**项目中的使用**:
```cpp
// OffscreenRenderer.cpp - 析构函数
if (mScreenVao != 0) {
    glDeleteVertexArrays(1, &mScreenVao);
}
```

---

### glEnableVertexAttribArray

```cpp
void glEnableVertexAttribArray(GLuint index);
```

**功能**: 启用顶点属性数组

**参数**:
- `index`: 顶点属性的索引（对应着色器中的 location）

**使用场景**: 在设置顶点属性指针后启用该属性

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::setupMesh()
glEnableVertexAttribArray(0); // 位置
glEnableVertexAttribArray(1); // 法线
glEnableVertexAttribArray(2); // 纹理坐标
glEnableVertexAttribArray(3); // 切线
glEnableVertexAttribArray(4); // 副切线

// OffscreenRenderer.cpp - initScreenRender()
glEnableVertexAttribArray(0); // 位置
glEnableVertexAttribArray(1); // 纹理坐标
```

---

### glVertexAttribPointer

```cpp
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, 
                           GLboolean normalized, GLsizei stride, const void* pointer);
```

**功能**: 定义顶点属性数据的格式和位置

**参数**:
- `index`: 顶点属性的索引
- `size`: 每个顶点属性的组件数量（1, 2, 3, 或 4）
- `type`: 数据类型（`GL_FLOAT`, `GL_INT` 等）
- `normalized`: 是否归一化（`GL_TRUE` 或 `GL_FALSE`）
- `stride`: 连续顶点属性之间的字节偏移量
- `pointer`: 第一个顶点属性在缓冲区中的偏移量

**使用场景**: 告诉 OpenGL 如何解释顶点缓冲区中的数据

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::setupMesh()
// 位置属性
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                      (void*)offsetof(Vertex, Position));
// 法线属性
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                      (void*)offsetof(Vertex, Normal));
// 纹理坐标属性
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                      (void*)offsetof(Vertex, TexCoords));

// OffscreenRenderer.cpp - initScreenRender()
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                      (void*)(2 * sizeof(float)));
```

---

## 渲染状态管理

### glEnable

```cpp
void glEnable(GLenum cap);
```

**功能**: 启用服务器端 OpenGL 功能

**参数**:
- `cap`: 要启用的功能
  - `GL_DEPTH_TEST`: 深度测试
  - `GL_BLEND`: 混合（透明度）
  - `GL_CULL_FACE`: 面剔除
  - `GL_STENCIL_TEST`: 模板测试
  - `GL_SCISSOR_TEST`: 裁剪测试

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initGLES()
glEnable(GL_BLEND);
glEnable(GL_DEPTH_TEST);

// OffscreenRenderer.cpp - beginFrame()
glEnable(GL_DEPTH_TEST);
```

---

### glDisable

```cpp
void glDisable(GLenum cap);
```

**功能**: 禁用服务器端 OpenGL 功能

**参数**:
- `cap`: 要禁用的功能（同 glEnable）

**项目中的使用**:
```cpp
// ModelRenderer.cpp - renderAuxiliaryElements()
glDisable(GL_DEPTH_TEST);
mAxis->render(viewMatrix, m_projectionMatrix);
glEnable(GL_DEPTH_TEST);

// OffscreenRenderer.cpp - drawToScreen()
glDisable(GL_DEPTH_TEST);
```

---

### glBlendFunc

```cpp
void glBlendFunc(GLenum sfactor, GLenum dfactor);
```

**功能**: 指定混合函数

**参数**:
- `sfactor`: 源因子
  - `GL_SRC_ALPHA`: 源颜色的 alpha 值
  - `GL_ONE`: 1.0
  - `GL_ZERO`: 0.0
- `dfactor`: 目标因子
  - `GL_ONE_MINUS_SRC_ALPHA`: 1 - 源颜色的 alpha 值
  - `GL_ONE`: 1.0

**使用场景**: 用于实现透明度效果

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initGLES()
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// ModelRenderer.cpp - renderModel()
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

---

### glDepthFunc

```cpp
void glDepthFunc(GLenum func);
```

**功能**: 指定深度比较函数

**参数**:
- `func`: 比较函数
  - `GL_LESS`: 小于（默认）
  - `GL_LEQUAL`: 小于或等于
  - `GL_GREATER`: 大于
  - `GL_EQUAL`: 等于
  - `GL_ALWAYS`: 总是通过
  - `GL_NEVER`: 从不通过

**使用场景**: 控制深度测试的行为

**项目中的使用**:
```cpp
// SkyBox.hpp - Draw()
glDepthFunc(GL_LEQUAL); // 天空盒使用 LEQUAL
// ... 绘制天空盒 ...
glDepthFunc(GL_LESS);   // 恢复默认
```

---

### glDepthMask

```cpp
void glDepthMask(GLboolean flag);
```

**功能**: 启用或禁用深度缓冲区的写入

**参数**:
- `flag`: `GL_TRUE` 启用写入，`GL_FALSE` 禁用写入

**使用场景**: 在渲染透明物体时通常禁用深度写入

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initGLES()
glDepthMask(GL_TRUE);

// ModelRenderer.cpp - renderModel()
glDepthMask(GL_FALSE); // 禁用深度写入
mModel->DrawInstancedWind(mProgram->getProgramId(), INSTANCES_COUNT);
glDepthMask(GL_TRUE);  // 恢复深度写入
```

---

### glViewport

```cpp
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
```

**功能**: 设置视口

**参数**:
- `x`, `y`: 视口左下角的坐标
- `width`, `height`: 视口的宽度和高度

**使用场景**: 定义渲染输出的区域

**项目中的使用**:
```cpp
// OffscreenRenderer.cpp - beginFrame()
glViewport(0, 0, mWidth, mHeight);
```

---

### glClearColor

```cpp
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
```

**功能**: 指定清除颜色缓冲区时使用的颜色

**参数**:
- `red`, `green`, `blue`, `alpha`: RGBA 颜色值（0.0 到 1.0）

**项目中的使用**:
```cpp
// OffscreenRenderer.cpp - beginFrame()
glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
```

---

### glClear

```cpp
void glClear(GLbitfield mask);
```

**功能**: 清除缓冲区

**参数**:
- `mask`: 要清除的缓冲区位掩码
  - `GL_COLOR_BUFFER_BIT`: 颜色缓冲区
  - `GL_DEPTH_BUFFER_BIT`: 深度缓冲区
  - `GL_STENCIL_BUFFER_BIT`: 模板缓冲区
  - 可以使用 `|` 组合多个

**项目中的使用**:
```cpp
// OffscreenRenderer.cpp - beginFrame()
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

---

### glGetIntegerv

```cpp
void glGetIntegerv(GLenum pname, GLint* data);
```

**功能**: 获取整型状态变量

**参数**:
- `pname`: 状态变量名称
  - `GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS`: 最大纹理单元数
  - `GL_DEPTH_FUNC`: 当前深度函数
  - `GL_VIEWPORT`: 当前视口
- `data`: 存储返回值的指针

**项目中的使用**:
```cpp
// TextureManager.cpp - initialize()
glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits);

// ModelRenderer.cpp - renderSkybox()
GLint depthFunc;
glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
```

---

### glGetBooleanv

```cpp
void glGetBooleanv(GLenum pname, GLboolean* data);
```

**功能**: 获取布尔型状态变量

**参数**:
- `pname`: 状态变量名称
  - `GL_DEPTH_WRITEMASK`: 深度写入掩码
- `data`: 存储返回值的指针

**项目中的使用**:
```cpp
// ModelRenderer.cpp - renderSkybox()
GLboolean depthMask;
glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
```

---

### glGetError

```cpp
GLenum glGetError(void);
```

**功能**: 获取错误标志

**返回值**: 
- `GL_NO_ERROR`: 没有错误
- `GL_INVALID_ENUM`: 枚举参数无效
- `GL_INVALID_VALUE`: 数值参数无效
- `GL_INVALID_OPERATION`: 操作无效
- `GL_OUT_OF_MEMORY`: 内存不足

**使用场景**: 用于调试 OpenGL 错误

**项目中的使用**:
```cpp
// TextureManager.cpp - createGLTexture()
GLenum error = glGetError();
if (error != GL_NO_ERROR) {
    LOGE("OpenGL error creating texture: 0x%x", error);
}
```

---

## 统一缓冲对象-UBO

### 什么是 UBO

UBO（Uniform Buffer Object）是一种特殊的缓冲区对象，用于在多个着色器程序之间共享 uniform 数据。相比传统的 uniform 变量，UBO 具有以下优势：

1. **性能更好**: 一次性上传大量数据，减少 CPU-GPU 通信
2. **共享数据**: 多个着色器程序可以共享同一个 UBO
3. **更大容量**: 可以存储更多的 uniform 数据

### UBO 使用流程

```cpp
// 1. 创建 UBO
UniformBuffer ubo(sizeof(GlobalsData), UboBindingPoints::Globals);

// 2. 在着色器中定义 uniform 块
layout(std140) uniform Globals {
    mat4 proj;
    mat4 view;
    mat4 model;
    float time;
};

// 3. 绑定着色器块到全局绑定点
GLuint blockIndex = glGetUniformBlockIndex(programId, "Globals");
glUniformBlockBinding(programId, blockIndex, UboBindingPoints::Globals);

// 4. 更新 UBO 数据
GlobalsData data;
// ... 填充数据 ...
ubo.SetData(&data, sizeof(data));
```

### 项目中的 UBO 实现

项目使用了全局绑定点约定（`GlobalBindingPoints.hpp`），确保所有着色器使用相同的绑定点：

```cpp
namespace UboBindingPoints {
    constexpr GLuint Globals = 0;  // 全局数据绑定点
}
```

---

## 绘制命令

### glDrawArrays

```cpp
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
```

**功能**: 从数组数据渲染图元

**参数**:
- `mode`: 图元类型
  - `GL_TRIANGLES`: 三角形
  - `GL_TRIANGLE_STRIP`: 三角形条带
  - `GL_TRIANGLE_FAN`: 三角形扇
  - `GL_LINES`: 线段
  - `GL_LINE_STRIP`: 线条
  - `GL_POINTS`: 点
- `first`: 起始索引
- `count`: 顶点数量

**使用场景**: 用于绘制不使用索引缓冲的几何体

**项目中的使用**:
```cpp
// OffscreenRenderer.cpp - drawToScreen()
glDrawArrays(GL_TRIANGLES, 0, 6); // 绘制全屏四边形

// SkyBox.hpp - Draw()
glDrawArrays(GL_TRIANGLES, 0, 36); // 绘制天空盒立方体
```

---

### glDrawElements

```cpp
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
```

**功能**: 从数组数据渲染图元（使用索引）

**参数**:
- `mode`: 图元类型（同 glDrawArrays）
- `count`: 索引数量
- `type`: 索引数据类型
  - `GL_UNSIGNED_BYTE`: 8 位无符号整数
  - `GL_UNSIGNED_SHORT`: 16 位无符号整数
  - `GL_UNSIGNED_INT`: 32 位无符号整数
- `indices`: 索引数组的偏移量（如果使用 EBO 则为偏移量）

**使用场景**: 用于绘制使用索引缓冲的几何体，可以重用顶点

**项目中的使用**:
```cpp
// ModelLoader_Universal.cpp - Mesh::Draw()
glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), 
               GL_UNSIGNED_INT, 0);
```

---

### glGetString

```cpp
const GLubyte* glGetString(GLenum name);
```

**功能**: 获取 OpenGL 字符串信息

**参数**:
- `name`: 字符串名称
  - `GL_VERSION`: OpenGL 版本
  - `GL_VENDOR`: 供应商
  - `GL_RENDERER`: 渲染器
  - `GL_SHADING_LANGUAGE_VERSION`: 着色语言版本
  - `GL_EXTENSIONS`: 扩展列表

**返回值**: 指向字符串的指针

**使用场景**: 用于获取 OpenGL 实现信息和调试

**项目中的使用**:
```cpp
// ModelRenderer.cpp - 构造函数
if (!glGetString(GL_VERSION)) {
    LOGE("OpenGL context not available");
}
```

---

## GLFW窗口管理

### glfwMakeContextCurrent

```cpp
void glfwMakeContextCurrent(GLFWwindow* window);
```

**功能**: 使指定窗口的 OpenGL 上下文成为当前线程的当前上下文

**参数**:
- `window`: 要设置为当前的窗口，传 NULL 则取消当前上下文

**使用场景**: 在使用 OpenGL 函数之前必须先设置当前上下文

**项目中的使用**:
```cpp
// ModelRenderer.cpp - 构造函数
glfwMakeContextCurrent(mWindow);

// ModelRenderer.cpp - initOpenGL()
glfwMakeContextCurrent(mWindow);
```

---

### glfwSwapBuffers

```cpp
void glfwSwapBuffers(GLFWwindow* window);
```

**功能**: 交换前后缓冲区（双缓冲）

**参数**:
- `window`: 要交换缓冲区的窗口

**使用场景**: 在每帧渲染完成后调用，将渲染结果显示到屏幕

**项目中的使用**:
```cpp
// ModelRenderer.cpp - draw()
glfwSwapBuffers(mWindow);

// ModelRenderer.cpp - drawLoadingView()
glfwSwapBuffers(mWindow);
```

---

### glfwDestroyWindow

```cpp
void glfwDestroyWindow(GLFWwindow* window);
```

**功能**: 销毁指定的窗口及其上下文

**参数**:
- `window`: 要销毁的窗口

**使用场景**: 在程序退出或不再需要窗口时调用

**项目中的使用**:
```cpp
// ModelRenderer.cpp - destroyOpenGL()
glfwDestroyWindow(mWindow);
```

---

### glfwTerminate

```cpp
void glfwTerminate(void);
```

**功能**: 终止 GLFW 库

**使用场景**: 在程序退出时调用，释放 GLFW 资源

**项目中的使用**:
```cpp
// ModelRenderer.cpp - destroyOpenGL()
glfwTerminate();
```

---

### glfwGetCurrentContext

```cpp
GLFWwindow* glfwGetCurrentContext(void);
```

**功能**: 获取当前线程的当前 OpenGL 上下文

**返回值**: 当前上下文的窗口，如果没有则返回 NULL

**使用场景**: 用于检查是否存在有效的 OpenGL 上下文

**项目中的使用**:
```cpp
// LyFBO.cpp - 析构函数
if (glfwGetCurrentContext() != nullptr) {
    // 安全地删除 OpenGL 资源
}
```

---

## EGL上下文管理

### eglGetDisplay

```cpp
EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id);
```

**功能**: 获取 EGL 显示连接

**参数**:
- `display_id`: 本地显示 ID，通常使用 `EGL_DEFAULT_DISPLAY`

**返回值**: EGL 显示连接，失败返回 `EGL_NO_DISPLAY`

**使用场景**: EGL 初始化的第一步（Android 平台）

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initEGL()
mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
```

---

### eglInitialize

```cpp
EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
```

**功能**: 初始化 EGL 显示连接

**参数**:
- `dpy`: EGL 显示连接
- `major`, `minor`: 返回 EGL 版本号（可以为 NULL）

**返回值**: `EGL_TRUE` 成功，`EGL_FALSE` 失败

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initEGL()
if (eglInitialize(mDisplay, nullptr, nullptr) != EGL_TRUE) {
    LOGE("eglInitialize failed");
}
```

---

### eglChooseConfig

```cpp
EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, 
                           EGLConfig* configs, EGLint config_size, EGLint* num_config);
```

**功能**: 选择匹配的 EGL 帧缓冲配置

**参数**:
- `dpy`: EGL 显示连接
- `attrib_list`: 配置属性列表
- `configs`: 返回的配置数组
- `config_size`: 配置数组的大小
- `num_config`: 返回的配置数量

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initEGL()
EGLConfig config;
EGLint numConfigs;
EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_NONE
};
eglChooseConfig(mDisplay, configAttribs, &config, 1, &numConfigs);
```

---

### eglCreateWindowSurface

```cpp
EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, 
                                  EGLNativeWindowType win, const EGLint* attrib_list);
```

**功能**: 创建 EGL 窗口表面

**参数**:
- `dpy`: EGL 显示连接
- `config`: EGL 帧缓冲配置
- `win`: 本地窗口句柄
- `attrib_list`: 表面属性列表（可以为 NULL）

**返回值**: EGL 表面，失败返回 `EGL_NO_SURFACE`

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initEGL()
mSurface = eglCreateWindowSurface(mDisplay, config, mWindow, nullptr);
```

---

### eglCreateContext

```cpp
EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, 
                            EGLContext share_context, const EGLint* attrib_list);
```

**功能**: 创建 EGL 渲染上下文

**参数**:
- `dpy`: EGL 显示连接
- `config`: EGL 帧缓冲配置
- `share_context`: 共享上下文（通常为 `EGL_NO_CONTEXT`）
- `attrib_list`: 上下文属性列表

**返回值**: EGL 上下文，失败返回 `EGL_NO_CONTEXT`

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initEGL()
EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, contextAttribs);
```

---

### eglMakeCurrent

```cpp
EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, 
                          EGLSurface read, EGLContext ctx);
```

**功能**: 将 EGL 上下文设置为当前

**参数**:
- `dpy`: EGL 显示连接
- `draw`: 绘制表面
- `read`: 读取表面（通常与 draw 相同）
- `ctx`: EGL 上下文

**返回值**: `EGL_TRUE` 成功，`EGL_FALSE` 失败

**项目中的使用**:
```cpp
// ModelRenderer.cpp - initEGL()
if (eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) != EGL_TRUE) {
    LOGE("eglMakeCurrent failed");
}

// ModelRenderer.cpp - destroyEGL()
eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
```

---

### eglSwapBuffers

```cpp
EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
```

**功能**: 交换 EGL 表面的前后缓冲区

**参数**:
- `dpy`: EGL 显示连接
- `surface`: EGL 表面

**返回值**: `EGL_TRUE` 成功，`EGL_FALSE` 失败

**使用场景**: 在每帧渲染完成后调用（Android 平台）

**项目中的使用**:
```cpp
// ModelRenderer.cpp - draw()
eglSwapBuffers(mDisplay, mSurface);
```

---

### eglDestroyContext

```cpp
EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
```

**功能**: 销毁 EGL 渲染上下文

**参数**:
- `dpy`: EGL 显示连接
- `ctx`: 要销毁的 EGL 上下文

**项目中的使用**:
```cpp
// ModelRenderer.cpp - destroyEGL()
eglDestroyContext(mDisplay, mContext);
```

---

### eglDestroySurface

```cpp
EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
```

**功能**: 销毁 EGL 表面

**参数**:
- `dpy`: EGL 显示连接
- `surface`: 要销毁的 EGL 表面

**项目中的使用**:
```cpp
// ModelRenderer.cpp - destroyEGL()
eglDestroySurface(mDisplay, mSurface);
```

---

### eglTerminate

```cpp
EGLBoolean eglTerminate(EGLDisplay dpy);
```

**功能**: 终止 EGL 显示连接

**参数**:
- `dpy`: EGL 显示连接

**项目中的使用**:
```cpp
// ModelRenderer.cpp - destroyEGL()
eglTerminate(mDisplay);
```

---

## 常用OpenGL常量

### 缓冲区类型
- `GL_ARRAY_BUFFER`: 顶点属性缓冲区
- `GL_ELEMENT_ARRAY_BUFFER`: 索引缓冲区
- `GL_UNIFORM_BUFFER`: 统一缓冲区

### 缓冲区使用模式
- `GL_STATIC_DRAW`: 数据不会改变
- `GL_DYNAMIC_DRAW`: 数据会频繁改变
- `GL_STREAM_DRAW`: 数据每次绘制都会改变

### 数据类型
- `GL_FLOAT`: 32 位浮点数
- `GL_INT`: 32 位整数
- `GL_UNSIGNED_INT`: 32 位无符号整数
- `GL_UNSIGNED_BYTE`: 8 位无符号整数
- `GL_UNSIGNED_SHORT`: 16 位无符号整数

### 纹理格式
- `GL_RGB`: RGB 颜色
- `GL_RGBA`: RGBA 颜色（带 alpha 通道）
- `GL_RED`: 单通道红色
- `GL_RG`: 双通道红绿

### 纹理参数
- `GL_TEXTURE_WRAP_S`: S 坐标环绕方式
- `GL_TEXTURE_WRAP_T`: T 坐标环绕方式
- `GL_TEXTURE_WRAP_R`: R 坐标环绕方式
- `GL_TEXTURE_MIN_FILTER`: 缩小过滤
- `GL_TEXTURE_MAG_FILTER`: 放大过滤

### 纹理环绕方式
- `GL_REPEAT`: 重复
- `GL_CLAMP_TO_EDGE`: 边缘拉伸
- `GL_MIRRORED_REPEAT`: 镜像重复

### 纹理过滤方式
- `GL_LINEAR`: 线性过滤
- `GL_NEAREST`: 最近邻过滤
- `GL_LINEAR_MIPMAP_LINEAR`: 三线性过滤
- `GL_LINEAR_MIPMAP_NEAREST`: 双线性过滤

### 帧缓冲附加点
- `GL_COLOR_ATTACHMENT0`: 颜色附加点 0
- `GL_DEPTH_ATTACHMENT`: 深度附加点
- `GL_STENCIL_ATTACHMENT`: 模板附加点
- `GL_DEPTH_STENCIL_ATTACHMENT`: 深度模板附加点

### 渲染状态
- `GL_DEPTH_TEST`: 深度测试
- `GL_BLEND`: 混合
- `GL_CULL_FACE`: 面剔除
- `GL_STENCIL_TEST`: 模板测试
- `GL_SCISSOR_TEST`: 裁剪测试

### 混合因子
- `GL_SRC_ALPHA`: 源 alpha
- `GL_ONE_MINUS_SRC_ALPHA`: 1 - 源 alpha
- `GL_ONE`: 1.0
- `GL_ZERO`: 0.0
- `GL_DST_ALPHA`: 目标 alpha
- `GL_ONE_MINUS_DST_ALPHA`: 1 - 目标 alpha

### 深度函数
- `GL_LESS`: 小于
- `GL_LEQUAL`: 小于或等于
- `GL_GREATER`: 大于
- `GL_EQUAL`: 等于
- `GL_ALWAYS`: 总是通过
- `GL_NEVER`: 从不通过

### 图元类型
- `GL_TRIANGLES`: 三角形
- `GL_TRIANGLE_STRIP`: 三角形条带
- `GL_TRIANGLE_FAN`: 三角形扇
- `GL_LINES`: 线段
- `GL_LINE_STRIP`: 线条
- `GL_LINE_LOOP`: 闭合线条
- `GL_POINTS`: 点

### 着色器类型
- `GL_VERTEX_SHADER`: 顶点着色器
- `GL_FRAGMENT_SHADER`: 片段着色器
- `GL_GEOMETRY_SHADER`: 几何着色器

### 纹理目标
- `GL_TEXTURE_2D`: 2D 纹理
- `GL_TEXTURE_CUBE_MAP`: 立方体贴图
- `GL_TEXTURE_CUBE_MAP_POSITIVE_X`: 立方体贴图 +X 面
- `GL_TEXTURE_CUBE_MAP_NEGATIVE_X`: 立方体贴图 -X 面
- `GL_TEXTURE_CUBE_MAP_POSITIVE_Y`: 立方体贴图 +Y 面
- `GL_TEXTURE_CUBE_MAP_NEGATIVE_Y`: 立方体贴图 -Y 面
- `GL_TEXTURE_CUBE_MAP_POSITIVE_Z`: 立方体贴图 +Z 面
- `GL_TEXTURE_CUBE_MAP_NEGATIVE_Z`: 立方体贴图 -Z 面

---

## 项目特定使用模式

### 1. 模型加载流程

```cpp
// 步骤 1: 使用 Assimp 加载模型数据到 RAM
Model model("path/to/model.obj");

// 步骤 2: 上传到 GPU（可选的多线程加载）
#ifdef __MULTITHREAD_LOAD__
model.uploadToGPU();
#endif

// 步骤 3: 绘制模型
model.Draw(programId);
```

**支持的模型格式**:
- OBJ
- GLTF / GLB
- FBX
- 其他 Assimp 支持的格式

---

### 2. 纹理管理流程

```cpp
// 步骤 1: 初始化纹理管理器（单例模式）
GlobalTextureManager& texMgr = GlobalTextureManager::getInstance();
texMgr.initialize();

// 步骤 2: 加载纹理
bool success = texMgr.loadTexture("path/to/texture.jpg", "myTexture", true);

// 步骤 3: 绑定到着色器
texMgr.bindToShader("myTexture", programId, "textureSampler", 0);

// 步骤 4: 激活所有纹理（在渲染循环中）
texMgr.activateTextures();

// 步骤 5: 清理（可选）
texMgr.forceRemoveTexture("myTexture");
```

**纹理管理器特性**:
- 自动引用计数
- 纹理单元自动分配
- 支持多个着色器共享纹理
- 线程安全的单例实现

---

### 3. FBO 离屏渲染流程

```cpp
// 步骤 1: 创建 FBO
OffscreenRenderer offscreen(width, height);

// 步骤 2: 开始离屏渲染
offscreen.beginFrame();

// 步骤 3: 渲染场景
// ... 你的渲染代码 ...
model.Draw(programId);

// 步骤 4: 结束离屏渲染（MSAA 解析）
offscreen.endFrame();

// 步骤 5: 将结果绘制到屏幕
offscreen.drawToScreen();
```

**FBO 特性**:
- 支持 MSAA（多重采样抗锯齿）
- 自动深度/模板缓冲
- 后处理支持

---

### 4. UBO 使用流程

```cpp
// 步骤 1: 定义数据结构（注意内存对齐）
struct GlobalsData {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
    float time;
    // ... 其他数据 ...
};

// 步骤 2: 创建 UBO
UniformBuffer ubo(sizeof(GlobalsData), UboBindingPoints::Globals);

// 步骤 3: 在着色器中定义 uniform 块
// layout(std140) uniform Globals {
//     mat4 proj;
//     mat4 view;
//     mat4 model;
//     float time;
// };

// 步骤 4: 绑定着色器块到全局绑定点
GLuint blockIndex = glGetUniformBlockIndex(programId, "Globals");
glUniformBlockBinding(programId, blockIndex, UboBindingPoints::Globals);

// 步骤 5: 更新 UBO 数据
GlobalsData data;
// ... 填充数据 ...
ubo.SetData(&data, sizeof(data));

// 或者更新部分数据
ubo.SetSubData(&data.time, offsetof(GlobalsData, time), sizeof(float));
```

**UBO 注意事项**:
- 使用 `std140` 布局保证内存对齐
- 矩阵按列主序存储
- vec3 会被填充到 vec4 的大小

---

### 5. 着色器编译流程

```cpp
// 步骤 1: 创建着色器对象
Shader shader(vertexShaderSource, fragmentShaderSource);

// 步骤 2: 编译和链接（在 use() 中完成）
GLuint programId = shader.use();

// 步骤 3: 设置 uniform 变量
GLint location = glGetUniformLocation(programId, "uniformName");
glUniform1f(location, value);

// 步骤 4: 清理
shader.release();
```

**着色器加载特性**:
- 延迟编译（在 use() 时编译）
- 自动错误检查和日志输出
- 支持从文件或字符串加载

---

### 6. 相机系统使用

```cpp
// 步骤 1: 创建相机
Camera camera;

// 步骤 2: 设置相机参数
camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
camera.setDistance(5.0f);

// 步骤 3: 更新相机（每帧）
camera.update(deltaTime);

// 步骤 4: 获取矩阵
glm::mat4 viewMatrix = camera.getViewMatrix();
glm::mat4 projMatrix = camera.getProjectionMatrix();

// 步骤 5: 相机交互
CameraInteractor interactor(&camera);
interactor.onMouseMove(deltaX, deltaY);
interactor.onMouseScroll(scrollDelta);
```

---

## 性能优化建议

### 1. 减少状态切换

**问题**: 频繁的状态切换会导致性能下降

**解决方案**:
```cpp
// 不好的做法
for (auto& object : objects) {
    glBindTexture(GL_TEXTURE_2D, object.texture);
    glUseProgram(object.shader);
    object.draw();
}

// 好的做法：按材质排序
std::sort(objects.begin(), objects.end(), [](const Object& a, const Object& b) {
    if (a.shader != b.shader) return a.shader < b.shader;
    return a.texture < b.texture;
});

GLuint currentShader = 0;
GLuint currentTexture = 0;
for (auto& object : objects) {
    if (object.shader != currentShader) {
        glUseProgram(object.shader);
        currentShader = object.shader;
    }
    if (object.texture != currentTexture) {
        glBindTexture(GL_TEXTURE_2D, object.texture);
        currentTexture = object.texture;
    }
    object.draw();
}
```

---

### 2. 使用实例化渲染

**问题**: 绘制大量相同几何体时性能低下

**解决方案**:
```cpp
// 不好的做法
for (int i = 0; i < 1000; i++) {
    glm::mat4 model = calculateModelMatrix(i);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// 好的做法：使用实例化
std::vector<glm::mat4> modelMatrices(1000);
for (int i = 0; i < 1000; i++) {
    modelMatrices[i] = calculateModelMatrix(i);
}

// 上传实例数据到 VBO
glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * 1000, 
             &modelMatrices[0], GL_STATIC_DRAW);

// 一次绘制所有实例
glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, 1000);
```

---

### 3. 优化缓冲区使用

**静态数据使用 GL_STATIC_DRAW**:
```cpp
// 模型顶点数据（不会改变）
glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
```

**动态数据使用 GL_DYNAMIC_DRAW**:
```cpp
// UBO 数据（每帧更新）
glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
```

**使用 glBufferSubData 更新部分数据**:
```cpp
// 只更新时间变量
glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Globals, time), 
                sizeof(float), &timeValue);
```

---

### 4. 纹理优化

**使用 Mipmap**:
```cpp
glGenerateMipmap(GL_TEXTURE_2D);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
```

**使用纹理图集**:
```cpp
// 将多个小纹理合并到一个大纹理中
// 减少纹理绑定次数
```

**合理设置纹理过滤**:
```cpp
// 像素艺术风格
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

// 真实感渲染
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```

---

### 5. 深度测试优化

**从前往后绘制不透明物体**:
```cpp
// 排序：从近到远
std::sort(opaqueObjects.begin(), opaqueObjects.end(), 
          [&camera](const Object& a, const Object& b) {
    float distA = glm::distance(camera.position, a.position);
    float distB = glm::distance(camera.position, b.position);
    return distA < distB;
});

// 绘制不透明物体
for (auto& obj : opaqueObjects) {
    obj.draw();
}
```

**从后往前绘制透明物体**:
```cpp
// 排序：从远到近
std::sort(transparentObjects.begin(), transparentObjects.end(), 
          [&camera](const Object& a, const Object& b) {
    float distA = glm::distance(camera.position, a.position);
    float distB = glm::distance(camera.position, b.position);
    return distA > distB;
});

// 禁用深度写入
glDepthMask(GL_FALSE);

// 绘制透明物体
for (auto& obj : transparentObjects) {
    obj.draw();
}

// 恢复深度写入
glDepthMask(GL_TRUE);
```

---

### 6. 使用 UBO 共享数据

**不好的做法**:
```cpp
// 每个着色器都单独设置
glUseProgram(shader1);
glUniformMatrix4fv(projLoc1, 1, GL_FALSE, glm::value_ptr(proj));
glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(view));

glUseProgram(shader2);
glUniformMatrix4fv(projLoc2, 1, GL_FALSE, glm::value_ptr(proj));
glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
```

**好的做法**:
```cpp
// 使用 UBO 一次性设置
struct CameraData {
    glm::mat4 proj;
    glm::mat4 view;
};

CameraData data = { proj, view };
cameraUBO.SetData(&data, sizeof(data));

// 所有着色器自动获取数据
glUseProgram(shader1);
// proj 和 view 已经通过 UBO 设置

glUseProgram(shader2);
// proj 和 view 已经通过 UBO 设置
```

---

## 调试技巧

### 1. OpenGL 错误检查宏

```cpp
// 定义错误检查宏
#define GLES_CHECK_ERROR(func) \
    func; \
    { \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            LOGE("OpenGL Error 0x%x at %s:%d in %s", \
                 error, __FILE__, __LINE__, #func); \
        } \
    }

// 使用示例
GLES_CHECK_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
GLES_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
```

**常见错误代码**:
- `0x0500` (GL_INVALID_ENUM): 枚举参数无效
- `0x0501` (GL_INVALID_VALUE): 数值参数无效
- `0x0502` (GL_INVALID_OPERATION): 操作无效
- `0x0503` (GL_STACK_OVERFLOW): 堆栈溢出
- `0x0504` (GL_STACK_UNDERFLOW): 堆栈下溢
- `0x0505` (GL_OUT_OF_MEMORY): 内存不足

---

### 2. 着色器编译检查

```cpp
GLint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // 检查编译状态
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    
    if (status == GL_FALSE) {
        // 获取错误日志
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
        std::vector<GLchar> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        
        LOGE("Shader compilation failed:\n%s", log.data());
        LOGE("Shader source:\n%s", source);
        
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}
```

---

### 3. 着色器链接检查

```cpp
GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // 检查链接状态
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    
    if (status == GL_FALSE) {
        // 获取错误日志
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        
        std::vector<GLchar> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        
        LOGE("Program linking failed:\n%s", log.data());
        
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}
```

---

### 4. FBO 完整性检查

```cpp
bool checkFramebufferStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                LOGE("FBO: Incomplete attachment");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                LOGE("FBO: Missing attachment");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                LOGE("FBO: Incomplete dimensions");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                LOGE("FBO: Unsupported format");
                break;
            default:
                LOGE("FBO: Unknown error 0x%x", status);
                break;
        }
        return false;
    }
    
    return true;
}
```

---

### 5. 枚举着色器 Uniform 变量

```cpp
void printActiveUniforms(GLuint program) {
    GLint uniformCount;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    
    LOGI("Active Uniforms: %d", uniformCount);
    
    for (GLint i = 0; i < uniformCount; i++) {
        char name[256];
        GLsizei length;
        GLint size;
        GLenum type;
        
        glGetActiveUniform(program, i, sizeof(name), 
                          &length, &size, &type, name);
        
        GLint location = glGetUniformLocation(program, name);
        
        const char* typeName = getGLTypeName(type);
        LOGI("  [%d] %s %s (location: %d, size: %d)", 
             i, typeName, name, location, size);
    }
}

const char* getGLTypeName(GLenum type) {
    switch (type) {
        case GL_FLOAT: return "float";
        case GL_FLOAT_VEC2: return "vec2";
        case GL_FLOAT_VEC3: return "vec3";
        case GL_FLOAT_VEC4: return "vec4";
        case GL_INT: return "int";
        case GL_INT_VEC2: return "ivec2";
        case GL_INT_VEC3: return "ivec3";
        case GL_INT_VEC4: return "ivec4";
        case GL_BOOL: return "bool";
        case GL_FLOAT_MAT2: return "mat2";
        case GL_FLOAT_MAT3: return "mat3";
        case GL_FLOAT_MAT4: return "mat4";
        case GL_SAMPLER_2D: return "sampler2D";
        case GL_SAMPLER_CUBE: return "samplerCube";
        default: return "unknown";
    }
}
```

---

### 6. 枚举着色器属性

```cpp
void printActiveAttributes(GLuint program) {
    GLint attributeCount;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    
    LOGI("Active Attributes: %d", attributeCount);
    
    for (GLint i = 0; i < attributeCount; i++) {
        char name[256];
        GLsizei length;
        GLint size;
        GLenum type;
        
        glGetActiveAttrib(program, i, sizeof(name), 
                         &length, &size, &type, name);
        
        GLint location = glGetAttribLocation(program, name);
        
        const char* typeName = getGLTypeName(type);
        LOGI("  [%d] %s %s (location: %d, size: %d)", 
             i, typeName, name, location, size);
    }
}
```

---

### 7. 检查 OpenGL 版本和扩展

```cpp
void printOpenGLInfo() {
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    
    LOGI("OpenGL Vendor: %s", vendor);
    LOGI("OpenGL Renderer: %s", renderer);
    LOGI("OpenGL Version: %s", version);
    LOGI("GLSL Version: %s", glslVersion);
    
    // 获取最大纹理单元数
    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    LOGI("Max Texture Units: %d", maxTextureUnits);
    
    // 获取最大纹理大小
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    LOGI("Max Texture Size: %d", maxTextureSize);
    
    // 获取最大顶点属性数
    GLint maxVertexAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    LOGI("Max Vertex Attributes: %d", maxVertexAttribs);
    
    // 获取最大 UBO 大小
    GLint maxUBOSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUBOSize);
    LOGI("Max UBO Size: %d bytes", maxUBOSize);
}
```

---

### 8. 性能分析

```cpp
class GPUTimer {
public:
    GPUTimer() {
        glGenQueries(1, &query);
    }
    
    ~GPUTimer() {
        glDeleteQueries(1, &query);
    }
    
    void begin() {
        glBeginQuery(GL_TIME_ELAPSED, query);
    }
    
    void end() {
        glEndQuery(GL_TIME_ELAPSED);
    }
    
    GLuint64 getElapsedTime() {
        GLuint64 elapsed;
        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed);
        return elapsed; // 纳秒
    }
    
    double getElapsedTimeMs() {
        return getElapsedTime() / 1000000.0; // 毫秒
    }
    
private:
    GLuint query;
};

// 使用示例
GPUTimer timer;
timer.begin();
// ... 渲染代码 ...
timer.end();
LOGI("Render time: %.2f ms", timer.getElapsedTimeMs());
```

---

### 9. 内存使用检查

```cpp
void checkGPUMemory() {
    // NVIDIA 扩展
    #ifdef GL_NVX_gpu_memory_info
    GLint totalMemory, availableMemory;
    glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemory);
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory);
    
    LOGI("GPU Memory: %d MB total, %d MB available", 
         totalMemory / 1024, availableMemory / 1024);
    #endif
    
    // AMD 扩展
    #ifdef GL_ATI_meminfo
    GLint memInfo[4];
    glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, memInfo);
    LOGI("VBO Free Memory: %d MB", memInfo[0] / 1024);
    
    glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, memInfo);
    LOGI("Texture Free Memory: %d MB", memInfo[0] / 1024);
    #endif
}
```

---

### 10. 调试输出（OpenGL 4.3+）

```cpp
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, 
                              GLenum severity, GLsizei length, 
                              const GLchar* message, const void* userParam) {
    // 忽略非重要消息
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    
    const char* sourceStr = getDebugSource(source);
    const char* typeStr = getDebugType(type);
    const char* severityStr = getDebugSeverity(severity);
    
    LOGE("OpenGL Debug Message:");
    LOGE("  Source: %s", sourceStr);
    LOGE("  Type: %s", typeStr);
    LOGE("  Severity: %s", severityStr);
    LOGE("  Message: %s", message);
}

void enableDebugOutput() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 
                         0, nullptr, GL_TRUE);
}
```

---

## 常见问题和解决方案

### 问题 1: 黑屏或什么都不显示

**可能原因**:
1. 着色器编译/链接失败
2. 顶点数据未正确上传
3. 深度测试配置错误
4. 视口设置错误
5. 相机位置不正确

**解决方案**:
```cpp
// 1. 检查着色器
printActiveUniforms(program);
printActiveAttributes(program);

// 2. 检查顶点数据
LOGI("Vertex count: %d", vertexCount);
LOGI("Index count: %d", indexCount);

// 3. 检查深度测试
GLboolean depthTestEnabled;
glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
LOGI("Depth test enabled: %d", depthTestEnabled);

// 4. 检查视口
GLint viewport[4];
glGetIntegerv(GL_VIEWPORT, viewport);
LOGI("Viewport: %d, %d, %d, %d", viewport[0], viewport[1], viewport[2], viewport[3]);

// 5. 打印相机信息
LOGI("Camera position: (%.2f, %.2f, %.2f)", 
     camera.position.x, camera.position.y, camera.position.z);
```

---

### 问题 2: 纹理显示不正确

**可能原因**:
1. 纹理坐标错误
2. 纹理未正确绑定
3. 纹理单元冲突
4. 纹理格式不匹配

**解决方案**:
```cpp
// 1. 检查纹理绑定
GLint currentTexture;
glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
LOGI("Current texture: %d", currentTexture);

// 2. 检查纹理单元
GLint activeTexture;
glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
LOGI("Active texture unit: GL_TEXTURE%d", activeTexture - GL_TEXTURE0);

// 3. 检查纹理参数
GLint minFilter, magFilter;
glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter);
LOGI("Texture filters: min=%d, mag=%d", minFilter, magFilter);
```

---

### 问题 3: 透明度不工作

**可能原因**:
1. 混合未启用
2. 混合函数设置错误
3. 深度写入未禁用
4. 绘制顺序错误

**解决方案**:
```cpp
// 1. 启用混合
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// 2. 禁用深度写入（透明物体）
glDepthMask(GL_FALSE);

// 3. 从后往前绘制透明物体
std::sort(transparentObjects.begin(), transparentObjects.end(), 
          [&camera](const Object& a, const Object& b) {
    return glm::distance(camera.position, a.position) > 
           glm::distance(camera.position, b.position);
});

// 4. 恢复深度写入
glDepthMask(GL_TRUE);
```

---

### 问题 4: FBO 渲染结果不正确

**可能原因**:
1. FBO 不完整
2. 视口未设置
3. 深度缓冲未附加
4. 纹理格式不匹配

**解决方案**:
```cpp
// 1. 检查 FBO 完整性
if (!checkFramebufferStatus()) {
    LOGE("FBO is not complete!");
}

// 2. 设置正确的视口
glViewport(0, 0, fboWidth, fboHeight);

// 3. 确保附加了深度缓冲
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                          GL_RENDERBUFFER, depthRBO);

// 4. 检查纹理格式
GLint internalFormat;
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, 
                        &internalFormat);
LOGI("Texture internal format: 0x%x", internalFormat);
```

---

## 参考资源

### 官方文档
- [OpenGL ES 3.0 规范](https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf)
- [OpenGL ES 3.0 参考页面](https://www.khronos.org/registry/OpenGL-Refpages/es3.0/)
- [GLFW 文档](https://www.glfw.org/documentation.html)
- [EGL 规范](https://www.khronos.org/registry/EGL/)

### 教程和书籍
- [LearnOpenGL](https://learnopengl.com/) - 优秀的 OpenGL 教程
- [OpenGL ES 3.0 Programming Guide](https://www.opengles-book.com/)
- [Real-Time Rendering](http://www.realtimerendering.com/)

### 工具
- [RenderDoc](https://renderdoc.org/) - OpenGL 调试工具
- [NVIDIA Nsight Graphics](https://developer.nvidia.com/nsight-graphics) - GPU 调试和性能分析
- [AMD Radeon GPU Profiler](https://gpuopen.com/rgp/) - AMD GPU 性能分析

### 库和框架
- [GLAD](https://glad.dav1d.de/) - OpenGL 加载器生成器
- [GLFW](https://www.glfw.org/) - 跨平台窗口和输入库
- [GLM](https://glm.g-truc.net/) - OpenGL 数学库
- [Assimp](https://www.assimp.org/) - 模型加载库
- [stb_image](https://github.com/nothings/stb) - 图像加载库

---

**文档版本**: 1.0  
**最后更新**: 2025-11-05  
**项目**: EGL_Component OpenGL 渲染引擎  
**作者**: Kiro AI Assistant

---

## 附录：项目文件结构

```
EGL_Component/
├── 3rdparty/              # 第三方库
│   ├── assimp/           # 模型加载
│   ├── glad/             # OpenGL 加载器
│   ├── glfw/             # 窗口管理
│   ├── glm/              # 数学库
│   └── SOIL2/            # 图像加载
├── Common/               # 公共类型和宏
├── Component_3DModels/   # 3D 模型渲染
├── Component_Camera/     # 相机系统
├── Component_FBO/        # 帧缓冲对象
├── Component_TextureManager/  # 纹理管理
├── Component_UBO/        # 统一缓冲对象
├── Component_Offscreen/  # 离屏渲染
├── Component_SkyBox/     # 天空盒
├── Component_Box/        # 包围盒
├── Component_Shader_Blinn_Phong/  # Blinn-Phong 着色器
├── Component_Silhouette/ # 轮廓渲染
├── Component_Mouse/      # 鼠标交互
├── ModelLoader/          # 模型加载器
└── Shader/               # 着色器加载
```

---

## 结语

本文档涵盖了 EGL_Component 项目中使用的所有主要 OpenGL API 函数，包括详细的参数说明、使用场景和实际代码示例。通过学习这些 API，你可以：

1. 理解 OpenGL 渲染管线的工作原理
2. 掌握缓冲区、纹理、着色器等核心概念
3. 学会使用 FBO 进行离屏渲染
4. 优化渲染性能
5. 调试和解决常见问题

希望这份文档能帮助你更好地理解和使用 OpenGL！
