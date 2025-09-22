#pragma once
#ifndef __gl_h_
#include "glad/glad.h"
#endif
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
#include "../../../Shaders/ShaderClass.h"
#include "../../../Shaders/VBO.h"
#include "../../../Shaders/VAO.h"
#include "../../../Shaders/EBO.h"
#include "../../../Shaders/FrameBuffer.h"
#include "../../../Shaders/Texture.h"

#include <vector>
#include <string>
#include <iostream>
#include "../../../Windows/Log.hpp"

#include "../../../Utils/Crypto/sha256.h"
#include <iomanip>

static std::string HashData(const std::vector<unsigned char>& data) {
	uint8_t hash[32];
	SHA256_CTX1 ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, data.data(), data.size());
	sha256_final(&ctx, hash);

	std::ostringstream oss;
	for (int i = 0; i < 32; ++i)
		oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	return oss.str();
}

class File {
public:
	bool IsBigEndianFile = false;
	const std::vector<unsigned char> RawFile;

	virtual void LoadFile(const std::vector<unsigned char>& rawFile) {

	}

	File() = default;
	File(const std::vector<unsigned char> rawFile) : RawFile(rawFile) {
		LoadFile(RawFile);
	}
	File(const std::vector<unsigned char> rawFile, bool IsBig) : RawFile(rawFile), IsBigEndianFile(IsBig) {
		LoadFile(RawFile);
	}

	virtual void Destroy() {
		// Cleanup if necessary
	}

};