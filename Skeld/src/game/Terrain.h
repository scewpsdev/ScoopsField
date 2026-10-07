#pragma once

#include <SDL3/SDL.h>

#include "physics/RigidBody.h"

#include "math/Vector.h"
#include "math/Shape.h"


#define TERRAIN_TILES_X 16
#define TERRAIN_TILES (TERRAIN_TILES_X * TERRAIN_TILES_X)
#define TERRAIN_VERTICES_X (TERRAIN_TILES_X + 1)
#define TERRAIN_VERTICES (TERRAIN_VERTICES_X * TERRAIN_VERTICES_X)
#define TERRAIN_TILE_SIZE 2.0f
#define TERRAIN_SIZE (TERRAIN_TILES_X * TERRAIN_TILE_SIZE)


struct VertexBuffer;
struct IndexBuffer;
struct Texture;

enum TerrainMaterial : uint8_t
{
	TERRAIN_MATERIAL_NONE = 0,

	TERRAIN_MATERIAL_GRASS = 1 << 0,
	TERRAIN_MATERIAL_DIRT = 1 << 1,
};

struct Terrain
{
	int tilex, tilez;

	float* heights;
	vec2* normals;
	uint8_t* grassCoverage;
	uint8_t* materials;

	//VertexBuffer* heightBuffer;
	//VertexBuffer* normalBuffer;
	IndexBuffer* indexBuffer;
	Texture* heightmap;
	Texture* normalmap;
	Texture* grassCoverageMap;
	Texture* materialMap;

	AABB boundingBox;
	Sphere boundingSphere;

	RigidBody collider;

	Texture* grassTexture;
	Texture* dirtTexture;

#define MAX_TERRAIN_TREES 256
	Tree* trees[MAX_TERRAIN_TREES];
	int numTrees;

	bool visible;


	float interpolateHeight(float localx, float localz);
	void dig(int gridx, int gridz);
	float getTileHeight(int gridx, int gridz);
};


void InitTerrain(Terrain* terrain, int tilex, int tilez, float* heights, vec2* normals, short* indices, SDL_GPUCommandBuffer* cmdBuffer);

void RenderTerrain(Terrain* terrain);
