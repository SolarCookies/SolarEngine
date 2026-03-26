#pragma once
#ifndef __gl_h_
#include "glad/glad.h"
#endif

#include <gli/gli.hpp>
#include <cstdint>

/* 1581 */
enum dbModelUnitId_e : __int32
{
	dbModelUnitId_Scenegraph = 0x0,
	dbModelUnitId_UNUSED_1 = 0x1,
	dbModelUnitId_Locator = 0x2,
	dbModelUnitId_Light = 0x3,
	dbModelUnitId_Volumes = 0x4,
	dbModelUnitId_Extents = 0x5,
	dbModelUnitId_TriHits = 0x6,
	dbModelUnitId_Textures = 0x7,
	dbModelUnitId_Joints = 0x8,
	dbModelUnitId_AStar = 0x9,
	dbModelUnitId_GardenSetup = 0xA,
	dbModelUnitId_Mirrors = 0xB,
	dbModelUnitId_HeatHaze = 0xC,
	dbModelUnitId_AStarLayer2 = 0xD,
	dbModelUnitId_AStarBreakConnectionLayer1 = 0xE,
	dbModelUnitId_AStarBreakConnectionLayer2 = 0xF,
	dbModelUnitId_ShadowGeometry = 0x10,
	dbModelUnitId_LightVolumes = 0x11,
	dbModelUnitId_ClimbPoles = 0x12,
	dbModelUnitId_MoldGeometry = 0x13,
	dbModelUnitId_HavokData = 0x14,
	dbModelUnitId_VertexPath = 0x15,
	dbModelUnitId_HavokMopp = 0x16,
	dbModelUnitId_HavokMesh = 0x17,
	dbModelUnitId_SwitchMaskNew = 0x18,
	dbModelUnitId_VehicleSkinGeometry = 0x19,
	dbModelUnitId_MAX = 0x1A,
};

struct dbModelUnit_s
{
	dbModelUnitId_e id;
	uint32_t data; //offset to data
};

struct dbModelUnit_s_x64_wrapper
{
	dbModelUnit_s x32_data;
	void* data; //x64 memory offset to data
};

/* 11760 */
struct dbTriHitPosArray_s
{
	unsigned char type;
	unsigned char fp;
	unsigned char stride;
	unsigned char __UNUSED;
	uint32_t numPositions;
	uint32_t positions;
};

/* 11760 */
struct dbTriHitPosArray_s_x64_wrapper
{
	dbTriHitPosArray_s x32_data;
	void* positions; //Might need to be a uint32_t as to not be 8 bytes on 64bit if being read directly from the file
};

/* 11761 */
struct dbTriHit_s
{
	uint32_t numParts;
	dbTriHitPosArray_s posArray;
};


/* 11707 */
const struct mlVec
{
	float x;
	float y;
	float z;
};

/* 11762 */
struct dbMirrorUnit_s
{
	uint16_t numTriangles;
	uint16_t startTriangle;
};

/* 11763 */
struct dbMirror_s
{
	uint16_t numMirrors;
	dbMirrorUnit_s* mirrorList;
	mlVec* vertexList;
	uint16_t* triangleList;
};

/* 11684 */
struct assetId_s
{
	char text[128]; //aid_texture_pinata_ui_icon_log for example
};


/* 989 */
enum dbTextureFormat_e : int32_t
{
	dbTextureFormat_UNKNOWN = 0x0,
	dbTextureFormat_DXT1 = 0x1,
	dbTextureFormat_DXT3 = 0x2,
	dbTextureFormat_DXT5 = 0x3,
	dbTextureFormat_A8R8G8B8 = 0x4,
	dbTextureFormat_X8R8G8B8 = 0x5,
	dbTextureFormat_LIN_A8R8G8B8 = 0x6,
	dbTextureFormat_LIN_X8R8G8B8 = 0x7,
	dbTextureFormat_L8 = 0x8,
	dbTextureFormat_A8L8 = 0x9,
	dbTextureFormat_R5G6B5 = 0xA,
	dbTextureFormat_A4R4G4B4 = 0xB,
	dbTextureFormat_DXN = 0xC,
	dbTextureFormat_DXT3A = 0xD,
	dbTextureFormat_G8R8 = 0xE,
	dbTextureFormat_MAX = 0xF,
};

/* 11764 */
struct dbTexture_s
{
	dbTextureFormat_e format;
	//D3DTexture* d3dHeader; //originally this was directx
	GLint OpenGL_TextureID; // we can just store the OpenGL texture ID here instead of the D3D texture pointer (Its the same size so it should work fine)
	uint16_t width;
	uint16_t height;
	unsigned char type;
	unsigned char flags;
	unsigned char userDefined;
	unsigned char maxLOD;
	unsigned char framesPerSecond;
	unsigned char numFrames;
	unsigned char currentFrameLoaded;
	unsigned char requiredFrame;
	uint32_t imageDataStart_x32;
	uint32_t sizeOfOneFrame;
};

struct dbTexture_s_x64_wrapper
{
	dbTexture_s x32_texturedata;
	void* imageDataStart_x64;
};


/* 11765 */
struct dbModelTextureUnit_s
{
	uint32_t numTextures;
	dbTexture_s** textureHeaders;
	assetId_s** textureAssetId;
};

/* 11661 */
struct vec3d
{
	float x;
	float y;
	float z;
};

/* 11766 */
struct dbMoldGeometryVertex_s
{
	vec3d pos;
	float intensity;
};

/* 11767 */
struct dbMoldGeometryTriangle_s
{
	uint32_t idx[3];
	int32_t attribute;
};

/* 11768 */
struct dbMoldGeometry_s
{
	dbMoldGeometryVertex_s* vertices;
	uint32_t numVertices;
	dbMoldGeometryTriangle_s* triangles;
	uint32_t numTriangles;
	vec3d aabbMin;
	vec3d aabbMax;
};

struct dbModelClimbPole_s;
struct dbExtent_s;
struct dbModelJoint_s;
struct dbLight_s;
struct dbLightVolume_s;
struct dbLocator_s;
struct dbScenegraph_s;
struct dbVolume_s;
struct dbModelSwitch_s;

/* 11785 */
struct dbModelRuntime_s
{
	dbTriHit_s* trihits;
	dbMirror_s* mirror;
	struct sgGraph* sg;
	dbModelTextureUnit_s* texpalette;
	struct dbShadowGeometry_s* shadowGeometry;
	dbMoldGeometry_s* moldGeometry;
	const dbModelClimbPole_s* climbPoles;
	const dbExtent_s* extent;
	const dbModelJoint_s* joint;
	const dbLight_s* light;
	const dbLightVolume_s* lightVolume;
	const dbLocator_s* locator;
	const dbScenegraph_s* sceneGraph;
	const dbVolume_s* volume;
	const dbModelSwitch_s* switchMask;
};



/* 11786 */
const struct dbModel_s
{
	dbModelUnit_s* units;
	uint32_t numUnits;
	uint32_t flags;
	uint32_t headerSize;
	dbModelRuntime_s* runtime;
};

