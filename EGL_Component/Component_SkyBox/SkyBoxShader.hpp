#include "ShaderProgram.hpp"

class SkyBoxShader : public ShaderProgram
{
public:
/*
    ModelProgram()
    : ShaderProgram(kVertexSrc, kFragmentSrc)
*/
    SkyBoxShader() : ShaderProgram( kVertexSrc, kFragmentSrc )
    {}
    ~SkyBoxShader()
    {}

private:
    constexpr static const char* kVertexSrc = R"(
        #version 300 es
        precision mediump float;
        layout (location = 0) in vec3 aPos;
        out vec3 TexCoords;
        uniform mat4 projection;
        uniform mat4 view;
        void main()
        {
            TexCoords = aPos;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }
    )";

    constexpr static const char* kFragmentSrc = R"(
        #version 300 es
        precision mediump float;
        out vec4 FragColor;
        in vec3 TexCoords;
        uniform samplerCube texture1;
        void main()
        {
            FragColor = texture(texture1, TexCoords);
        }
    )";
};
