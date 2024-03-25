
//////////////////////////////////////////////////////////////////////////////////////////

#include <exception>

#include <GL/glew.h>
#ifdef WIN32
	#include <windows.h>
	#include <GL/wglew.h>
#endif

#include "fbo.h"

//////////////////////////////////////////////////////////////////////////////////////////

Fbo::Fbo(int w, int h)
	: height(h)
	, width(w)
{
	//just in case
	glewInit();

	if (!GLEW_EXT_framebuffer_object)
		throw std::exception("No FBO support.");

	glGenFramebuffersEXT(1, &frameBufferId);
    glGenRenderbuffersEXT(1, &depthBufferId);
    glGenTextures(1, &textureId);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBufferId);
    glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, 0);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBufferId);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBufferId);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
		throw std::exception("FBO creation failed.");
}

//////////////////////////////////////////////////////////////////////////////////////////

Fbo::~Fbo()
{
	glDeleteRenderbuffersEXT(1, &depthBufferId);
	glDeleteFramebuffersEXT(1, &frameBufferId);
	glDeleteTextures(1, &textureId);
}

//////////////////////////////////////////////////////////////////////////////////////////

void Fbo::setViewport()
{
	glViewport(0, 0, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////

void Fbo::bind(unsigned int i)
{
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

//////////////////////////////////////////////////////////////////////////////////////////

void Fbo::bindFrameBuffer()
{
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBufferId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBufferId);
}

//////////////////////////////////////////////////////////////////////////////////////////

void Fbo::unbindFrameBuffer()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
