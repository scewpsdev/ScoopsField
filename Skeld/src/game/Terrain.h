#pragma once

#include <SDL3/SDL.h>

#include "physics/RigidBody.h"

#include "math/Vector.h"
#include "math/Shape.h"


#define TERRAIN_TILES_X 32
#define TERRAIN_TILES (TERRAIN_TILES_X * TERRAIN_TILES_X)
#define TERRAIN_VERTICES_X (TERRAIN_TILES_X + 1)
#define TERRAIN_VERTICES (TERRAIN_VERTICES_X * TERRAIN_VERTICES_X)
#define TERRAIN_TILE_SIZE 2.0f
#define TERRAIN_SIZE (TERRAIN_TILES_X * TERRAIN_TILE_SIZE)


struct VertexBuffer;
struct IndexBuffer;
struct Texture;

struct Terrain
{
	int tilex, tilez;

	float* heights;
	vec3* normals;

	VertexBuffer* heightBuffer;
	VertexBuffer* normalBuffer;
	IndexBuffer* indexBuffer;
	Texture* heightmap;

	AABB boundingBox;
	Sphere boundingSphere;

	RigidBody collider;

	Texture* texture;

#define MAX_TREES 256
	Tree* trees[MAX_TREES];
	int numTrees;
	VertexBuffer* treeInstances;
	TransferBuffer* treeInstanceTransfer;


	float interpolateHeight(float localx, float localz);
	void dig(int gridx, int gridz);
	float getTileHeight(int gridx, int gridz);
};


void InitTerrain(Terrain* terrain, int tilex, int tilez, float* heights, vec3* normals, short* indices, SDL_GPUCommandBuffer* cmdBuffer);

void RenderTerrain(Terrain* terrain);
