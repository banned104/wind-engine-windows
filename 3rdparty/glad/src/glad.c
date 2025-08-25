/*
 * GLAD OpenGL loader implementation - Simplified version
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/* Function pointer variables */
void (*glad_glClear)(GLbitfield mask) = NULL;
void (*glad_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void (*glad_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void (*glad_glEnable)(GLenum cap) = NULL;
void (*glad_glDisable)(GLenum cap) = NULL;
void (*glad_glBlendFunc)(GLenum sfactor, GLenum dfactor) = NULL;
void (*glad_glDepthFunc)(GLenum func) = NULL;
void (*glad_glDepthMask)(GLboolean flag) = NULL;
void (*glad_glActiveTexture)(GLenum texture) = NULL;
void (*glad_glBindTexture)(GLenum target, GLuint texture) = NULL;
void (*glad_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) = NULL;
void (*glad_glGenerateMipmap)(GLenum target) = NULL;
void (*glad_glTexParameteri)(GLenum target, GLenum pname, GLint param) = NULL;
void (*glad_glGenVertexArrays)(GLsizei n, GLuint *arrays) = NULL;
void (*glad_glBindVertexArray)(GLuint array) = NULL;
void (*glad_glGenBuffers)(GLsizei n, GLuint *buffers) = NULL;
void (*glad_glBindBuffer)(GLenum target, GLuint buffer) = NULL;
void (*glad_glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage) = NULL;
void (*glad_glEnableVertexAttribArray)(GLuint index) = NULL;
void (*glad_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) = NULL;
void (*glad_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices) = NULL;
void (*glad_glDrawArrays)(GLenum mode, GLint first, GLsizei count) = NULL;
void (*glad_glUseProgram)(GLuint program) = NULL;
GLint (*glad_glGetUniformLocation)(GLuint program, const char *name) = NULL;
void (*glad_glUniform1i)(GLint location, GLint v0) = NULL;
void (*glad_glUniform1f)(GLint location, GLfloat v0) = NULL;
void (*glad_glUniform3fv)(GLint location, GLsizei count, const GLfloat *value) = NULL;
void (*glad_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
void (*glad_glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) = NULL;
void (*glad_glBindBufferBase)(GLenum target, GLuint binding, GLuint buffer) = NULL;
void (*glad_glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) = NULL;
void (*glad_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) = NULL;
void (*glad_glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint *value) = NULL;
void (*glad_glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) = NULL;
void (*glad_glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) = NULL;
void (*glad_glVertexAttribDivisor)(GLuint index, GLuint divisor) = NULL;
void (*glad_glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) = NULL;
void (*glad_glGenFramebuffers)(GLsizei n, GLuint *framebuffers) = NULL;
void (*glad_glBindFramebuffer)(GLenum target, GLuint framebuffer) = NULL;
void (*glad_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
void (*glad_glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
GLenum (*glad_glCheckFramebufferStatus)(GLenum target) = NULL;
void (*glad_glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers) = NULL;
void (*glad_glBindRenderbuffer)(GLenum target, GLuint renderbuffer) = NULL;
void (*glad_glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void (*glad_glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void (*glad_glDeleteVertexArrays)(GLsizei n, const GLuint *arrays) = NULL;
void (*glad_glDeleteBuffers)(GLsizei n, const GLuint *buffers) = NULL;
void (*glad_glDeleteTextures)(GLsizei n, const GLuint *textures) = NULL;
void (*glad_glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers) = NULL;
void (*glad_glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers) = NULL;
void (*glad_glDeleteProgram)(GLuint program) = NULL;
void (*glad_glDeleteShader)(GLuint shader) = NULL;
void (*glad_glDetachShader)(GLuint program, GLuint shader) = NULL;
void (*glad_glLinkProgram)(GLuint program) = NULL;
void (*glad_glAttachShader)(GLuint program, GLuint shader) = NULL;
GLuint (*glad_glCreateProgram)(void) = NULL;
GLuint (*glad_glCreateShader)(GLenum type) = NULL;
void (*glad_glShaderSource)(GLuint shader, GLsizei count, const char *const*string, const GLint *length) = NULL;
void (*glad_glCompileShader)(GLuint shader) = NULL;
void (*glad_glGetShaderiv)(GLuint shader, GLenum pname, GLint *params) = NULL;
void (*glad_glGetProgramiv)(GLuint program, GLenum pname, GLint *params) = NULL;
void (*glad_glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, char *infoLog) = NULL;
void (*glad_glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, char *infoLog) = NULL;
GLuint (*glad_glGetUniformBlockIndex)(GLuint program, const char *uniformBlockName) = NULL;
void (*glad_glPixelStorei)(GLenum pname, GLint param) = NULL;
GLenum (*glad_glGetError)(void) = NULL;
void (*glad_glGetIntegerv)(GLenum pname, GLint *data) = NULL;
GLboolean (*glad_glIsEnabled)(GLenum cap) = NULL;

/* Function loading */
int gladLoadGL(void) {
    /* Load function pointers using glfwGetProcAddress */
    glad_glClear = (void (*)(GLbitfield))glfwGetProcAddress("glClear");
    glad_glClearColor = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat))glfwGetProcAddress("glClearColor");
    glad_glViewport = (void (*)(GLint, GLint, GLsizei, GLsizei))glfwGetProcAddress("glViewport");
    glad_glEnable = (void (*)(GLenum))glfwGetProcAddress("glEnable");
    glad_glDisable = (void (*)(GLenum))glfwGetProcAddress("glDisable");
    glad_glBlendFunc = (void (*)(GLenum, GLenum))glfwGetProcAddress("glBlendFunc");
    glad_glDepthFunc = (void (*)(GLenum))glfwGetProcAddress("glDepthFunc");
    glad_glDepthMask = (void (*)(GLboolean))glfwGetProcAddress("glDepthMask");
    glad_glActiveTexture = (void (*)(GLenum))glfwGetProcAddress("glActiveTexture");
    glad_glBindTexture = (void (*)(GLenum, GLuint))glfwGetProcAddress("glBindTexture");
    glad_glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*))glfwGetProcAddress("glTexImage2D");
    glad_glGenerateMipmap = (void (*)(GLenum))glfwGetProcAddress("glGenerateMipmap");
    glad_glTexParameteri = (void (*)(GLenum, GLenum, GLint))glfwGetProcAddress("glTexParameteri");
    glad_glGenVertexArrays = (void (*)(GLsizei, GLuint*))glfwGetProcAddress("glGenVertexArrays");
    glad_glBindVertexArray = (void (*)(GLuint))glfwGetProcAddress("glBindVertexArray");
    glad_glGenBuffers = (void (*)(GLsizei, GLuint*))glfwGetProcAddress("glGenBuffers");
    glad_glBindBuffer = (void (*)(GLenum, GLuint))glfwGetProcAddress("glBindBuffer");
    glad_glBufferData = (void (*)(GLenum, GLsizeiptr, const void*, GLenum))glfwGetProcAddress("glBufferData");
    glad_glEnableVertexAttribArray = (void (*)(GLuint))glfwGetProcAddress("glEnableVertexAttribArray");
    glad_glVertexAttribPointer = (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*))glfwGetProcAddress("glVertexAttribPointer");
    glad_glDrawElements = (void (*)(GLenum, GLsizei, GLenum, const void*))glfwGetProcAddress("glDrawElements");
    glad_glDrawArrays = (void (*)(GLenum, GLint, GLsizei))glfwGetProcAddress("glDrawArrays");
    glad_glUseProgram = (void (*)(GLuint))glfwGetProcAddress("glUseProgram");
    glad_glGetUniformLocation = (GLint (*)(GLuint, const char*))glfwGetProcAddress("glGetUniformLocation");
    glad_glUniform1i = (void (*)(GLint, GLint))glfwGetProcAddress("glUniform1i");
    glad_glUniform1f = (void (*)(GLint, GLfloat))glfwGetProcAddress("glUniform1f");
    glad_glUniform3fv = (void (*)(GLint, GLsizei, const GLfloat*))glfwGetProcAddress("glUniform3fv");
    glad_glUniformMatrix4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*))glfwGetProcAddress("glUniformMatrix4fv");
    glad_glUniformBlockBinding = (void (*)(GLuint, GLuint, GLuint))glfwGetProcAddress("glUniformBlockBinding");
    glad_glBindBufferBase = (void (*)(GLenum, GLuint, GLuint))glfwGetProcAddress("glBindBufferBase");
    glad_glBufferSubData = (void (*)(GLenum, GLintptr, GLsizeiptr, const void*))glfwGetProcAddress("glBufferSubData");
    glad_glReadPixels = (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*))glfwGetProcAddress("glReadPixels");
    glad_glClearBufferuiv = (void (*)(GLenum, GLint, const GLuint*))glfwGetProcAddress("glClearBufferuiv");
    glad_glClearBufferfi = (void (*)(GLenum, GLint, GLfloat, GLint))glfwGetProcAddress("glClearBufferfi");
    glad_glDrawElementsInstanced = (void (*)(GLenum, GLsizei, GLenum, const void*, GLsizei))glfwGetProcAddress("glDrawElementsInstanced");
    glad_glVertexAttribDivisor = (void (*)(GLuint, GLuint))glfwGetProcAddress("glVertexAttribDivisor");
    glad_glBlitFramebuffer = (void (*)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))glfwGetProcAddress("glBlitFramebuffer");
    glad_glGenFramebuffers = (void (*)(GLsizei, GLuint*))glfwGetProcAddress("glGenFramebuffers");
    glad_glBindFramebuffer = (void (*)(GLenum, GLuint))glfwGetProcAddress("glBindFramebuffer");
    glad_glFramebufferTexture2D = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint))glfwGetProcAddress("glFramebufferTexture2D");
    glad_glFramebufferRenderbuffer = (void (*)(GLenum, GLenum, GLenum, GLuint))glfwGetProcAddress("glFramebufferRenderbuffer");
    glad_glCheckFramebufferStatus = (GLenum (*)(GLenum))glfwGetProcAddress("glCheckFramebufferStatus");
    glad_glGenRenderbuffers = (void (*)(GLsizei, GLuint*))glfwGetProcAddress("glGenRenderbuffers");
    glad_glBindRenderbuffer = (void (*)(GLenum, GLuint))glfwGetProcAddress("glBindRenderbuffer");
    glad_glRenderbufferStorage = (void (*)(GLenum, GLenum, GLsizei, GLsizei))glfwGetProcAddress("glRenderbufferStorage");
    glad_glRenderbufferStorageMultisample = (void (*)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))glfwGetProcAddress("glRenderbufferStorageMultisample");
    glad_glDeleteVertexArrays = (void (*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteVertexArrays");
    glad_glDeleteBuffers = (void (*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteBuffers");
    glad_glDeleteTextures = (void (*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteTextures");
    glad_glDeleteFramebuffers = (void (*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteFramebuffers");
    glad_glDeleteRenderbuffers = (void (*)(GLsizei, const GLuint*))glfwGetProcAddress("glDeleteRenderbuffers");
    glad_glDeleteProgram = (void (*)(GLuint))glfwGetProcAddress("glDeleteProgram");
    glad_glDeleteShader = (void (*)(GLuint))glfwGetProcAddress("glDeleteShader");
    glad_glDetachShader = (void (*)(GLuint, GLuint))glfwGetProcAddress("glDetachShader");
    glad_glLinkProgram = (void (*)(GLuint))glfwGetProcAddress("glLinkProgram");
    glad_glAttachShader = (void (*)(GLuint, GLuint))glfwGetProcAddress("glAttachShader");
    glad_glCreateProgram = (GLuint (*)(void))glfwGetProcAddress("glCreateProgram");
    glad_glCreateShader = (GLuint (*)(GLenum))glfwGetProcAddress("glCreateShader");
    glad_glShaderSource = (void (*)(GLuint, GLsizei, const char* const*, const GLint*))glfwGetProcAddress("glShaderSource");
    glad_glCompileShader = (void (*)(GLuint))glfwGetProcAddress("glCompileShader");
    glad_glGetShaderiv = (void (*)(GLuint, GLenum, GLint*))glfwGetProcAddress("glGetShaderiv");
    glad_glGetProgramiv = (void (*)(GLuint, GLenum, GLint*))glfwGetProcAddress("glGetProgramiv");
    glad_glGetShaderInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, char*))glfwGetProcAddress("glGetShaderInfoLog");
    glad_glGetProgramInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, char*))glfwGetProcAddress("glGetProgramInfoLog");
    glad_glGetUniformBlockIndex = (GLuint (*)(GLuint, const char*))glfwGetProcAddress("glGetUniformBlockIndex");
    glad_glPixelStorei = (void (*)(GLenum, GLint))glfwGetProcAddress("glPixelStorei");
    glad_glGetError = (GLenum (*)(void))glfwGetProcAddress("glGetError");
    glad_glGetIntegerv = (void (*)(GLenum, GLint*))glfwGetProcAddress("glGetIntegerv");
    glad_glIsEnabled = (GLboolean (*)(GLenum))glfwGetProcAddress("glIsEnabled");

    /* Check if all functions were loaded successfully */
    if (!glad_glClear || !glad_glClearColor || !glad_glViewport || !glad_glEnable || !glad_glDisable) {
        return 0;
    }

    return 1;
}
