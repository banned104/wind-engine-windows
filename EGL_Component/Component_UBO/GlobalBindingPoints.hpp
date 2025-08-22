#pragma once
#include <GLES3/gl3.h>
/**
 * @brief 定义整个应用程序中使用的全局 Uniform Buffer Object (UBO) 绑定点。
 *
 * 使用这个文件可以确保：
 * 1. 绑定点在整个项目中是唯一的，避免冲突。
 * 2. 提高代码可读性，用有意义的名称代替“魔法数字”。
 * 3. 方便管理和添加新的全局 UBO。
 */
namespace UboBindingPoints {
    // 绑定点给相机矩阵 (View, Projection)
    static constexpr GLuint Matrices = 0;

    // 绑定点给场景光照信息
    static constexpr GLuint Lights = 1;

    // 绑定点给时间、分辨率等全局变量
    static constexpr GLuint Globals = 2;

    // ... 未来可以继续添加新的绑定点 ...
    // static constexpr GLuint Materials = 3;
    // static constexpr GLuint SkeletonBones = 4;
}