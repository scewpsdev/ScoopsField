#pragma once

#include "math/Vector.h"
#include "utils/List.h"


struct VertexBuffer;
struct TransferBuffer;
struct VertexBufferLayout;

struct TrailNode
{
	vec3 position;
	float distance;
};

struct Trail
{
	float width;

#define MAX_TRAIL_NODES 16
	TrailNode nodes[MAX_TRAIL_NODES];
	int numNodes;

	VertexBuffer* vertexBuffer;
	TransferBuffer* transferBuffer;

	float lastNodeUpdate;
};


void InitTrailVertexLayout(VertexBufferLayout* layout);

void InitTrail(Trail* trail, float width, vec3 startPosition, int numNodes = MAX_TRAIL_NODES);
void DestroyTrail(Trail* trail);

void UpdateTrail(Trail* trail, vec3 position, SDL_GPUCommandBuffer* cmdBuffer);
void RenderTrail(Trail* trail);
