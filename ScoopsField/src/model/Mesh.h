#pragma once

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"

#include "math/Vector.h"
#include "math/Quaternion.h"


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
	IndexBuffer* indexBuffer;
};


void InitMesh(Mesh* mesh, int vertexCount, const vec3* positions, const vec3* normals, int indexCount, const uint8_t* indices, SDL_GPUIndexElementSize indexElementSize, SDL_GPUCommandBuffer* cmdBuffer);

Mesh* LoadMesh(const char* path, SDL_GPUCommandBuffer* cmdBuffer);
