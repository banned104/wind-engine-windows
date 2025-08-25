#ifndef _SHADER_LOADER_
#define _SHADER_LOADER_

#include<glad/glad.h>


#include <stdio.h>
// 使用 do-while(0) 是一个标准技巧，可以使宏在任何地方都像一个安全的单条语句
#define Shader_LOGD(...) do { printf("Shader: "); printf(__VA_ARGS__); printf("\n"); } while(0)

class Shader {

private:
    GLint program;
    const char* vertexShader;
    const char* fragmentShader;

    GLint initShader(const char *source, int type);

public:
    Shader(const char* vertexShader,const char* fragmentShader);

    int use();

    void release();

};

#endif