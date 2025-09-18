#pragma once

#include<glad/glad.h>
#include<stb_image/stb_image.h>
#include <vector>

#include"shaderClass.h"

class Texture
{
public:
	GLuint ID;
	GLenum type;
	Texture() = default;
	Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
	Texture(const char* DDSimage);
	Texture(const std::vector<unsigned char>& DDSdata);

	// Assigns a texture unit to a texture
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	// Binds a texture
	void Bind();
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();
};