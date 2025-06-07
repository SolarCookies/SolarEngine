#pragma once

#include <glad/glad.h>
#include <iostream>

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	GLuint FBO;
	GLuint RBO;
	GLuint texture_id;
	const GLint WIDTH = 800;
	const GLint HEIGHT = 600;

	void create_framebuffer();
	void rescale_framebuffer(float width, float height);
	void Bind();
	void Unbind();
	void Delete();
};