#include"Texture.h"
#include <gli/gli.hpp>

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	// Assigns the type of the texture ot the texture object
	type = texType;

	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;
	// Flips the image so it appears right side up
	stbi_set_flip_vertically_on_load(true);
	// Reads the image from a file and stores it in bytes
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);
	// Assigns the texture to a Texture Unit
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	// Configures the type of algorithm that is used to make the image smaller or bigger
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Configures the way the texture repeats (if it does at all)
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Extra lines in case you choose to use GL_CLAMP_TO_BORDER
	// float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	// Assigns the image to the OpenGL Texture object
	glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	// Generates MipMaps
	glGenerateMipmap(texType);

	// Deletes the image data as it is already in the OpenGL Texture object
	stbi_image_free(bytes);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(texType, 0);
}

Texture::Texture(const char* DDSimage)
{
	gli::texture texture = gli::load(DDSimage);
	if (texture.empty()) {
		std::cerr << "Failed to load texture with gli." << std::endl;
	}

	// Create OpenGL texture
	GLuint testTextureID = 0;

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
	GLenum target = GL.translate(texture.target());

	glGenTextures(1, &testTextureID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(target, testTextureID);

	glm::tvec3<GLsizei> extent = texture.extent();
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (std::size_t level = 0; level < texture.levels(); ++level) {
		glm::tvec3<GLsizei> levelExtent = texture.extent(level);
		if (gli::is_compressed(texture.format())) {
			glCompressedTexImage2D(
				target, static_cast<GLint>(level), format.Internal,
				levelExtent.x, levelExtent.y, 0,
				static_cast<GLsizei>(texture.size(level)),
				texture.data(0, 0, level)
			);
		}
		else {
			glTexImage2D(
				target, static_cast<GLint>(level), format.Internal,
				levelExtent.x, levelExtent.y, 0,
				format.External, format.Type,
				texture.data(0, 0, level)
			);
		}
	}

	ID = testTextureID;
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	// Shader needs to be activated before changing the value of a uniform
	shader.Activate();
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}

void Texture::Bind()
{
	glBindTexture(type, ID);
}

void Texture::Unbind()
{
	glBindTexture(type, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}