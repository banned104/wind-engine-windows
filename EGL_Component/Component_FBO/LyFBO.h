/**
 * @file LyFBO.h
 * @author yangyuzhi (yangyuzhi@szlanyou.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <glad/glad.h>

#include <string>


class LyFBO
{
public:

    // 默认构造函数
    LyFBO();

    // 构造函数
    // 解释：explicit关键字，用于防止隐式转换
    // 解释：隐式转换，是指将一个类型的值赋给另一个类型的变量时，编译器自动执行的转换
    explicit LyFBO(int width, int height,
                    GLint internalFormat = GL_RGBA8,
                    GLenum format        = GL_RGBA,
                    GLenum type          = GL_UNSIGNED_BYTE);
            
    // 析构函数
    ~LyFBO();

	void bind();
	void unbind();

    GLuint getFBO();

    GLuint getTex();

protected:
    GLuint fbo;
    GLuint rbo;
    GLuint tex;

};
