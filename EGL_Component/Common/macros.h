#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>

#include <string>


// #include <android/log.h>
// #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
// #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)   
#define DEBUG_GL_ENABLE 1
#if DEBUG_GL_ENABLE
#include <android/log.h>
#define LOG_TAG "CPP TAG"
#define LOGI(...) __android_log_print( ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__ )
#define LOGE(...) __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__ )

#define GLES_CHECK_ERROR(x)                                                         \
	x;																		        \
	{																			    \
		GLenum err(glGetError());                                                   \
		while (err != GL_NO_ERROR)                                                  \
		{                                                                           \
			std::string error;                                                      \
																					\
			switch (err)                                                            \
			{                                                                       \
			case GL_INVALID_OPERATION:                                              \
				error = "invalid operation";                                        \
				break;                                                              \
			case GL_INVALID_ENUM:                                                   \
				error = "invalid enum";                                             \
				break;                                                              \
			case GL_INVALID_VALUE:                                                  \
				error = "invalid value";                                            \
				break;                                                              \
			case GL_OUT_OF_MEMORY:                                                  \
				error = "out of memory";                                            \
				break;                                                              \
			case GL_INVALID_FRAMEBUFFER_OPERATION:                                  \
				error = "invalid framebuffer operation";                            \
				break;                                                              \
			}                                                                       \
																					\
			std::string formattedError = "OpenGL ES : ";                            \
			formattedError = formattedError + error;                                \
			formattedError = formattedError + ", file : ";                          \
			formattedError = formattedError + __FILE__;                             \
			formattedError = formattedError + ", line : ";                          \
			formattedError = formattedError + std::to_string(__LINE__);             \
        	LOGI("%s.\n", formattedError.c_str());			\
			err = glGetError();                                                     \
		}																			\
	}																				\

#else
#define GLES_CHECK_ERROR(x) x
#endif