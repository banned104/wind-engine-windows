#pragma once

#include "macros.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <string>
#include <stdexcept>
#include <utility>

class ShaderProgram {
public:
    /// 使用字符串源码创建着色器程序
    ShaderProgram(const std::string& vertexSrc,
                  const std::string& fragmentSrc)
    {
        compile(vertexSrc, fragmentSrc);
    }

    /// 允许移动，不允许拷贝 ID是一个独一无二的资源, 不能被复制 RAII(资源获取即初始化)
    // operator相当于类中的方法, 
    ShaderProgram(ShaderProgram&& other) noexcept : ID(other.ID) { other.ID = 0; }
    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            if (ID) glDeleteProgram(ID);
            ID = other.ID;
            other.ID = 0;
        }
        return *this;
    }

    // 删除( =delete ) 拷贝; 如果不写下面这两行,C++编译器会自动生成默认的拷贝函数
    // 如果一个实例的 program ID 被另一个实例拷贝, 在两个实例析构函数调用时会重复释放两次导致崩溃或者GPU错误
    ShaderProgram(const ShaderProgram&)            = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ~ShaderProgram() { if (ID) glDeleteProgram(ID); }

    /* ---------- 使用 / 句柄 / uniform ---------- */
    void   use()         const { glUseProgram(ID); }
    GLuint handle()      const { return ID; }
    GLint  uniform(const char* name) const { return glGetUniformLocation(ID, name); }

    // uniform的实用函数
    /*
        example:
            ourShader.use();
            ourShader.setVec3("viewPos", camera.Position);
            ourShader.setFloat("material.shininess", 32.0f);
    */
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
private:
    GLuint ID = 0;

    void compile(const std::string& vsSrc,
                 const std::string& fsSrc)
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER,   vsSrc);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

        ID = glCreateProgram();
        glAttachShader(ID, vs);
        glAttachShader(ID, fs);
        glLinkProgram(ID);
        checkLinkErrors();

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    static GLuint compileShader(GLenum type,
                                const std::string& src)
    {
        GLuint shader = glCreateShader(type);
        const char* csrc = src.c_str();
        glShaderSource(shader, 1, &csrc, nullptr);
        glCompileShader(shader);

        GLint ok{};
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLchar log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            throw std::runtime_error("Shader compile error:\n" + std::string(log));
        }
        return shader;
    }

    void checkLinkErrors()
    {
        GLint ok{};
        glGetProgramiv(ID, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLchar log[1024];
            glGetProgramInfoLog(ID, 1024, nullptr, log);
            throw std::runtime_error("Program link error:\n" + std::string(log));
        }
    }
};
