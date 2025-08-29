// AxisRenderer.h
// Single-file modern C++17 helper to render RGB axes (X=red, Y=green, Z=blue)
// Dependencies: OpenGL 3.3+ context, GLM (for matrices), glad/glew loaded, and an OpenGL loader bound.
// Design goals: modular, easy to integrate with existing renderer, small shader, configurable length & colors.

#pragma once

#include <string>
#include <array>
#include <vector>
#include <iostream>

#include <glad/glad.h> // or your loader
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class AxisRenderer {
public:
    struct Config {
        float length = 100.0f;                  // length of each axis in world units
        float lineWidth = 5.0f;               // glLineWidth (platform-dependent)
        std::array<glm::vec3, 3> colors{ {     // default colors: X=red, Y=green, Z=blue
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        } };
        bool depthTest = true;                // whether to enable depth testing when rendering axes
        bool originMarker = true;             // render a small marker at origin (tiny cross)
    };

    AxisRenderer() = default;
    explicit AxisRenderer(const Config& cfg) : cfg_(cfg) {}

    ~AxisRenderer() { destroy(); }

    // Initialize GL resources. Call after GL context is ready.
    bool init() {
        if (initialized_) return true;
        if (!compileShaders()) return false;
        setupBuffers();
        initialized_ = true;
        return true;
    }

    // Release GL resources.
    void destroy() {
        if (!initialized_) return;
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
        if (program_) glDeleteProgram(program_);
        vao_ = vbo_ = 0;
        program_ = 0;
        initialized_ = false;
    }

    // Setters
    void setLength(float len) { cfg_.length = len; updateVertexData(); }
    void setLineWidth(float w) { cfg_.lineWidth = w; }
    void setColors(const std::array<glm::vec3, 3>& cols) { cfg_.colors = cols; updateVertexData(); }
    void setDepthTest(bool on) { cfg_.depthTest = on; }

    // Render the axes. Provide model/view/proj matrices (or an MVP combined matrix).
    // Call with GL context bound. This does not change other GL state except depth test and line width,
    // and restores depth test state after rendering.
    void render(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model = glm::mat4(1.0f)) {
        if (!initialized_) {
            if (!init()) return;
        }

        // optional state changes
        GLboolean wasDepth = glIsEnabled(GL_DEPTH_TEST);
        if (cfg_.depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);

        glLineWidth(cfg_.lineWidth);

        glUseProgram(program_);
        glm::mat4 mvp = proj * view * model;
        glUniformMatrix4fv(uniformMVP_, 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(vao_);
        // draw 3 lines (6 vertices: 0->1, 2->3, 4->5)
        glDrawArrays(GL_LINES, 0, 6);

        if (cfg_.originMarker) {
            // draw small origin cross scaled by epsilon
            float e = cfg_.length * 0.02f; // 2% of axis length
            // update origin marker vertices in-place and draw as lines
            // marker vertices are placed after the 6 main axis vertices
            glDrawArrays(GL_LINES, 6, 6);
        }

        glBindVertexArray(0);
        glUseProgram(0);

        // restore depth state
        if (wasDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    }

private:
    Config cfg_;
    bool initialized_ = false;

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint program_ = 0;
    GLint attribPos_ = -1;
    GLint attribColor_ = -1;
    GLint uniformMVP_ = -1;

    // interleaved vertex: position (vec3) + color (vec3)
    std::vector<float> vertexData_; // will hold 12* (3+3) floats (6 main + 6 marker)

    // Minimal shader sources (OpenGL 3.3 core)
    const char* vsSrc_ = R"GLSL(
#version 330 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inColor;
uniform mat4 uMVP;
out vec3 vColor;
void main() {
    gl_Position = uMVP * vec4(inPos, 1.0);
    vColor = inColor;
}
)GLSL";

    const char* fsSrc_ = R"GLSL(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
)GLSL";

    bool compileShaders() {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vsSrc_, nullptr);
        glCompileShader(vs);
        if (!checkCompileStatus(vs, "Vertex Shader")) { glDeleteShader(vs); return false; }

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fsSrc_, nullptr);
        glCompileShader(fs);
        if (!checkCompileStatus(fs, "Fragment Shader")) { glDeleteShader(vs); glDeleteShader(fs); return false; }

        program_ = glCreateProgram();
        glAttachShader(program_, vs);
        glAttachShader(program_, fs);
        glLinkProgram(program_);

        GLint linked = 0;
        glGetProgramiv(program_, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len = 0; glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(program_, len, nullptr, &log[0]);
            std::cerr << "Program link error: " << log << '\n';
            glDeleteShader(vs); glDeleteShader(fs);
            glDeleteProgram(program_);
            program_ = 0;
            return false;
        }

        glDetachShader(program_, vs);
        glDetachShader(program_, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        // fetch uniform/attrib locations
        attribPos_ = 0;            // layout(location=0)
        attribColor_ = 1;          // layout(location=1)
        uniformMVP_ = glGetUniformLocation(program_, "uMVP");
        return true;
    }

    bool checkCompileStatus(GLuint shader, const char* name) {
        GLint ok = 0; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            std::cerr << name << " compile error:\n" << log << '\n';
            return false;
        }
        return true;
    }

    void setupBuffers() {
        // build vertex data for 3 axes (6 verts) + small origin marker (3 axes * 2 verts)
        vertexData_.clear();
        vertexData_.reserve((6 + 6) * 6);

        // main axes (each axis is a line from origin to +length)
        addAxis(glm::vec3(0.0f), glm::vec3(cfg_.length, 0.0f, 0.0f), cfg_.colors[0]); // X
        addAxis(glm::vec3(0.0f), glm::vec3(0.0f, cfg_.length, 0.0f), cfg_.colors[1]); // Y
        addAxis(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, cfg_.length), cfg_.colors[2]); // Z

        // origin marker: small lines centered at origin along each axis (negative -> positive small epsilon)
        float e = cfg_.length * 0.02f;
        addAxis(glm::vec3(-e, 0.0f, 0.0f), glm::vec3(e, 0.0f, 0.0f), cfg_.colors[0]);
        addAxis(glm::vec3(0.0f, -e, 0.0f), glm::vec3(0.0f, e, 0.0f), cfg_.colors[1]);
        addAxis(glm::vec3(0.0f, 0.0f, -e), glm::vec3(0.0f, 0.0f, e), cfg_.colors[2]);

        if (vao_ == 0) glGenVertexArrays(1, &vao_);
        if (vbo_ == 0) glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertexData_.size() * sizeof(float), vertexData_.data(), GL_DYNAMIC_DRAW);

        // position (location = 0)
        glEnableVertexAttribArray(attribPos_);
        glVertexAttribPointer(attribPos_, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(0));
        // color (location = 1)
        glEnableVertexAttribArray(attribColor_);
        glVertexAttribPointer(attribColor_, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // helper: append axis line (two verts)
    void addAxis(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color) {
        // vertex A
        vertexData_.push_back(a.x); vertexData_.push_back(a.y); vertexData_.push_back(a.z);
        vertexData_.push_back(color.r); vertexData_.push_back(color.g); vertexData_.push_back(color.b);
        // vertex B
        vertexData_.push_back(b.x); vertexData_.push_back(b.y); vertexData_.push_back(b.z);
        vertexData_.push_back(color.r); vertexData_.push_back(color.g); vertexData_.push_back(color.b);
    }

    // when user changes length or colors after init, update VBO in-place
    void updateVertexData() {
        if (!initialized_) return; // will be built on init
        // rebuild CPU-side data
        setupBuffers();
        // setupBuffers writes to VBO already via glBufferData
    }
};

// Usage example (pseudo-code):
// AxisRenderer::Config cfg; cfg.length = 2.0f; AxisRenderer axes(cfg);
// axes.init();
// in render loop: axes.render(viewMat, projMat, modelMat);

// Notes / Extensions:
// - If you need thicker, resolution-independent axes, replace GL_LINES with thin cylinder geometry per axis.
// - If you want to hide axes when zoomed out, perform distance-based scaling in CPU before render.
// - This small utility encapsulates shader, buffers, and simple configuration -> modular and portable.
