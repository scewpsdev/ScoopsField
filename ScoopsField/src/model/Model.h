#pragma once

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Texture.h"

#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"
#include "math/Shape.h"

#include "utils/HashMap.h"


struct Mesh
{
	int vertexCount;
	int indexCount;

	VertexBuffer* positionBuffer;
	VertexBuffer* normalBuffer;
	VertexBuffer* weightsBuffer;
	VertexBuffer* texcoordBuffer;
	IndexBuffer* indexBuffer;

	int skeletonID;
	int materialID;

	AABB boundingBox;
	Sphere boundingSphere;

	vec3* cachedPositions;
	uint8_t* cachedIndices;
};

struct Material
{
	union {
		struct {
			vec4 data0;
			vec4 data1;
			vec4 data2;
			vec4 data3;

#define MAX_MATERIAL_TEXTURES 6
			Texture* textures[MAX_MATERIAL_TEXTURES];
			TextureSampler samplers[MAX_MATERIAL_TEXTURES];
			int numTextures;
		};
		struct {
			vec4 textureStates;
			vec4 color;
			vec3 emissiveColor;
			float emissiveStrength;
			float roughnessFactor;
			float metallicFactor;
			vec2 padding;

			Texture* diffuse;
			Texture* roughness;
			Texture* metallic;
			Texture* normal;
			Texture* emissive;
			Texture* height;
			TextureSampler samplers[MAX_MATERIAL_TEXTURES];
			int numTextures;
		};
	};
};

struct Node
{
	char name[64];
	int id;
	int armatureID;
	int parent;
	mat4 transform;

#define MAX_NODE_CHILDREN 64
	int children[MAX_NODE_CHILDREN];
	int numChildren;

#define MAX_NODE_MESHES 8
	int meshes[MAX_NODE_MESHES];
	int numMeshes;
};

struct Bone
{
	char name[32];
	mat4 offsetMatrix;
	int nodeID;
};

struct Skeleton
{
	Bone* bones;
	int numBones;
	mat4 inverseBindPose;
};

struct PositionKeyframe
{
	vec3 value;
	float time;
};

struct RotationKeyframe
{
	quat value;
	float time;
};

struct ScalingKeyframe
{
	vec3 value;
	float time;
};

struct AnimationChannel
{
	char name[32];

	int positionsOffset;
	int positionsCount;
	int rotationsOffset;
	int rotationsCount;
	int scalingsOffset;
	int scalingsCount;
};

struct Animation
{
	char name[32];
	float duration;

	// TODO put these inside Model as big memory blocks
	int numPositions;
	int numRotations;
	int numScalings;

	PositionKeyframe* positions;
	RotationKeyframe* rotations;
	ScalingKeyframe* scalings;

	AnimationChannel* channels;
	int numChannels;

#define MAX_ANIMATION_CHANNELS 128
	HashMap<uint32_t, int, MAX_ANIMATION_CHANNELS> channelNameMap;
};

struct Model
{
#define MAX_MESHES 64
	Mesh meshes[MAX_MESHES];
	int numMeshes;

#define MAX_MATERIALS 16
	Material materials[MAX_MATERIALS];
	int numMaterials;

#define MAX_SKELETONS 8
	Skeleton skeletons[MAX_SKELETONS];
	int numSkeletons;

#define MAX_NODES 128
	Node nodes[MAX_NODES];
	int numNodes;

#define MAX_ANIMATIONS 32
	Animation animations[MAX_ANIMATIONS];
	int numAnimations;

	AABB boundingBox;
	Sphere boundingSphere;
};


bool LoadModel(Model* model, const char* path, bool cacheMeshes, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyModel(Model* model);

Node* GetNodeByName(Model* model, const char* name);
Animation* GetAnimationByName(Model* model, const char* name);
