#pragma once

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Texture.h"

#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"


struct PositionKeyframe
{
	vec3 value;
	float time;
};

struct RotationKeyframe
{
	Quaternion value;
	float time;
};

struct ScalingKeyframe
{
	vec3 value;
	float time;
};

struct Mesh
{
	int vertexCount;
	int indexCount;

	VertexBuffer* positionBuffer;
	VertexBuffer* normalBuffer;
	VertexBuffer* weightsBuffer;
	IndexBuffer* indexBuffer;

	int skeletonID;
	int materialID;
};

struct Material
{
	uint32_t color;
	float metallicFactor;
	float roughnessFactor;
	vec3 emissiveColor;
	float emissiveStrength;

	Texture* diffuse;
	Texture* normal;
	Texture* roughness;
	Texture* metallic;
	Texture* emissive;
	Texture* height;
};

struct Node
{
	char name[64];
	int id;
	int armatureID;
	mat4 transform;

#define MAX_NODE_CHILDREN 16
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
#define MAX_BONES 64
	Bone bones[MAX_BONES];
	int numBones;
	mat4 inverseBindPose;
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

	int numPositions;
	int numRotations;
	int numScalings;

	PositionKeyframe* positions;
	RotationKeyframe* rotations;
	ScalingKeyframe* scalings;

#define MAX_ANIMATION_CHANNELS 64
	AnimationChannel channels[MAX_ANIMATION_CHANNELS];
	int numChannels;
};

struct Model
{
#define MAX_MESHES 16
	Mesh meshes[MAX_MESHES];
	int numMeshes;

#define MAX_MATERIALS 8
	Material materials[MAX_MATERIALS];
	int numMaterials;

#define MAX_SKELETONS 1
	Skeleton skeletons[MAX_SKELETONS];
	int numSkeletons;

#define MAX_NODES 64
	Node nodes[MAX_NODES];
	int numNodes;

#define MAX_ANIMATIONS 32
	Animation animations[MAX_ANIMATIONS];
	int numAnimations;
};


void InitMesh(Mesh* mesh, int vertexCount, const vec3* positions, const vec3* normals, int indexCount, const uint8_t* indices, SDL_GPUIndexElementSize indexElementSize, SDL_GPUCommandBuffer* cmdBuffer);

void LoadModel(Model* model, const char* path, SDL_GPUCommandBuffer* cmdBuffer);
