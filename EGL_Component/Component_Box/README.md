# BoundingBoxRenderer 组件

## 概述

`BoundingBoxRenderer` 是一个用于绘制3D模型包围盒轮廓线框的渲染器组件。它可以帮助开发者可视化模型的边界，用于调试、编辑或展示目的。

## 功能特性

- 绘制3D模型的包围盒线框
- 支持自定义线框颜色
- 自动处理深度测试和混合状态
- 轻量级实现，性能开销小
- 易于集成到现有渲染管线中

## 文件结构

```
Component_Box/
├── BoundingBoxRenderer.hpp    # 头文件，包含类定义
├── BoundingBoxRenderer.cpp    # 实现文件
└── README.md                  # 说明文档
```

## 使用方法

### 1. 包含头文件

```cpp
#include "BoundingBoxRenderer.hpp"
```

### 2. 创建和初始化

```cpp
// 创建包围盒渲染器
auto boundingBoxRenderer = std::make_unique<BoundingBoxRenderer>();

// 初始化（需要在OpenGL上下文中调用）
if (!boundingBoxRenderer->initialize()) {
    // 处理初始化失败
    LOGE("Failed to initialize BoundingBoxRenderer");
}
```

### 3. 绘制包围盒

```cpp
// 获取模型的包围盒信息
glm::vec3 minBounds = model->boundsMin();
glm::vec3 maxBounds = model->boundsMax();

// 计算MVP矩阵
glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

// 设置线框颜色（可选，默认为白色）
glm::vec3 color(1.0f, 1.0f, 0.0f); // 黄色

// 绘制包围盒
boundingBoxRenderer->drawBoundingBox(minBounds, maxBounds, mvpMatrix, color);
```

### 4. 清理资源

```cpp
// 手动清理（析构函数会自动调用）
boundingBoxRenderer->cleanup();
```

## 在ModelRenderer中的集成

`BoundingBoxRenderer` 已经集成到 `ModelRenderer` 类中：

```cpp
// 控制包围盒显示
modelRenderer->setBoundingBoxVisible(true);  // 显示包围盒
modelRenderer->setBoundingBoxVisible(false); // 隐藏包围盒

// 检查当前状态
bool isVisible = modelRenderer->isBoundingBoxVisible();
```

## 技术细节

### 着色器

- **顶点着色器**: 执行基本的MVP变换
- **片段着色器**: 输出统一颜色的线框

### 几何体

- 使用8个顶点定义立方体的角点
- 使用12条边（24个索引）定义线框结构
- 通过变换矩阵将单位立方体调整到实际包围盒大小

### 渲染状态

- 启用深度测试但禁用深度写入，确保线框可见但不影响其他物体
- 启用混合以支持透明效果
- 使用 `GL_LINES` 模式绘制线框

## 注意事项

1. 必须在有效的OpenGL上下文中调用 `initialize()` 方法
2. 包围盒的坐标应该在世界坐标系中
3. MVP矩阵应该包含完整的模型-视图-投影变换
4. 线框颜色使用RGB格式，范围为0.0-1.0

## 性能考虑

- 包围盒渲染的性能开销很小（只有24个顶点）
- 建议在调试模式下启用，发布版本中可以禁用
- 可以通过 `setBoundingBoxVisible()` 方法动态控制显示