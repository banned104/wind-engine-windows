# Wind Engine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Wind Engine 是一个跨平台的现代化 C++ 3D 渲染引擎，专注于高性能模型渲染、实例化绘制和相机交互。支持 Windows 桌面环境和 Android 移动平台。

在新能源车安卓车机上添加低占用的空调风效果(也许低)，替代Unity，已经在车机上实测过并且正常运行。

**注意**: 需要手动下载GLFW源文件并直接拷贝到3rdparty/GLFW目录下，其余库诸如Assimp已经预编译为 Windows/Android 下可用的二进制文件。
**注意**: 项目使用VS2022/2026进行编辑和编译，编译为Android可用的.so 文件需要运行.bat文件，已经做多平台化适配，无需修改代码即可编译为.so。

## 功能特性

### 效果预览

![Wind Engine 渲染效果预览](Docs\PixPin_2025-10-26_01-08-52.png)

### 核心渲染功能
- **跨平台支持**: Windows (GLFW + OpenGL Core) 和 Android (EGL + OpenGL ES 3.0)
- **3D 模型加载**: 基于 Assimp 库，支持多种 3D 格式 (OBJ, FBX, glTF 等)
- **实例化渲染**: 支持大量重复模型的高效渲染
- **着色器系统**: Blinn-Phong 光照模型，支持 GLSL 着色器
- **相机控制**: 自由视角相机，支持鼠标/触摸交互
- **物体拾取**: 基于颜色缓冲的精确物体选择
- **纹理管理**: 统一的纹理资源管理系统

### 高级特性
- **离屏渲染**: 帧缓冲对象 (FBO) 支持
- **包围盒渲染**: 3D 模型边界框可视化
- **天空盒系统**: 环境背景渲染
- **轴辅助显示**: 3D 坐标轴可视化工具
- **多线程渲染**: 异步模型加载和渲染
- **统一缓冲对象**: UBO 管理全局渲染状态

## 项目架构

### 目录结构
```
Wind Engine/
├── CMakeLists.txt              # 主构建配置
├── main.cpp                    # 桌面版入口点 (GLFW)
├── jni_main.cpp               # Android 版入口点 (JNI)
├── Compile_exe.bat            # Windows 编译脚本
├── compile_so.bat             # Android 编译脚本
├── Docs/                      # 技术文档
├── models/                    # 3D 模型资源
└── EGL_Component/             # 核心引擎库
    ├── 3rdparty/              # 第三方依赖库
    ├── Common/                # 公共类型和宏定义
    ├── Component_3DModels/    # 3D 模型渲染器
    ├── Component_Camera/      # 相机系统
    ├── Component_Shader_Blinn_Phong/  # 着色器组件
    ├── Component_Mouse/       # 交互控制
    ├── Component_FBO/         # 帧缓冲对象
    ├── Component_UBO/         # 统一缓冲对象
    ├── Component_TextureManager/  # 纹理管理
    ├── ModelLoader/           # 模型加载器
    └── Shader/                # 着色器工具
```

### 核心组件

#### ModelRenderer (渲染器核心)
负责整体渲染流程控制，集成相机、着色器、模型加载等功能模块。

#### 模型加载系统
- `ModelLoader_Universal`: 通用模型加载器
- `ModelLoader_Universal_Instancing`: 实例化模型加载器  
- `ModelLoader_GLTF`: glTF 格式专用加载器

#### 着色器系统
- `WindShader`: 主要的 Blinn-Phong 着色器
- `PhongModelProgram`: 光照模型程序
- GLSL 到 C++ 头文件的自动转换工具

#### 相机与交互
- `Camera`: 视图矩阵管理和相机变换
- `CameraInteractor`: 鼠标/触摸输入处理

## 构建与编译

### 环境要求

**Windows:**
- CMake 3.16+
- MinGW-w64 (GCC) 或 MSVC 2022
- Ninja 构建系统
- Python 3.x (用于着色器转换)

**Android:**
- Android NDK 23+
- CMake 3.16+
- Python 3.x (用于着色器验证)

### 编译方法

#### Windows 桌面版
```bash
# 使用提供的批处理脚本
./Compile_exe.bat

# 或手动编译
cmake -B build_win -S . -G "Ninja"
cmake --build build_win
```

#### Android 版本
```bash
# 验证着色器 (可选)
python EGL_Component/Component_Shader_Blinn_Phong/validate_shaders.py

# 编译 Android SO 库
./compile_so.bat
```

### 依赖库

- **Assimp**: 3D 模型加载
- **GLM**: 数学库 (矩阵、向量运算)
- **GLFW**: 桌面窗口管理 (仅 Windows)
- **GLAD**: OpenGL 加载器 (仅 Windows)
- **SOIL2**: 纹理加载

## 使用方法

### 基本使用示例

```cpp
#include "EGL_Component/Component_3DModels/ModelRenderer.hpp"

int main() {
    // 1. 初始化窗口系统
    GLFWwindow* window = /* 创建 GLFW 窗口 */;
    
    // 2. 创建渲染器
    std::string modelDir = "path/to/models";
    ModelRenderer* renderer = new ModelRenderer(window, modelDir, 800, 600);
    
    // 3. 启动渲染循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // 渲染器在后台线程自动运行
    }
    
    delete renderer;
    return 0;
}
```

### 交互控制

```cpp
// 鼠标/触摸事件处理
void onTouchDown(float x, float y) {
    if (renderer && renderer->getInteractor()) {
        renderer->getInteractor()->onMouseDown(x, y, 
            CameraInteractor::MouseButton::Left);
        renderer->requestPick(); // 触发物体拾取
    }
}

void onTouchMove(float x, float y) {
    if (renderer && renderer->getInteractor()) {
        renderer->getInteractor()->onMouseMove(x, y);
    }
}
```

### 着色器开发

引擎支持 GLSL 着色器的自动转换：

1. 编写 `.glsl` 着色器文件
2. 运行 Python 转换脚本：
   ```bash
   python Component_Shader_Blinn_Phong/Convert_GLSL_to_h.py
   ```
3. 生成的 `.h` 头文件可直接在 C++ 中使用

## 平台差异

### Windows vs Android
```cpp
#ifdef __ANDROID__
    // Android: EGL + OpenGL ES 3.0
    #include <EGL/egl.h>
    #include <GLES3/gl3.h>
#else
    // Windows: GLFW + OpenGL Core
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#endif
```

### 实例化渲染
```cpp
#ifdef ENABLE_INSTANCING
    #include "FlexableTouchPad_Instancing.hpp"
#else
    #include "FlexableTouchPad.hpp"
#endif
```

## 开发文档

项目包含详细的技术文档：

- [3D 模型加载与 GPU 传输流程](Docs/3D_LOADING_GPU_PIPELINE.md)
- [模型加载系统详解](Docs/MODEL_LOADING_SYSTEM.md)
- [C++ 开发者路线图](Docs/CPP_Developer_Roadmap.md)
- [纹理加载修复指南](Docs/TEXTURE_LOADING_FIX.md)
- [纹理单元问题排查](Docs/TEXTURE_UNIT_Corruption.md)

## 性能特性

### 渲染优化
- 实例化绘制减少 Draw Call
- VBO/VAO 缓存优化
- 纹理单元复用
- 视锥体裁剪

### 内存管理
- 智能指针管理资源生命周期
- 纹理缓存池避免重复加载
- 异步模型加载不阻塞主线程

### 多线程架构
- 主线程处理用户交互
- 渲染线程专注图形绘制
- 资源加载线程后台处理

## 贡献指南

1. Fork 项目仓库
2. 创建功能分支 (`git checkout -b feature/new-feature`)
3. 提交更改 (`git commit -am 'Add new feature'`)
4. 推送到分支 (`git push origin feature/new-feature`)
5. 创建 Pull Request

### 编码规范
- 使用 C++17 标准
- 遵循组件化架构模式
- 每个组件独立目录结构
- 包含平台差异化处理
- 添加适当的注释和文档

## 许可证

本项目采用 [MIT License](LICENSE) 许可证。

MIT License 是最宽松的开源许可证之一，您可以：
- 自由使用、修改、分发本软件
- 用于商业目的
- 创建衍生作品

唯一要求是在所有副本中保留原始版权声明和许可证文本。

## 联系方式

- 项目仓库: [wind-engine-windows](https://github.com/banned104/wind-engine-windows)
- 问题反馈: 请使用 GitHub Issues

## 更新日志

### 最新版本
- 支持跨平台渲染 (Windows/Android)
- 实现实例化渲染系统
- 集成 Blinn-Phong 光照模型
- 添加物体拾取功能
- 优化纹理管理系统