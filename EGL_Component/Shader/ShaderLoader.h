/*
    安卓用
*/

#ifndef _SHADER_LOADER_
#define _SHADER_LOADER_

#include <GLES3/gl3.h>

#if defined(__ANDROID__)
    // 如果是为 Android 平台编译，__ANDROID__ 宏会被 NDK 编译器自动定义
    #include <android/log.h>
    #define Shader_LOGD(...) __android_log_print(ANDROID_LOG_WARN, "Shader", __VA_ARGS__)
#else
    #include <stdio.h>
    // 使用 do-while(0) 是一个标准技巧，可以使宏在任何地方都像一个安全的单条语句
    #define Shader_LOGD(...) do { printf("Shader: "); printf(__VA_ARGS__); printf("\n"); } while(0)
#endif
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