#pragma once


#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#else
// GLFW + GLAD
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <string>
#include <stdexcept>


class IntFBO
{
public:

    // 默认构造函数
    IntFBO();

    // 构造函数
    // 解释：explicit关键字，用于防止隐式转换
    // 解释：隐式转换，是指将一个类型的值赋给另一个类型的变量时，编译器自动执行的转换
    explicit IntFBO(int width, int height,
                    GLint internalFormat,
                    GLenum format,
                    GLenum type);
            
    // 析构函数
    ~IntFBO();

	void bind();
	void unbind();

    GLuint getFBO();

    GLuint getTex();

protected:
    GLuint fbo;
    GLuint rbo;
    GLuint tex;

};
