#pragma once
#include <wtypes.h>

enum dbTextureFormat_e : __int32
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

struct D3DTexture;
struct dbTexture_s; //This would be the vdat for the textures, this is omitted because its just a pointer
struct dbShadowGeometry_s;

const struct mlVec
{
	float x;
	float y;
	float z;
};

struct assetId_s
{
	char text[128];
};

struct dbModelTextureUnit_s
{
	unsigned int numTextures;
	dbTexture_s** textureHeaders;
	assetId_s** textureAssetId;
};

struct vec3d
{
	float x;
	float y;
	float z;
};

struct dbMoldGeometryVertex_s
{
	vec3d pos;
	float intensity;
};

struct dbMoldGeometryTriangle_s
{
	unsigned int idx[3];
	int attribute;
};

struct dbMoldGeometry_s
{
	dbMoldGeometryVertex_s* vertices;
	unsigned int numVertices;
	dbMoldGeometryTriangle_s* triangles;
	unsigned int numTriangles;
	vec3d aabbMin;
	vec3d aabbMax;
};

struct mlRot_s
{
	float p;
	float y;
	float r;
};

struct dbModelClimbPoleUnitNode_s
{
	mlVec wp;
	mlRot_s wr;
	float yaw;
	float height;
	int jointIndex;
	int type;
	float angleLimit;
	float radius;
};

struct dbModelClimbPoleUnit_s
{
	int numPathNodes;
	dbModelClimbPoleUnitNode_s* pathNodes;
};

const struct dbModelClimbPole_s
{
	int numClimbPoles;
	dbModelClimbPoleUnit_s* pathList;
};

struct dbExtent_s_proximitySphere
{
	float radius;
};

const struct dbExtent_s
{
	dbExtent_s_proximitySphere proximitySphere;
	BYTE boundingSphere[16];
	BYTE boundingCylinder[20];
	BYTE boundingBox[24];
};

const struct dbModelJoint_s
{
	unsigned __int8 lookupTable[76];
};

const struct dbLight_s
{
	unsigned int no;
};

struct vec4d
{
	float x;
	float y;
	float z;
	float w;
};

struct dbLightVolumeUnit_s
{
	int type;
	vec3d position;
	float radius;
	vec4d colour;
	int jointIndex;
	vec3d direction;
	vec3d up;
	float fov;
	float lightShaftAttenuationRadius;
	int isMainLight;
	int colourTextureIndex;
	int noiseTextureIndex;
	float* keyframedIntensity;
	int keyframedIntensitySize;
	float animationDurationSecs;
	float animationTimestepSecs;
};

struct dbModelTextureUnit_s
{
	unsigned int numTextures;
	dbTexture_s** textureHeaders;
	assetId_s** textureAssetId;
};

const struct dbLightVolume_s
{
	dbLightVolumeUnit_s* lightVolumeList;
	int lightVolumeListSize;
	dbModelTextureUnit_s lightTextureList;
};

const struct dbLocator_s
{
	unsigned int numLocators;
};

struct _ndGroup_s
{
	unsigned __int8 type;
	unsigned __int8 passMask;
	unsigned __int16 id;
	_ndGroup_s* child;
	_ndGroup_s* sibling;
	_ndGroup_s* parent;
};

struct _ndVisGroup_s
{
	unsigned __int8 type;
	unsigned __int8 __pad;
	unsigned __int16 targetNodeIdListSize;
	unsigned __int16* targetNodeIdList;
	_ndVisGroup_s* child;
	_ndVisGroup_s* sibling;
	_ndVisGroup_s* parent;
};

struct dbModelTextureUnit_s
{
	unsigned int numTextures;
	dbTexture_s** textureHeaders;
	assetId_s** textureAssetId;
};

const struct dbScenegraph_s
{
	_ndGroup_s* sgRoot;
	_ndVisGroup_s* visRoot;
	dbModelTextureUnit_s textureList;
	char** textureMapping;
	int textureMappingSize;
	int numJoints;
	struct _blendShapeUnit_s* blendShapeList;
	int blendShapeListSize;
	struct _customShaderConst_s* customVsConstList;
	int customVsConstListSize;
	int vsConstBase;
	struct _customShaderConst_s* customPsConstList;
	int customPsConstListSize;
	int psConstBase;
	char** shaderContextMapping;
	int shaderContextMappingSize;
	int visArraySize;
	char** colourDisplacementNameMapping;
	int colourDisplacementNameMappingSize;
	struct _bitUnit_s* bitUnitList;
	int bitUnitListSize;
	int numExtendedJoints;
	void* vertexBufferList;
	int vertexBufferListSize;
	void* indexBufferList;
	int indexBufferListSize;
	void* vertexDeclarationList;
	int vertexDeclarationListSize;
	int shadersHaveBeenBuilt;
	struct _membraneUnit_s* membraneUnitList;
	int membraneUnitListSize;
	void* _gamesideAllocationPtr;
	int maxNumBlendShapeInfluences;
	unsigned int flags;
	float furPhysicsSphereConstraintOffset;
	float furPhysicsSphereConstraintRadius;
	void* _SPARE_1;
};

const struct dbVolume_s
{
	unsigned int numVolumes;
	float cullRadius;
};

const struct dbModelSwitch_s
{
	unsigned __int8 numSwitches;
	unsigned __int8 numStatesPerSwitch;
	unsigned __int8 pad_1;
	unsigned __int8 pad_2;
};

struct dbMirrorUnit_s
{
	unsigned __int16 numTriangles;
	unsigned __int16 startTriangle;
};

struct dbMirror_s
{
	unsigned __int16 numMirrors;
	dbMirrorUnit_s* mirrorList;
	mlVec* vertexList;
	unsigned __int16* triangleList;
};

struct dbTriHitPosArray_s
{
	unsigned __int8 type;
	unsigned __int8 fp;
	unsigned __int8 stride;
	unsigned __int8 __UNUSED;
	unsigned int numPositions;
	void* positions;
};

struct dbTriHit_s
{
	unsigned int numParts;
	dbTriHitPosArray_s posArray;
};

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

struct dbModelUnit_s
{
	dbModelUnitId_e id;
	void* data;
};

const struct dbModel_s
{
	dbModelUnit_s* units;
	unsigned int numUnits;
	unsigned int flags;
	unsigned int headerSize;
	dbModelRuntime_s* runtime;
};

dbModelUnit_s* meGetUnit(const dbModel_s* model, dbModelUnitId_e id)
{
	unsigned int i = 0;

	while (i < model->numUnits)
	{
		if (id == model->units[i].id)
			return (dbModelUnit_s*)model->units[i].data;
		++i;
	}
	return 0;
}

int meInitModel(dbModel_s* asset)
{
	dbModelUnit_s* Unit; 
	dbModelRuntime_s* ModelRuntime;

	ModelRuntime = (dbModelRuntime_s*)glMemAlloc(0x3Cu, 0);
	asset->runtime = ModelRuntime;
	ModelRuntime->mirror = (dbMirror_s*)meGetUnit(asset, dbModelUnitId_Mirrors);
	ModelRuntime->trihits = (dbTriHit_s*)meGetUnit(asset, dbModelUnitId_TriHits);
	ModelRuntime->shadowGeometry = (dbShadowGeometry_s*)meGetUnit(asset, dbModelUnitId_ShadowGeometry);
	ModelRuntime->moldGeometry = (dbMoldGeometry_s*)meGetUnit(asset, dbModelUnitId_MoldGeometry);
	ModelRuntime->texpalette = (dbModelTextureUnit_s*)meGetUnit(asset, dbModelUnitId_Textures);
	ModelRuntime->climbPoles = (const dbModelClimbPole_s*)meGetUnit(asset, dbModelUnitId_ClimbPoles);
	ModelRuntime->extent = (const dbExtent_s*)meGetUnit(asset, dbModelUnitId_Extents);
	ModelRuntime->joint = (const dbModelJoint_s*)meGetUnit(asset, dbModelUnitId_Joints);
	ModelRuntime->light = (const dbLight_s*)meGetUnit(asset, dbModelUnitId_Light);
	ModelRuntime->lightVolume = (const dbLightVolume_s*)meGetUnit(asset, dbModelUnitId_LightVolumes);
	ModelRuntime->locator = (const dbLocator_s*)meGetUnit(asset, dbModelUnitId_Locator);
	ModelRuntime->volume = (const dbVolume_s*)meGetUnit(asset, dbModelUnitId_Volumes);
	ModelRuntime->sceneGraph = (const dbScenegraph_s*)meGetUnit(asset, dbModelUnitId_Scenegraph);
	ModelRuntime->switchMask = (const dbModelSwitch_s*)meGetUnit(asset, dbModelUnitId_SwitchMaskNew);
	ModelRuntime->sg = 0;
	
	if (ModelRuntime->texpalette)
		//dbModelTextureUnitInit(ModelRuntime->texpalette, asset);
	
	if (ModelRuntime->sceneGraph)
		//dbScenegraphInit(ModelRuntime->sceneGraph, asset);
	
	if (ModelRuntime->lightVolume)
		//dbLightVolumeInit(ModelRuntime->lightVolume, asset);
	
	if (meGetUnit(asset, dbModelUnitId_GardenSetup))
	{
		Unit = meGetUnit(asset, dbModelUnitId_GardenSetup);
		//dbGardenSetupInit((dbGardenSetup_s*)Unit);
	}
	//videoMainUpdateUILoadingScreen();
	return 1;
}