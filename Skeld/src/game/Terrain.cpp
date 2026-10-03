#include "Terrain.h"

#include "Core.h"

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"


void InitTerrain(Terrain* terrain, int tilex, int tilez, float* heights, vec2* normals, short* indices, SDL_GPUCommandBuffer* cmdBuffer)
{
	terrain->tilex = tilex;
	terrain->tilez = tilez;

	terrain->heights = (float*)SDL_malloc(TERRAIN_VERTICES * sizeof(float));
	SDL_memcpy(terrain->heights, heights, TERRAIN_VERTICES * sizeof(float));

	terrain->normals = (vec2*)SDL_malloc(TERRAIN_VERTICES * sizeof(vec2));
	SDL_memcpy(terrain->normals, normals, TERRAIN_VERTICES * sizeof(vec2));

	terrain->heightBuffer = CreateVertexBuffer(TERRAIN_VERTICES, &game->renderer.terrainLayout[0], 0);
	UpdateVertexBuffer(terrain->heightBuffer, 0, (const uint8_t*)heights, TERRAIN_VERTICES * sizeof(float), true, cmdBuffer);

	terrain->normalBuffer = CreateVertexBuffer(TERRAIN_VERTICES, &game->renderer.terrainLayout[1], 0);
	UpdateVertexBuffer(terrain->normalBuffer, 0, (const uint8_t*)normals, TERRAIN_VERTICES * sizeof(vec2), true, cmdBuffer);

	terrain->indexBuffer = CreateIndexBuffer(TERRAIN_TILES * 6, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	UpdateIndexBuffer(terrain->indexBuffer, 0, (const uint8_t*)indices, TERRAIN_TILES * 6 * sizeof(short), true, cmdBuffer);

	TextureInfo heightmapInfo = {};
	heightmapInfo.format = SDL_GPU_TEXTUREFORMAT_R32_FLOAT;
	heightmapInfo.width = TERRAIN_VERTICES_X;
	heightmapInfo.height = TERRAIN_VERTICES_X;
	heightmapInfo.depth = 1;
	heightmapInfo.numMips = 1;
	heightmapInfo.numLayers = 1;
	heightmapInfo.numFaces = 1;
	terrain->heightmap = CreateTexture(&heightmapInfo);
	SetTextureData(terrain->heightmap->handle, (const uint8_t*)terrain->heights, TERRAIN_VERTICES * sizeof(float), TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, 1, cmdBuffer);

	TextureInfo normalmapInfo = {};
	normalmapInfo.format = SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT;
	normalmapInfo.width = TERRAIN_VERTICES_X;
	normalmapInfo.height = TERRAIN_VERTICES_X;
	normalmapInfo.depth = 1;
	normalmapInfo.numMips = 1;
	normalmapInfo.numLayers = 1;
	normalmapInfo.numFaces = 1;
	terrain->normalmap = CreateTexture(&normalmapInfo);
	SetTextureData(terrain->normalmap->handle, (const uint8_t*)terrain->normals, TERRAIN_VERTICES * sizeof(vec2), TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, 1, cmdBuffer);

	VertexBufferLayout instanceLayout = {};
	instanceLayout.numAttributes = 4;
	instanceLayout.attributes[0].location = 5;
	instanceLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	instanceLayout.attributes[1].location = 6;
	instanceLayout.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	instanceLayout.attributes[2].location = 7;
	instanceLayout.attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	instanceLayout.attributes[3].location = 8;
	instanceLayout.attributes[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	instanceLayout.perInstance = true;
	terrain->treeInstances = CreateVertexBuffer(MAX_TREES, &instanceLayout, 0);
	terrain->treeInstanceTransfer = CreateTransferBuffer(MAX_TREES * sizeof(mat4), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD);

	float minHeight = FLT_MAX, maxHeight = -FLT_MAX;
	for (int i = 0; i < TERRAIN_VERTICES; i++)
	{
		minHeight = min(minHeight, heights[i]);
		maxHeight = max(maxHeight, heights[i]);
	}
	terrain->boundingBox = { vec3(tilex * TERRAIN_SIZE, minHeight, tilez * TERRAIN_SIZE),
		vec3(tilex * TERRAIN_SIZE + TERRAIN_SIZE, maxHeight, tilez * TERRAIN_SIZE + TERRAIN_SIZE) };

	InitRigidBody(&terrain->collider, RIGID_BODY_STATIC, vec3(tilex * TERRAIN_SIZE, 0.0f, tilez * TERRAIN_SIZE), quat::Identity, terrain);
	physx::PxHeightFieldSample* heightField = (physx::PxHeightFieldSample*)BumpAllocatorMalloc(&memory->transientAllocator, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	SDL_memset(heightField, 0, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	for (int z = 0; z < TERRAIN_VERTICES_X; z++)
	{
		for (int x = 0; x < TERRAIN_VERTICES_X; x++)
		{
			float height = heights[x + z * TERRAIN_VERTICES_X];
			int16_t flooredHeight = (int16_t)SDL_roundf(height * 10);
			heightField[z + x * TERRAIN_VERTICES_X].height = flooredHeight;
		}
	}
	AddHeightFieldCollider(&terrain->collider, TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, heightField, 0.1f, TERRAIN_TILE_SIZE, vec3(0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_TERRAIN, ENTITY_FILTER_DEFAULT);

	terrain->texture = GetTexture("textures/grass_diffuse.png");

	terrain->numTrees = 0;
}

void DestroyTerrain(Terrain* terrain)
{
	DestroyRigidBody(&terrain->collider);
	DestroyVertexBuffer(terrain->heightBuffer);
	DestroyVertexBuffer(terrain->normalBuffer);
	DestroyIndexBuffer(terrain->indexBuffer);
	SDL_free(terrain->heights);
	SDL_free(terrain->normals);
}

static void UpdateVertexBuffer(Terrain* terrain)
{
	UpdateVertexBuffer(terrain->heightBuffer, 0, (const uint8_t*)terrain->heights, TERRAIN_VERTICES * sizeof(float), true, cmdBuffer);

	SetTextureData(terrain->heightmap->handle, (const uint8_t*)terrain->heights, TERRAIN_VERTICES * sizeof(float), TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, 1, cmdBuffer);

	RemoveColliders(&terrain->collider);

	physx::PxHeightFieldSample* heightField = (physx::PxHeightFieldSample*)BumpAllocatorMalloc(&memory->transientAllocator, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	SDL_memset(heightField, 0, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	for (int z = 0; z < TERRAIN_VERTICES_X; z++)
	{
		for (int x = 0; x < TERRAIN_VERTICES_X; x++)
		{
			float height = terrain->heights[x + z * TERRAIN_VERTICES_X];
			int16_t flooredHeight = (int16_t)SDL_roundf(height * 10);
			heightField[z + x * TERRAIN_VERTICES_X].height = flooredHeight;
		}
	}
	AddHeightFieldCollider(&terrain->collider, TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, heightField, 0.1f, TERRAIN_TILE_SIZE, vec3(0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_TERRAIN, ENTITY_FILTER_DEFAULT);
}

static void UpdateNormalBuffer(Terrain* terrain)
{
	UpdateVertexBuffer(terrain->normalBuffer, 0, (const uint8_t*)terrain->normals, TERRAIN_VERTICES * sizeof(vec2), true, cmdBuffer);

	SetTextureData(terrain->normalmap->handle, (const uint8_t*)terrain->normals, TERRAIN_VERTICES * sizeof(vec2), TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, 1, cmdBuffer);
}

static void RecalculateNormals(Terrain* terrain, int x0, int z0, int x1, int z1)
{
	for (int z = z0; z <= z1; z++)
	{
		for (int x = x0; x <= x1; x++)
		{
			float leftHeight = terrain->heights[x - 1 + z * TERRAIN_VERTICES_X];
			float rightHeight = terrain->heights[x + 1 + z * TERRAIN_VERTICES_X];
			float frontHeight = terrain->heights[x + (z - 1) * TERRAIN_VERTICES_X];
			float backHeight = terrain->heights[x + (z + 1) * TERRAIN_VERTICES_X];

			float nx = (leftHeight - rightHeight) / TERRAIN_TILE_SIZE;
			float nz = (frontHeight - backHeight) / TERRAIN_TILE_SIZE;

			terrain->normals[x + z * TERRAIN_VERTICES_X] = vec2(nx, nz).normalized();
		}
	}
}

float Terrain::interpolateHeight(float localx, float localz)
{
	int gridx = (int)SDL_floorf(localx / TERRAIN_TILE_SIZE);
	int gridz = (int)SDL_floorf(localz / TERRAIN_TILE_SIZE);
	if (gridx < 0 || gridx >= TERRAIN_TILES_X || gridz < 0 || gridz >= TERRAIN_TILES_X)
		return 0;

	float dx = localx / TERRAIN_TILE_SIZE - gridx;
	float dz = localz / TERRAIN_TILE_SIZE - gridz;

	dz = 1 - dz;

	float h00 = heights[gridx + (gridz + 1) * TERRAIN_VERTICES_X];
	float h10 = heights[gridx + 1 + (gridz + 1) * TERRAIN_VERTICES_X];
	float h01 = heights[gridx + gridz * TERRAIN_VERTICES_X];
	float h11 = heights[gridx + 1 + gridz * TERRAIN_VERTICES_X];

	if (dx + dz < 1)
		return h00 + (h10 - h00) * dx + (h01 - h00) * dz;
	else
		return h11 + (h01 - h11) * (1 - dx) + (h10 - h11) * (1 - dz);
}

void Terrain::dig(int gridx, int gridz)
{
	int x0 = gridx;
	int x1 = gridx + 1;
	int z0 = gridz;
	int z1 = gridz + 1;

	float& v0 = heights[x0 + z0 * TERRAIN_VERTICES_X];
	float& v1 = heights[x1 + z0 * TERRAIN_VERTICES_X];
	float& v2 = heights[x0 + z1 * TERRAIN_VERTICES_X];
	float& v3 = heights[x1 + z1 * TERRAIN_VERTICES_X];

	int h0 = (int)SDL_roundf(v0 * 2);
	int h1 = (int)SDL_roundf(v1 * 2);
	int h2 = (int)SDL_roundf(v2 * 2);
	int h3 = (int)SDL_roundf(v3 * 2);

	int maxHeight = max(max(h0, h1), max(h2, h3));
	int newHeight = maxHeight - 1;

	float fh0 = min(newHeight, h0) / 2.0f;
	float fh1 = min(newHeight, h1) / 2.0f;
	float fh2 = min(newHeight, h2) / 2.0f;
	float fh3 = min(newHeight, h3) / 2.0f;

	Terrain* left = GetTerrainAtGridPosition(tilex - 1, tilez);
	Terrain* right = GetTerrainAtGridPosition(tilex + 1, tilez);
	Terrain* front = GetTerrainAtGridPosition(tilex, tilez - 1);
	Terrain* back = GetTerrainAtGridPosition(tilex, tilez + 1);
	Terrain* leftfront = GetTerrainAtGridPosition(tilex - 1, tilez - 1);
	Terrain* rightfront = GetTerrainAtGridPosition(tilex + 1, tilez - 1);
	Terrain* leftback = GetTerrainAtGridPosition(tilex - 1, tilez + 1);
	Terrain* rightback = GetTerrainAtGridPosition(tilex + 1, tilez + 1);

	if (x0 == 0 && (newHeight < h0 || newHeight < h2) && left)
	{
		left->heights[TERRAIN_TILES_X + z0 * TERRAIN_VERTICES_X] = fh0;
		left->heights[TERRAIN_TILES_X + z1 * TERRAIN_VERTICES_X] = fh2;
		UpdateVertexBuffer(left);
	}
	if (x1 == TERRAIN_TILES_X && (newHeight < h1 || newHeight < h3) && right)
	{
		right->heights[0 + z0 * TERRAIN_VERTICES_X] = fh1;
		right->heights[0 + z1 * TERRAIN_VERTICES_X] = fh3;
		UpdateVertexBuffer(right);
	}
	if (z0 == 0 && (newHeight < h0 || newHeight < h1) && front)
	{
		front->heights[x0 + TERRAIN_TILES_X * TERRAIN_VERTICES_X] = fh0;
		front->heights[x1 + TERRAIN_TILES_X * TERRAIN_VERTICES_X] = fh1;
		UpdateVertexBuffer(front);
	}
	if (z1 == TERRAIN_TILES_X && (newHeight < h2 || newHeight < h3) && back)
	{
		back->heights[x0 + 0 * TERRAIN_VERTICES_X] = fh2;
		back->heights[x1 + 0 * TERRAIN_VERTICES_X] = fh3;
		UpdateVertexBuffer(back);
	}
	if (x0 == 0 && z0 == 0 && newHeight < h0 && leftfront)
	{
		leftfront->heights[TERRAIN_TILES_X + TERRAIN_TILES_X * TERRAIN_VERTICES_X] = fh0;
		UpdateVertexBuffer(leftfront);
	}
	if (x1 == TERRAIN_TILES_X && z0 == 0 && newHeight < h1 && rightfront)
	{
		rightfront->heights[0 + TERRAIN_TILES_X * TERRAIN_VERTICES_X] = fh1;
		UpdateVertexBuffer(rightfront);
	}
	if (x0 == 0 && z1 == TERRAIN_TILES_X && newHeight < h2 && leftback)
	{
		leftback->heights[TERRAIN_TILES_X + 0 * TERRAIN_VERTICES_X] = fh2;
		UpdateVertexBuffer(leftback);
	}
	if (x1 == TERRAIN_TILES_X && z1 == TERRAIN_TILES_X && newHeight < h3 && rightback)
	{
		rightback->heights[0 + 0 * TERRAIN_VERTICES_X] = fh3;
		UpdateVertexBuffer(rightback);
	}

	v0 = fh0;
	v1 = fh1;
	v2 = fh2;
	v3 = fh3;

	UpdateVertexBuffer(this);

	int nx0 = x0 - 1;
	int nx1 = x1 + 1;
	int nz0 = z0 - 1;
	int nz1 = z1 + 1;

	if (nx0 > 0 && nz0 > 0 && nx1 < TERRAIN_TILES_X && nz1 < TERRAIN_TILES_X)
	{
		RecalculateNormals(this, nx0, nz0, nx1, nz1);
		UpdateNormalBuffer(this);
	}
}

float Terrain::getTileHeight(int gridx, int gridz)
{
	int x0 = gridx;
	int x1 = gridx + 1;
	int z0 = gridz;
	int z1 = gridz + 1;

	float& v0 = heights[x0 + z0 * TERRAIN_VERTICES_X];
	float& v1 = heights[x1 + z0 * TERRAIN_VERTICES_X];
	float& v2 = heights[x0 + z1 * TERRAIN_VERTICES_X];
	float& v3 = heights[x1 + z1 * TERRAIN_VERTICES_X];

	int h0 = (int)SDL_roundf(v0 * 2);
	int h1 = (int)SDL_roundf(v1 * 2);
	int h2 = (int)SDL_roundf(v2 * 2);
	int h3 = (int)SDL_roundf(v3 * 2);

	int maxHeight = max(max(h0, h1), max(h2, h3));

	return maxHeight / 2.0f;
}

void RenderTerrain(Terrain* terrain)
{
	RenderTerrain(&game->renderer, terrain);

	if (terrain->numTrees)
	{
		mat4* transforms = (mat4*)MapTransferBuffer(terrain->treeInstanceTransfer, true);
		for (int i = 0; i < terrain->numTrees; i++)
		{
			Tree* tree = terrain->trees[i];
			transforms[i] = tree->animatedTransform;
		}
		UnmapTransferBuffer(terrain->treeInstanceTransfer);

		UpdateVertexBuffer(terrain->treeInstances, 0, terrain->numTrees * sizeof(mat4), terrain->treeInstanceTransfer->buffer, true, cmdBuffer);

		RenderInstancedModel(&game->renderer, terrain->trees[0]->model, terrain->trees[0]->shader, terrain->trees[0]->shadowShader, nullptr, terrain->treeInstances, terrain->numTrees, mat4::Identity);
	}
}
