#include "macros.h"
#include "glm/glm.hpp"
#include "ShaderProgram.hpp"

struct VertexColor
{
    glm::vec2 position;
    glm::vec3 color;
};


struct GlobalsUBO {
        float uTime;
    };

class LoadingViewClass : ShaderProgram {
public:
    // UBO 绑定点
    static constexpr GLuint BINDING_GLOBALS = 0;

    LoadingViewClass() : ShaderProgram( vertex_shader, frag_shader ) {
        GLint program = handle();

        // --- UBO 设置 (保持不变) ---
        GLuint global_index = glGetUniformBlockIndex( program, "Globals" );
        if ( global_index != GL_INVALID_INDEX ) {
            glUniformBlockBinding( program, global_index, BINDING_GLOBALS );
        }
        glGenBuffers( 1, &uboGlobals );
        glBindBuffer( GL_UNIFORM_BUFFER, uboGlobals );
        glBufferData( GL_UNIFORM_BUFFER, sizeof( GlobalsUBO ), nullptr, GL_DYNAMIC_DRAW );
        glBindBufferBase( GL_UNIFORM_BUFFER, BINDING_GLOBALS, uboGlobals );
        glBindBuffer( GL_UNIFORM_BUFFER, 0 );

        // --- VAO, VBO, EBO 设置 ---

        // 1. 定义顶点和索引数据
        VertexColor vertices[] = {
            // 位置(vec2)      // 颜色(vec3)
            { { 1.0f,  1.0f}, {1.0f, 0.0f, 0.0f} }, // 0: 右上角, 红色
            { { 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} }, // 1: 右下角, 绿色
            { {-1.0f, -1.0f}, {0.0f, 0.0f, 1.0f} }, // 2: 左下角, 蓝色
            { {-1.0f,  1.0f}, {1.0f, 1.0f, 0.0f} }  // 3: 左上角, 黄色
        };
        unsigned int indices[] = {
            0, 1, 3, // 第一个三角形
            1, 2, 3  // 第二个三角形
        };

        // 2. 创建并绑定 VAO, VBO, EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 3. 将顶点数据上传到 VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 4. 将索引数据上传到 EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // 5. 设置顶点属性指针
        // 位置属性 (layout location = 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexColor), (void*)offsetof(VertexColor, position));
        glEnableVertexAttribArray(0);
        // 颜色属性 (layout location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor), (void*)offsetof(VertexColor, color));
        glEnableVertexAttribArray(1);

        // 6. 解绑 VAO，防止意外修改
        glBindVertexArray(0);
    }

    ~LoadingViewClass () {
        // 释放所有 OpenGL 资源
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &uboGlobals);
    }

    void updataGlobals( GlobalsUBO& g ) {
        glBindBuffer( GL_UNIFORM_BUFFER, uboGlobals );
        glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( GlobalsUBO ), &g );
        glBindBuffer( GL_UNIFORM_BUFFER, 0 );
    }

    void use() {
        glUseProgram( handle() );
    }

    // 新增的绘制函数
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    GLuint VAO{}, VBO{}, EBO{};
    GLuint uboGlobals{};

    // --- 修改后的着色器代码 ---
    constexpr static const char* vertex_shader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec3 aColor;

        layout(std140) uniform Globals {
            float uTime;
        };

        out vec3 fragColor;

        void main()
        {
            // gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);       // 纯透明 只显示安卓部分            glEnable(GL_BLEND);
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            fragColor = aColor;
        }
    )";

    constexpr static const char* frag_shader = R"(
        #version 330 core
        precision mediump float;
        in vec3 fragColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(fragColor, 0.0); // 使用传入的颜色
        }
    )";
};