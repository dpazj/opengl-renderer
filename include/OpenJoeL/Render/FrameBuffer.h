#pragma once

#include <glm/glm.hpp>
#include <glload/gl_4_0.h>
#include <glload/gl_load.h>


class FrameBuffer
{
public:

	FrameBuffer();
	void Bind();
	void Unbind();
	void Clear();
	void AddRenderBuffer(GLuint width, GLuint height);
	void Attach2DTexture(GLuint texture, GLenum attachment, GLenum target = GL_TEXTURE_2D);
	void AttachTexture(GLuint texture);

	void RemoveDrawBuffer();
	void RemoveReadBuffer();

	void CheckFrameBufferStatus();

private:
	
	GLuint m_fbo = 0;
	GLuint m_rbo = 0;

};