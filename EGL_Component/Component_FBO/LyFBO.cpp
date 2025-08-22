
#include "LyFBO.h"
#include "macros.h"


/**/


/**/


LyFBO::LyFBO() : fbo(0), rbo(0), tex(0)
{

}

LyFBO::LyFBO(int width, int height, GLint internalFormat, GLenum format, GLenum type)
{
    GLES_CHECK_ERROR(glGenFramebuffers(1, &fbo));
	GLES_CHECK_ERROR(glGenRenderbuffers(1, &rbo));
	GLES_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
	GLES_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	GLES_CHECK_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
	GLES_CHECK_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));

	GLES_CHECK_ERROR(glGenTextures(1, &tex));
	GLES_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, tex));
	GLES_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL));
	GLES_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLES_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLES_CHECK_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0));
	// Test FrameBuffer completness
	GLenum status = GLES_CHECK_ERROR(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("FrameBuffer initialization failed. Error: 0x%x", status);
	}
	GLES_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLES_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

LyFBO::~LyFBO()
{
	GLES_CHECK_ERROR(glDeleteFramebuffers(1, &fbo));
	GLES_CHECK_ERROR(glDeleteRenderbuffers(1, &rbo));
	GLES_CHECK_ERROR(glDeleteTextures(1, &tex));
	fbo = 0;
    rbo = 0;
    tex = 0;
}

void LyFBO::
bind()
{
	GLES_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
}

void LyFBO::unbind()
{
	GLES_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint LyFBO::getFBO()
{
	return fbo;
}

GLuint LyFBO::getTex()
{
	return tex;
}