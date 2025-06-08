#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "stb_image/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include <array>

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>

// Include VivaEngine Shaders
#include "../Shaders/ShaderClass.h"
#include "../Shaders/VBO.h"
#include "../Shaders/VAO.h"
#include "../Shaders/EBO.h"
#include "../Shaders/FrameBuffer.h"
#include "../Shaders/Texture.h"

#include "../World/Camera.h"

#include "../Windows/Log.hpp"

struct meshData {
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
};

class Model
{
public:
	Model(const char* VertexShader, const char* FragmentShader, std::vector<GLfloat> verts, std::vector<GLuint> ind)
		: shaderProgram(VertexShader, FragmentShader), vertices(std::move(verts)), indices(std::move(ind)),
		ModelVBO(vertices), ModelEBO(indices)
	{
		Log("Shader Program Created using: " + std::string(VertexShader) + " and " + std::string(FragmentShader), EType::BLUE);

		ModelVAO.Bind();
		ModelVBO.Bind();
		ModelEBO.Bind();

		ModelVAO.LinkAttrib(ModelVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
		ModelVAO.LinkAttrib(ModelVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		ModelVAO.LinkAttrib(ModelVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		ModelVAO.LinkAttrib(ModelVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
		ModelVAO.Unbind();
		ModelVBO.Unbind();
		ModelEBO.Unbind();
	}

	void Draw() {
		ModelVAO.Bind();
		if (shaderProgram.ID != 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		}
	}

	void SetMaterialParameter(const char* name, glm::vec3 value) {
		shaderProgram.Activate();
		glUniform3f(glGetUniformLocation(shaderProgram.ID, name), value.x, value.y, value.z);
	}
	void SetMaterialParameter(const char* name, glm::vec4 value) {
		shaderProgram.Activate();
		glUniform4f(glGetUniformLocation(shaderProgram.ID, name), value.x, value.y, value.z, value.w);
	}
	void SetMaterialParameter(const char* name, float value) {
		shaderProgram.Activate();
		glUniform1f(glGetUniformLocation(shaderProgram.ID, name), value);
	}
	void SetMaterialParameter(const char* name, int value) {
		shaderProgram.Activate();
		glUniform1i(glGetUniformLocation(shaderProgram.ID, name), value);
	}
	void SetMaterialParameter(const char* name, Texture texture) {
		shaderProgram.Activate();
		texture.texUnit(shaderProgram, "tex0", 0);
	}
	void SetMaterialParameter(const char* name, glm::mat4 value) {
		shaderProgram.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void Destroy() {
		shaderProgram.Delete();
		ModelVAO.Delete();
		ModelVBO.Delete();
		ModelEBO.Delete();
	}

	Shader shaderProgram;

private:
	// Vertices coordinates
	//     COORDINATES vec3     /        COLORS vec3         /    TexCoord vec2  /        NORMALS vec3      //
	std::vector <GLfloat> vertices;

	// Indices for vertices order
	std::vector <GLuint> indices;

	VAO ModelVAO;
	VBO ModelVBO;
	EBO ModelEBO;
};

/*
// Vertices coordinates
GLfloat vertices[] =
{ //     COORDINATES     /        COLORS          /    TexCoord   /        NORMALS       //
	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side

	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,     -0.8f, 0.5f,  0.0f, // Left Side

	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.0f, 0.5f, -0.8f, // Non-facing side

	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.8f, 0.5f,  0.0f, // Right side

	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.0f, 0.5f,  0.8f  // Facing side
};

// Indices for vertices order
GLuint indices[] =
{
	0, 1, 2, // Bottom side
	0, 2, 3, // Bottom side
	4, 6, 5, // Left side
	7, 9, 8, // Non-facing side
	10, 12, 11, // Right side
	13, 15, 14 // Facing side
};

GLfloat lightVertices[] =
{ //     COORDINATES     //
	-0.1f, -0.1f,  0.1f,
	-0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f,  0.1f,
	-0.1f,  0.1f,  0.1f,
	-0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f,  0.1f
};

GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};
*/