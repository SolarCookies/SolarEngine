#pragma once
#include <vector>
#include <string>
#include "../../../Utils/ZLibHelpers.h"
#include "../../../../../Windows/Log.hpp"
#include "half/half.hpp"

struct LowVector2 {
	half_float::half u, v; // 4 bytes total
};
struct Vector3 {
	float x, y, z;
};
struct Vector2 {
	float u, v;
};


struct VertexBlock {
	Vector3 position;
	Vector3 normal;
	int normalOffset = 0;
	bool hasNormal = false;
	Vector2 texCoord;
	int texCoordOffset = 0;
	bool hasTexCoord = false;
	Vector3 VertexColor;
	int VertexColorOffset = 0;
	bool hasVertexColor = false;
};


inline static float byteswap_float(float value) {
	uint32_t temp_int;
	// Copy the float's bytes to an integer of the same size
	std::memcpy(&temp_int, &value, sizeof(float));
	// Byteswap the integer
	temp_int = _byteswap_ulong(temp_int);
	float result;
	// Copy the bytes back to a float
	std::memcpy(&result, &temp_int, sizeof(float));
	return result;
}

inline static half_float::half byteswap_half(half_float::half value) {
	uint16_t temp_int;
	// Copy the half's bytes to an integer of the same size
	std::memcpy(&temp_int, &value, sizeof(half_float::half));
	// Byteswap the integer
	temp_int = _byteswap_ushort(temp_int);
	half_float::half result;
	// Copy the bytes back to a half
	std::memcpy(&result, &temp_int, sizeof(half_float::half));
	return result;
}

inline Vector3 ReadVector3FromSignedShorts(std::vector<unsigned char> data) {
	Vector3 vec;
	int16_t x, y, z;
	memcpy(&x, &data[0], sizeof(int16_t));
	memcpy(&y, &data[2], sizeof(int16_t));
	memcpy(&z, &data[4], sizeof(int16_t));
	
	vec.x = static_cast<float>(x) / 32767.0f; // Normalize to -1.0 to 1.0
	vec.y = static_cast<float>(y) / 32767.0f;
	vec.z = static_cast<float>(z) / 32767.0f;
	return vec;
}

inline VertexBlock ConstructVertexBlockFromSize(int size, bool bigEndian, std::vector<unsigned char> block) {
	VertexBlock Vert;
	memcpy(&Vert.position, &block[0], sizeof(Vector3)); // Read Position (Always at the start of the block)

	if (bigEndian) { // TIP
		Vert.position.x = byteswap_float(Vert.position.x);
		Vert.position.y = byteswap_float(Vert.position.y);
		Vert.position.z = byteswap_float(Vert.position.z);

		if (size == 52) {
			LowVector2 lv;
			Vector2 uv;
			memcpy(&lv, &block[32], sizeof(LowVector2));
			if (bigEndian) {
				lv.u = byteswap_half(lv.u);
				lv.v = byteswap_half(lv.v);
			}
			uv.u = static_cast<float>(lv.u);
			uv.v = static_cast<float>(lv.v);
			Vert.texCoord = uv;
			Vert.hasTexCoord = true;
		}
		else if (size == 48) {
			LowVector2 lv;
			Vector2 uv;
			memcpy(&lv, &block[28], sizeof(LowVector2));

			lv.u = byteswap_half(lv.u);
			lv.v = byteswap_half(lv.v);

			uv.u = static_cast<float>(lv.u);
			uv.v = static_cast<float>(lv.v);
			Vert.texCoord = uv;
			Vert.hasTexCoord = true;
		}
	}
	else { // PC GLFW
		if (size == 76) {
			memcpy(&Vert.texCoord, &block[36], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36;
		}
		else if (size == 72) {
			memcpy(&Vert.texCoord, &block[36], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36;
		}
		else if (size == 68) {
			memcpy(&Vert.texCoord, &block[36], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36;
		}
		else if (size == 64) {
			memcpy(&Vert.texCoord, &block[36], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36;
		}
		else if (size == 60) {
			float uv[2];
			memcpy(&uv, &block[36], sizeof(float) * 2);
			Vert.texCoord.u = uv[0];
			Vert.texCoord.v = uv[1];
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36; //This isnt correct at 40
			std::vector<unsigned char> data;
			data.resize(6);
			memcpy(&data[0], &block[12], 6); // Read Normal (3 signed shorts, 6 bytes)
			Vert.normal = ReadVector3FromSignedShorts(data);
		}
		else if (size == 56) {
			float uv[2];
			memcpy(&uv, &block[36], sizeof(float) * 2);
			Vert.texCoord.u = uv[0];
			Vert.texCoord.v = uv[1];
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36; //This isnt correct at 40
			std::vector<unsigned char> data;
			data.resize(6);
			memcpy(&data[0], &block[12], 6); // Read Normal (3 signed shorts, 6 bytes)
			Vert.normal = ReadVector3FromSignedShorts(data);
		}
		else if (size == 52) {
			float uv[2];
			memcpy(&uv, &block[36], sizeof(float) * 2);
			Vert.texCoord.u = uv[0];
			Vert.texCoord.v = uv[1];
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 36;
			std::vector<unsigned char> data;
			data.resize(6);
			memcpy(&data[0], &block[12], 6); // Read Normal (3 signed shorts, 6 bytes)
			Vert.normal = ReadVector3FromSignedShorts(data);
		}
		else if (size == 48) {
			float uv[2];
			memcpy(&uv, &block[28], sizeof(float) * 2); //36, 30
			Vert.texCoord.u = uv[0];
			Vert.texCoord.v = uv[1];
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 28; //This isnt correct at 40
			std::vector<unsigned char> data;
			data.resize(6);
			memcpy(&data[0], &block[12], 6); // Read Normal (3 signed shorts, 6 bytes)
			Vert.normal = ReadVector3FromSignedShorts(data);
		}
		else if (size == 44) {
			memcpy(&Vert.texCoord, &block[20], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 24;
		}
		else if (size == 40) {
			unsigned char uv[2];
			memcpy(&uv, &block[24], 2);
			Vert.texCoord.u = static_cast<float>(uv[0]) / 255.0f; // Normalize to 0.0 to 1.0
			Vert.texCoord.v = static_cast<float>(uv[1]) / 255.0f;
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 24;
		}
		else if (size == 36) {
			memcpy(&Vert.texCoord, &block[20], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 20;
		}
		else if (size == 34) {
			memcpy(&Vert.texCoord, &block[24], sizeof(Vector2));
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 24;
		}
		else if (size == 24) {
			LowVector2 lv;
			Vector2 uv;
			memcpy(&lv, &block[20], sizeof(LowVector2));
			uv.u = static_cast<float>(lv.u);
			uv.v = static_cast<float>(lv.v);
			Vert.texCoord = uv;
			Vert.hasTexCoord = true;
			Vert.texCoordOffset = 20;
		}
		else if (size == 32) {
			//No UVs
		}
	}

	return Vert;
}