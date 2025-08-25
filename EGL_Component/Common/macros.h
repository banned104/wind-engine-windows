#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#define DEBUG_GL_ENABLE 1
#if DEBUG_GL_ENABLE
#include <iostream>
#define LOG_TAG "CPP TAG"
#define LOGI(...) std::cout << "INFO: " << __VA_ARGS__ << std::endl
#define LOGE(...) std::cerr << "ERROR: " << __VA_ARGS__ << std::endl

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
			std::string formattedError = "OpenGL : ";                               \
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