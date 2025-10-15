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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "ShaderProgram.hpp"
#include "macros.h"

/**
 * 包围盒渲染器类
 * 用于绘制3D模型的包围盒轮廓线框
 */
class BoundingBoxRenderer {
public:
    /**
     * 构造函数
     */
    BoundingBoxRenderer();
    
    /**
     * 析构函数
     */
    ~BoundingBoxRenderer();
    
    /**
     * 初始化包围盒渲染器
     * @return 初始化是否成功
     */
    bool initialize();
    
    /**
     * 绘制包围盒
     * @param minBounds 包围盒最小坐标
     * @param maxBounds 包围盒最大坐标
     * @param mvpMatrix MVP变换矩阵
     * @param color 线框颜色，默认为白色
     */
    void drawBoundingBox(const glm::vec3& minBounds, 
                        const glm::vec3& maxBounds,
                        const glm::mat4& mvpMatrix,
                        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));
    
    /**
     * 清理资源
     */
    void cleanup();
    
private:
    /**
     * 创建包围盒的顶点数据
     */
    void createBoundingBoxGeometry();
    
    /**
     * 包围盒着色器程序
     */
    class BoundingBoxProgram : public ShaderProgram {
    public:
        BoundingBoxProgram() : ShaderProgram(kVertexSrc, kFragmentSrc) {
            // 获取uniform位置
            mvpLocation = glGetUniformLocation(handle(), "uMVP");
            colorLocation = glGetUniformLocation(handle(), "uColor");
        }
        
        void setMVP(const glm::mat4& mvp) {
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        }
        
        void setColor(const glm::vec3& color) {
            glUniform3fv(colorLocation, 1, glm::value_ptr(color));
        }
        
    private:
        GLint mvpLocation = -1;
        GLint colorLocation = -1;
        
        // 顶点着色器源码
        #ifdef __ANDROID__
                static constexpr const char* kVertexSrc = R"(
            #version 310 es
            precision mediump float;
            
            layout(location = 0) in vec3 aPosition;
            
            uniform mat4 uMVP;
            
            void main() {
                gl_Position = uMVP * vec4(aPosition, 1.0);
            }
        )";
        
        // 片段着色器源码
        static constexpr const char* kFragmentSrc = R"(
            #version 310 es
            precision mediump float;
            
            uniform vec3 uColor;
            
            out vec4 FragColor;
            
            void main() {
                FragColor = vec4(uColor, 1.0);
            }
        )";
        #else
        static constexpr const char* kVertexSrc = R"(
            #version 330 core
            precision mediump float;
            
            layout(location = 0) in vec3 aPosition;
            
            uniform mat4 uMVP;
            
            void main() {
                gl_Position = uMVP * vec4(aPosition, 1.0);
            }
        )";
        
        // 片段着色器源码
        static constexpr const char* kFragmentSrc = R"(
            #version 330 core
            precision mediump float;
            
            uniform vec3 uColor;
            
            out vec4 FragColor;
            
            void main() {
                FragColor = vec4(uColor, 1.0);
            }
        )";
        #endif
    };
    
    std::unique_ptr<BoundingBoxProgram> mProgram;
    
    // OpenGL对象
    GLuint mVAO = 0;
    GLuint mVBO = 0;
    GLuint mEBO = 0;
    
    // 包围盒几何数据
    static constexpr int VERTEX_COUNT = 8;  // 立方体8个顶点
    static constexpr int INDEX_COUNT = 24;  // 12条边，每条边2个顶点索引
    
    bool mInitialized = false;
};