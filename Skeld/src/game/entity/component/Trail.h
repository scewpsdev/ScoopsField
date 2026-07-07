#pragma once

#include "game/entity/EntityBase.h"

#include "math/Vector.h"
#include "utils/List.h"


struct VertexBuffer;
struct TransferBuffer;
struct VertexBufferLayout;
struct Material;

struct TrailNode
{
	vec3 position;
	float distance;
	vec3 right;
	float width;
};

struct Trail : EntityBase
{
	float width;
	vec4 color;
	float emissive;
	Texture* texture;
	TextureSampler textureSampler;
	float scrollSpeed;
	bool billboard;

	bool fadeWidth;
	bool fadeAlpha;
	bool destroyOnCollapse;

#define MAX_TRAIL_NODES 16
	TrailNode nodes[MAX_TRAIL_NODES];
	int numNodes;

	VertexBuffer* vertexBuffer;
	TransferBuffer* transferBuffer;

	float lastNodeUpdate;

	AABB boundingBox;
	Sphere boundingSphere;
};


void InitTrailVertexLayout(VertexBufferLayout* layout);

void InitTrail(Trail* trail, vec3 position, bool additive, int numNodes = MAX_TRAIL_NODES);
void DestroyTrail(Trail* trail);

void UpdateTrail(Trail* trail);
void BendTrailEnd(Trail* trail, vec3 position, float range);

void RenderTrail(Trail* trail);
