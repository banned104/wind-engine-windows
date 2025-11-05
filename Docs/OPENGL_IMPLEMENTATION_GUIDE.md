# OpenGL 常用功能实现流程指南

本文档总结了OpenGL中常用功能的完整实现流程，包括关键API调用和最佳实践。

## 目录

1. [纹理系统](#1-纹理系统)
2. [摄像机系统](#2-摄像机系统)
3. [深度测试与模板测试](#3-深度测试与模板测试)
4. [面剔除](#4-面剔除)
5. [帧缓冲对象 (FBO)](#5-帧缓冲对象-fbo)
6. [立方体贴图 (Cubemap)](#6-立方体贴图-cubemap)
7. [实例化渲染](#7-实例化渲染)
8. [抗锯齿 (MSAA)](#8-抗锯齿-msaa)
9. [法线贴图](#9-法线贴图)

---

## 1. 纹理系统

### 1.1 完整流程概述

```
图片文件 → SOIL加载 → OpenGL纹理对象 → 绑定到纹理单元 → Shader采样
```

### 1.2 SOIL 图片加载

**关键API：**

```cpp
// 加载图片并创建OpenGL纹理（一步到位）
GLuint textureID = SOIL_load_OGL_texture(
    "texture.jpg",              // 文件路径
    SOIL_LOAD_AUTO,             // 通道数（自动检测）
    SOIL_CREATE_NEW_ID,         // 创建新纹理ID
    SOIL_FLAG_MIPMAPS |         // 生成Mipmap
    SOIL_FLAG_INVERT_Y |        // 翻转Y轴
    SOIL_FLAG_NTSC_SAFE_RGB     // 颜色安全
);

// 仅加载图片数据（手动创建纹理）
unsigned char* image = SOIL_load_image(
    "texture.jpg",
    &width, &height, &channels,
    SOIL_LOAD_RGB
);
```

### 1.3 OpenGL 纹理创建与配置


**步骤1：生成纹理对象**

```cpp
GLuint textureID;
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_2D, textureID);
```

**步骤2：设置纹理参数**

```cpp
// 纹理环绕方式
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);     // S轴（U）
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);     // T轴（V）

// 纹理过滤方式
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 缩小过滤
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);                // 放大过滤

// 常用环绕模式：
// GL_REPEAT          - 重复纹理
// GL_MIRRORED_REPEAT - 镜像重复
// GL_CLAMP_TO_EDGE   - 边缘拉伸
// GL_CLAMP_TO_BORDER - 边界颜色
```

**步骤3：上传纹理数据**

```cpp
glTexImage2D(
    GL_TEXTURE_2D,      // 纹理目标
    0,                  // Mipmap级别
    GL_RGB,             // 内部格式
    width, height,      // 宽高
    0,                  // 边框（必须为0）
    GL_RGB,             // 数据格式
    GL_UNSIGNED_BYTE,   // 数据类型
    image               // 图片数据
);
glGenerateMipmap(GL_TEXTURE_2D);  // 生成Mipmap

// 释放图片内存
SOIL_free_image_data(image);
glBindTexture(GL_TEXTURE_2D, 0);
```

### 1.4 绑定纹理到Shader

**C++端：**

```cpp
// 激活纹理单元并绑定纹理
glActiveTexture(GL_TEXTURE0);           // 激活纹理单元0
glBindTexture(GL_TEXTURE_2D, textureID);

// 设置Shader中的采样器
GLint location = glGetUniformLocation(shaderProgram, "textureSampler");
glUniform1i(location, 0);  // 对应GL_TEXTURE0
```

**Shader端（Fragment Shader）：**

```glsl
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D textureSampler;

void main() {
    FragColor = texture(textureSampler, TexCoords);
}
```

### 1.5 多纹理使用

```cpp
// 绑定多个纹理
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, diffuseMap);
glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);

glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, specularMap);
glUniform1i(glGetUniformLocation(shader, "material.specular"), 1);
```

---

## 2. 摄像机系统

### 2.1 摄像机核心概念

摄像机由三个要素定义：
- **位置 (Position)**：摄像机在世界空间的坐标
- **方向 (Direction)**：摄像机朝向
- **上向量 (Up)**：定义摄像机的旋转

### 2.2 View矩阵构建

```cpp
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// 构建View矩阵
glm::mat4 view = glm::lookAt(
    cameraPos,              // 摄像机位置
    cameraPos + cameraFront, // 目标点
    cameraUp                // 上向量
);
```

### 2.3 欧拉角摄像机实现


```cpp
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    float Yaw;         // 偏航角
    float Pitch;       // 俯仰角
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;        // FOV
    
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f)) {
        Position = position;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = -90.0f;
        Pitch = 0.0f;
        MovementSpeed = 2.5f;
        MouseSensitivity = 0.1f;
        Zoom = 45.0f;
        updateCameraVectors();
    }
    
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }
    
    void ProcessMouseMovement(float xoffset, float yoffset) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        
        Yaw   += xoffset;
        Pitch += yoffset;
        
        // 限制俯仰角
        if (Pitch > 89.0f)  Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
        
        updateCameraVectors();
    }
    
    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)  Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT)     Position -= Right * velocity;
        if (direction == RIGHT)    Position += Right * velocity;
    }
    
private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
```

### 2.4 投影矩阵

```cpp
// 透视投影
glm::mat4 projection = glm::perspective(
    glm::radians(camera.Zoom),  // FOV
    (float)width / height,       // 宽高比
    0.1f,                        // 近平面
    100.0f                       // 远平面
);

// 正交投影
glm::mat4 projection = glm::ortho(
    -10.0f, 10.0f,  // 左右
    -10.0f, 10.0f,  // 下上
    0.1f, 100.0f    // 近远
);
```

---

## 3. 深度测试与模板测试

### 3.1 深度测试 (Depth Testing)

**启用深度测试：**

```cpp
glEnable(GL_DEPTH_TEST);

// 渲染循环中清除深度缓冲
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

**深度测试函数：**

```cpp
glDepthFunc(GL_LESS);  // 默认：深度值小于当前值通过

// 其他选项：
// GL_ALWAYS   - 总是通过
// GL_NEVER    - 总不通过
// GL_LESS     - 小于通过（默认）
// GL_EQUAL    - 等于通过
// GL_LEQUAL   - 小于等于通过
// GL_GREATER  - 大于通过
// GL_NOTEQUAL - 不等于通过
// GL_GEQUAL   - 大于等于通过
```

**深度写入控制：**

```cpp
glDepthMask(GL_FALSE);  // 禁止写入深度缓冲（只读）
glDepthMask(GL_TRUE);   // 允许写入深度缓冲
```

### 3.2 模板测试 (Stencil Testing)

**启用模板测试：**

```cpp
glEnable(GL_STENCIL_TEST);

// 清除模板缓冲
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
```

**模板函数配置：**

```cpp
glStencilFunc(
    GL_NOTEQUAL,  // 测试函数
    1,            // 参考值
    0xFF          // 掩码
);

glStencilOp(
    GL_KEEP,      // 模板测试失败时的操作
    GL_KEEP,      // 模板测试通过但深度测试失败
    GL_REPLACE    // 两者都通过时的操作
);

// 操作选项：
// GL_KEEP      - 保持当前值
// GL_ZERO      - 设为0
// GL_REPLACE   - 替换为参考值
// GL_INCR      - 增加1（有上限）
// GL_INCR_WRAP - 增加1（循环）
// GL_DECR      - 减少1（有下限）
// GL_DECR_WRAP - 减少1（循环）
// GL_INVERT    - 按位取反
```

**物体描边示例：**


```cpp
// 第一遍：绘制物体，写入模板缓冲
glStencilFunc(GL_ALWAYS, 1, 0xFF);
glStencilMask(0xFF);  // 允许写入
DrawObject();

// 第二遍：绘制放大的物体作为描边
glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
glStencilMask(0x00);  // 禁止写入
glDisable(GL_DEPTH_TEST);
DrawScaledObject();  // 稍微放大
glEnable(GL_DEPTH_TEST);
glStencilMask(0xFF);
```

---

## 4. 面剔除

### 4.1 基本概念

面剔除通过判断三角形的环绕顺序来剔除不可见的面，提高渲染性能。

**启用面剔除：**

```cpp
glEnable(GL_CULL_FACE);
```

### 4.2 剔除配置

```cpp
// 指定剔除哪个面
glCullFace(GL_BACK);   // 剔除背面（默认）
// GL_FRONT            // 剔除正面
// GL_FRONT_AND_BACK   // 剔除正面和背面

// 指定正面的环绕顺序
glFrontFace(GL_CCW);   // 逆时针为正面（默认）
// GL_CW               // 顺时针为正面
```

### 4.3 使用场景

```cpp
// 渲染不透明物体时启用
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
DrawOpaqueObjects();

// 渲染透明物体或双面物体时禁用
glDisable(GL_CULL_FACE);
DrawTransparentObjects();
```

---

## 5. 帧缓冲对象 (FBO)

### 5.1 完整流程

```
创建FBO → 创建附件（颜色/深度/模板） → 绑定附件 → 检查完整性 → 渲染到FBO → 使用纹理
```

### 5.2 创建帧缓冲

```cpp
GLuint fbo;
glGenFramebuffers(1, &fbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
```

### 5.3 创建颜色附件（纹理）

```cpp
GLuint textureColorbuffer;
glGenTextures(1, &textureColorbuffer);
glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 附加到帧缓冲
glFramebufferTexture2D(
    GL_FRAMEBUFFER,           // 帧缓冲目标
    GL_COLOR_ATTACHMENT0,     // 附件类型
    GL_TEXTURE_2D,            // 纹理目标
    textureColorbuffer,       // 纹理ID
    0                         // Mipmap级别
);
```

### 5.4 创建深度和模板附件（RBO）

```cpp
GLuint rbo;
glGenRenderbuffers(1, &rbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);

// 深度和模板组合缓冲
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

// 附加到帧缓冲
glFramebufferRenderbuffer(
    GL_FRAMEBUFFER,
    GL_DEPTH_STENCIL_ATTACHMENT,
    GL_RENDERBUFFER,
    rbo
);

// 或者分别创建
// GL_DEPTH_ATTACHMENT   - 仅深度
// GL_STENCIL_ATTACHMENT - 仅模板
```

### 5.5 检查完整性

```cpp
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR: Framebuffer is not complete!" << std::endl;
}
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

### 5.6 使用帧缓冲

```cpp
// 第一遍：渲染到FBO
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glEnable(GL_DEPTH_TEST);
DrawScene();

// 第二遍：渲染到默认帧缓冲
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);
glDisable(GL_DEPTH_TEST);

// 使用FBO的颜色纹理
glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
DrawScreenQuad();
```

### 5.7 多重渲染目标 (MRT)

```cpp
// 创建多个颜色附件
GLuint colorBuffers[2];
glGenTextures(2, colorBuffers);

for (int i = 0; i < 2; i++) {
    glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
}

// 指定渲染目标
GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
glDrawBuffers(2, attachments);
```

**Fragment Shader：**

```glsl
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    BrightColor = vec4(0.0, 1.0, 0.0, 1.0);
}
```

---

## 6. 立方体贴图 (Cubemap)

### 6.1 创建立方体贴图


```cpp
GLuint cubemapTexture;
glGenTextures(1, &cubemapTexture);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

// 6个面的路径
std::vector<std::string> faces = {
    "right.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    "left.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    "top.jpg",     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    "bottom.jpg",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    "front.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    "back.jpg"     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

int width, height, nrChannels;
for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
        SOIL_free_image_data(data);
    }
}

// 设置纹理参数
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
```

### 6.2 天空盒渲染

**Vertex Shader：**

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;  // 确保深度值为1.0
}
```

**Fragment Shader：**

```glsl
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, TexCoords);
}
```

**渲染代码：**

```cpp
// 最后渲染天空盒
glDepthFunc(GL_LEQUAL);  // 深度值等于1.0也通过
skyboxShader.use();
glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 移除位移
skyboxShader.setMat4("view", view);
skyboxShader.setMat4("projection", projection);

glBindVertexArray(skyboxVAO);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
glDrawArrays(GL_TRIANGLES, 0, 36);
glBindVertexArray(0);
glDepthFunc(GL_LESS);
```

### 6.3 环境映射

**反射：**

```glsl
vec3 I = normalize(Position - cameraPos);
vec3 R = reflect(I, normalize(Normal));
FragColor = texture(skybox, R);
```

**折射：**

```glsl
float ratio = 1.00 / 1.52;  // 空气/玻璃折射率
vec3 I = normalize(Position - cameraPos);
vec3 R = refract(I, normalize(Normal), ratio);
FragColor = texture(skybox, R);
```

---

## 7. 实例化渲染

### 7.1 基本实例化

**使用 glDrawArraysInstanced：**

```cpp
// 准备实例化数据
glm::vec2 translations[100];
int index = 0;
float offset = 0.1f;
for (int y = -10; y < 10; y += 2) {
    for (int x = -10; x < 10; x += 2) {
        glm::vec2 translation;
        translation.x = (float)x / 10.0f + offset;
        translation.y = (float)y / 10.0f + offset;
        translations[index++] = translation;
    }
}

// 绘制100个实例
glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
```

**Vertex Shader（使用 gl_InstanceID）：**

```glsl
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fColor;

uniform vec2 offsets[100];

void main() {
    vec2 offset = offsets[gl_InstanceID];
    gl_Position = vec4(aPos + offset, 0.0, 1.0);
    fColor = aColor;
}
```

### 7.2 实例化数组（推荐）

**创建实例化VBO：**

```cpp
GLuint instanceVBO;
glGenBuffers(1, &instanceVBO);
glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);

glBindVertexArray(quadVAO);
glEnableVertexAttribArray(2);
glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
glBindBuffer(GL_ARRAY_BUFFER, 0);

// 设置为实例化属性
glVertexAttribDivisor(2, 1);  // 每个实例更新一次
```

**Vertex Shader：**

```glsl
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;  // 实例化属性

out vec3 fColor;

void main() {
    gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
    fColor = aColor;
}
```

### 7.3 实例化矩阵


```cpp
// 为每个实例准备模型矩阵
glm::mat4 modelMatrices[1000];
for (unsigned int i = 0; i < 1000; i++) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, positions[i]);
    model = glm::rotate(model, angles[i], glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scales[i]);
    modelMatrices[i] = model;
}

// 创建实例化缓冲
GLuint buffer;
glGenBuffers(1, &buffer);
glBindBuffer(GL_ARRAY_BUFFER, buffer);
glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

// mat4需要4个顶点属性（每个vec4一个）
for (unsigned int i = 0; i < 4; i++) {
    glEnableVertexAttribArray(3 + i);
    glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), 
                          (void*)(sizeof(glm::vec4) * i));
    glVertexAttribDivisor(3 + i, 1);
}
```

**Vertex Shader：**

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;  // 占用location 3-6

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
}
```

---

## 8. 抗锯齿 (MSAA)

### 8.1 启用MSAA

**GLFW窗口创建时：**

```cpp
glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA
GLFWwindow* window = glfwCreateWindow(800, 600, "MSAA", NULL, NULL);

// 启用MSAA
glEnable(GL_MULTISAMPLE);
```

### 8.2 离屏MSAA（使用FBO）

**创建多重采样纹理：**

```cpp
GLuint texColorBufferMultiSampled;
glGenTextures(1, &texColorBufferMultiSampled);
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorBufferMultiSampled);
glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

// 附加到FBO
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                       GL_TEXTURE_2D_MULTISAMPLE, texColorBufferMultiSampled, 0);
```

**创建多重采样RBO：**

```cpp
GLuint rbo;
glGenRenderbuffers(1, &rbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
glBindRenderbuffer(GL_RENDERBUFFER, 0);

glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
```

**解析到普通纹理：**

```cpp
// 渲染到多重采样FBO
glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
DrawScene();

// Blit到普通FBO
glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
glBindFramebuffer(GL_DRAW_FRAMEBUFFER, normalFBO);
glBlitFramebuffer(
    0, 0, width, height,
    0, 0, width, height,
    GL_COLOR_BUFFER_BIT,
    GL_NEAREST
);
```

### 8.3 自定义抗锯齿（FXAA/SMAA）

使用后处理Shader实现，在屏幕空间进行边缘检测和模糊处理。

---

## 9. 法线贴图

### 9.1 切线空间基础

法线贴图存储在切线空间（Tangent Space），需要TBN矩阵转换到世界空间。

**TBN矩阵构成：**
- T (Tangent)：切线，沿U方向
- B (Bitangent)：副切线，沿V方向
- N (Normal)：法线

### 9.2 计算切线和副切线

```cpp
// 对于每个三角形
glm::vec3 pos1, pos2, pos3;
glm::vec2 uv1, uv2, uv3;

glm::vec3 edge1 = pos2 - pos1;
glm::vec3 edge2 = pos3 - pos1;
glm::vec2 deltaUV1 = uv2 - uv1;
glm::vec2 deltaUV2 = uv3 - uv1;

float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

glm::vec3 tangent;
tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
tangent = glm::normalize(tangent);

glm::vec3 bitangent;
bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
bitangent = glm::normalize(bitangent);
```

### 9.3 Vertex Shader


```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    
    // 构建TBN矩阵
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);  // Gram-Schmidt正交化
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));  // 世界空间到切线空间
    
    // 将光照相关向量转换到切线空间
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

### 9.4 Fragment Shader

```glsl
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main() {
    // 从法线贴图采样（范围[0,1]）
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // 转换到[-1,1]范围
    normal = normalize(normal * 2.0 - 1.0);
    
    // 获取漫反射颜色
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    
    // 环境光
    vec3 ambient = 0.1 * color;
    
    // 漫反射
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    
    // 镜面反射
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;
    
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
```

### 9.5 视差贴图（Parallax Mapping）

视差贴图是法线贴图的增强版本，使用高度图创建深度错觉。

**Fragment Shader（简单视差）：**

```glsl
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    float height = texture(depthMap, texCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * heightScale);
    return texCoords - p;
}

void main() {
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
    
    // 边界检查
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
    
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    
    // ... 继续光照计算
}
```

**陡峭视差映射（Steep Parallax Mapping）：**

```glsl
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy * heightScale;
    vec2 deltaTexCoords = P / numLayers;
    
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    
    while(currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    
    return currentTexCoords;
}
```

---

## 附录：常见问题与最佳实践

### A. 纹理相关

**问题：纹理显示为黑色**
- 检查是否正确激活纹理单元
- 确认Shader中uniform采样器已设置
- 验证纹理数据是否成功加载

**最佳实践：**
- 使用Mipmap提高性能和质量
- 对于重复纹理使用GL_REPEAT，边缘使用GL_CLAMP_TO_EDGE
- 压缩纹理格式（DXT/ETC）节省显存

### B. 深度测试

**问题：Z-Fighting（深度冲突）**
- 增大近平面距离
- 使用对数深度缓冲
- 避免过大的远近平面比例

**最佳实践：**
- 近平面不要太近（0.1而非0.001）
- 远平面不要太远（100而非10000）

### C. 帧缓冲

**问题：帧缓冲不完整**
- 检查所有附件格式是否兼容
- 确保至少有一个颜色附件
- 验证纹理/RBO尺寸一致

**最佳实践：**
- 使用RBO存储不需要采样的附件（深度/模板）
- 使用纹理存储需要后续使用的附件（颜色）

### D. 性能优化

**通用建议：**
- 减少状态切换（纹理绑定、Shader切换）
- 使用实例化渲染大量相同物体
- 启用面剔除减少片段处理
- 使用LOD（细节层次）技术
- 批量绘制调用（Batch Rendering）

### E. 调试技巧

```cpp
// 检查OpenGL错误
GLenum err;
while((err = glGetError()) != GL_NO_ERROR) {
    std::cout << "OpenGL error: " << err << std::endl;
}

// 使用调试回调（OpenGL 4.3+）
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, 
                            GLenum severity, GLsizei length, 
                            const GLchar *message, const void *userParam) {
    std::cout << "GL CALLBACK: " << message << std::endl;
}

glEnable(GL_DEBUG_OUTPUT);
glDebugMessageCallback(glDebugOutput, 0);
```

---

## 总结

本文档涵盖了OpenGL开发中最常用的功能模块：

1. **纹理系统**：从图片加载到Shader采样的完整流程
2. **摄像机**：View矩阵构建和交互控制
3. **深度/模板测试**：3D渲染基础和特效实现
4. **面剔除**：性能优化的重要手段
5. **帧缓冲**：离屏渲染和后处理基础
6. **立方体贴图**：天空盒和环境映射
7. **实例化**：高效渲染大量物体
8. **抗锯齿**：提升渲染质量
9. **法线贴图**：增强表面细节

掌握这些核心技术，可以构建出功能完整的3D渲染引擎。建议按照文档顺序逐步学习，每个模块都配有完整的代码示例。

---

**文档版本：** 1.0  
**最后更新：** 2025-11-05
