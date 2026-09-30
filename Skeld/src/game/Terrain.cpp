#include "Terrain.h"

#include "Core.h"

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"


void InitTerrain(Terrain* terrain, int tilex, int tilez, vec3* heights, vec3* normals, short* indices, SDL_GPUCommandBuffer* cmdBuffer)
{
	terrain->tilex = tilex;
	terrain->tilez = tilez;

	terrain->vertices = (vec3*)SDL_malloc(TERRAIN_VERTICES * sizeof(vec3));
	SDL_memcpy(terrain->vertices, heights, TERRAIN_VERTICES * sizeof(vec3));

	terrain->normals = (vec3*)SDL_malloc(TERRAIN_VERTICES * sizeof(vec3));
	SDL_memcpy(terrain->normals, normals, TERRAIN_VERTICES * sizeof(vec3));

	terrain->heightBuffer = CreateVertexBuffer(TERRAIN_VERTICES, &game->renderer.terrainLayout[0], 0);
	UpdateVertexBuffer(terrain->heightBuffer, 0, (const uint8_t*)heights, TERRAIN_VERTICES * sizeof(vec3), true, cmdBuffer);

	terrain->normalBuffer = CreateVertexBuffer(TERRAIN_VERTICES, &game->renderer.terrainLayout[1], 0);
	UpdateVertexBuffer(terrain->normalBuffer, 0, (const uint8_t*)normals, TERRAIN_VERTICES * sizeof(vec3), true, cmdBuffer);

	terrain->indexBuffer = CreateIndexBuffer(TERRAIN_TILES * 6, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	UpdateIndexBuffer(terrain->indexBuffer, 0, (const uint8_t*)indices, TERRAIN_TILES * 6 * sizeof(short), true, cmdBuffer);

	terrain->boundingBox.min = vec3(FLT_MAX);
	terrain->boundingBox.max = vec3(-FLT_MAX);
	for (int i = 0; i < TERRAIN_VERTICES; i++)
	{
		terrain->boundingBox.min = min(terrain->boundingBox.min, heights[i]);
		terrain->boundingBox.max = max(terrain->boundingBox.max, heights[i]);
	}
	terrain->boundingSphere.center = 0.5f * (terrain->boundingBox.min + terrain->boundingBox.max);
	for (int i = 0; i < TERRAIN_VERTICES; i++)
	{
		terrain->boundingSphere.radius = max(terrain->boundingSphere.radius, (heights[i] - terrain->boundingSphere.center).length());
	}

	InitRigidBody(&terrain->collider, RIGID_BODY_STATIC, vec3(tilex * TERRAIN_SIZE, 0.0f, tilez * TERRAIN_SIZE), quat::Identity, terrain);
	physx::PxHeightFieldSample* heightField = (physx::PxHeightFieldSample*)BumpAllocatorMalloc(&memory->transientAllocator, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	SDL_memset(heightField, 0, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	for (int z = 0; z < TERRAIN_VERTICES_X; z++)
	{
		for (int x = 0; x < TERRAIN_VERTICES_X; x++)
		{
			float height = heights[x + z * TERRAIN_VERTICES_X].y;
			int16_t flooredHeight = (int16_t)SDL_roundf(height * 10);
			heightField[z + x * TERRAIN_VERTICES_X].height = flooredHeight;
		}
	}
	AddHeightFieldCollider(&terrain->collider, TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, heightField, 0.1f, TERRAIN_TILE_SIZE, vec3(0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_TERRAIN, ENTITY_FILTER_DEFAULT);

	terrain->texture = GetTexture("textures/grass_diffuse.png");
}

void DestroyTerrain(Terrain* terrain)
{
	DestroyRigidBody(&terrain->collider);
	DestroyVertexBuffer(terrain->heightBuffer);
	DestroyVertexBuffer(terrain->normalBuffer);
	DestroyIndexBuffer(terrain->indexBuffer);
	SDL_free(terrain->vertices);
	SDL_free(terrain->normals);
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

	float h00 = vertices[gridx + (gridz + 1) * TERRAIN_VERTICES_X].y;
	float h10 = vertices[gridx + 1 + (gridz + 1) * TERRAIN_VERTICES_X].y;
	float h01 = vertices[gridx + gridz * TERRAIN_VERTICES_X].y;
	float h11 = vertices[gridx + 1 + gridz * TERRAIN_VERTICES_X].y;

	if (dx + dz < 1)
		return h00 + (h10 - h00) * dx + (h01 - h00) * dz;
	else
		return h11 + (h01 - h11) * (1 - dx) + (h10 - h11) * (1 - dz);
}

static void UpdateTerrainHeightmap(Terrain* terrain)
{
	UpdateVertexBuffer(terrain->heightBuffer, 0, (const uint8_t*)terrain->vertices, TERRAIN_VERTICES * sizeof(vec3), true, cmdBuffer);

	RemoveColliders(&terrain->collider);

	physx::PxHeightFieldSample* heightField = (physx::PxHeightFieldSample*)BumpAllocatorMalloc(&memory->transientAllocator, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	SDL_memset(heightField, 0, TERRAIN_VERTICES_X * TERRAIN_VERTICES_X * sizeof(physx::PxHeightFieldSample));
	for (int z = 0; z < TERRAIN_VERTICES_X; z++)
	{
		for (int x = 0; x < TERRAIN_VERTICES_X; x++)
		{
			float height = terrain->vertices[x + z * TERRAIN_VERTICES_X].y;
			int16_t flooredHeight = (int16_t)SDL_roundf(height * 10);
			heightField[z + x * TERRAIN_VERTICES_X].height = flooredHeight;
		}
	}
	AddHeightFieldCollider(&terrain->collider, TERRAIN_VERTICES_X, TERRAIN_VERTICES_X, heightField, 0.1f, TERRAIN_TILE_SIZE, vec3(0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_TERRAIN, ENTITY_FILTER_DEFAULT);
}

static void UpdateTerrainNormals(Terrain* terrain)
{
	UpdateVertexBuffer(terrain->normalBuffer, 0, (const uint8_t*)terrain->normals, TERRAIN_VERTICES * sizeof(vec3), true, cmdBuffer);
}

static void RecalculateNormals(Terrain* terrain, int x0, int z0, int x1, int z1)
{
	for (int z = z0; z <= z1; z++)
	{
		for (int x = x0; x <= x1; x++)
		{
			float leftHeight = terrain->vertices[x - 1 + z * TERRAIN_VERTICES_X].y;
			float rightHeight = terrain->vertices[x + 1 + z * TERRAIN_VERTICES_X].y;
			float frontHeight = terrain->vertices[x + (z - 1) * TERRAIN_VERTICES_X].y;
			float backHeight = terrain->vertices[x + (z + 1) * TERRAIN_VERTICES_X].y;

			float nx = leftHeight - rightHeight;
			float ny = TERRAIN_TILE_SIZE;
			float nz = frontHeight - backHeight;

			terrain->normals[x + z * TERRAIN_VERTICES_X] = vec3(nx, ny, nz).normalized();
		}
	}
}

void Terrain::dig(int gridx, int gridz)
{
	int x0 = gridx;
	int x1 = gridx + 1;
	int z0 = gridz;
	int z1 = gridz + 1;

	vec3& v0 = vertices[x0 + z0 * TERRAIN_VERTICES_X];
	vec3& v1 = vertices[x1 + z0 * TERRAIN_VERTICES_X];
	vec3& v2 = vertices[x0 + z1 * TERRAIN_VERTICES_X];
	vec3& v3 = vertices[x1 + z1 * TERRAIN_VERTICES_X];

	int h0 = (int)SDL_roundf(v0.y * 2);
	int h1 = (int)SDL_roundf(v1.y * 2);
	int h2 = (int)SDL_roundf(v2.y * 2);
	int h3 = (int)SDL_roundf(v3.y * 2);

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
		left->vertices[TERRAIN_TILES_X + z0 * TERRAIN_VERTICES_X].y = fh0;
		left->vertices[TERRAIN_TILES_X + z1 * TERRAIN_VERTICES_X].y = fh2;
		UpdateTerrainHeightmap(left);
	}
	if (x1 == TERRAIN_TILES_X && (newHeight < h1 || newHeight < h3) && right)
	{
		right->vertices[0 + z0 * TERRAIN_VERTICES_X].y = fh1;
		right->vertices[0 + z1 * TERRAIN_VERTICES_X].y = fh3;
		UpdateTerrainHeightmap(right);
	}
	if (z0 == 0 && (newHeight < h0 || newHeight < h1) && front)
	{
		front->vertices[x0 + TERRAIN_TILES_X * TERRAIN_VERTICES_X].y = fh0;
		front->vertices[x1 + TERRAIN_TILES_X * TERRAIN_VERTICES_X].y = fh1;
		UpdateTerrainHeightmap(front);
	}
	if (z1 == TERRAIN_TILES_X && (newHeight < h2 || newHeight < h3) && back)
	{
		back->vertices[x0 + 0 * TERRAIN_VERTICES_X].y = fh2;
		back->vertices[x1 + 0 * TERRAIN_VERTICES_X].y = fh3;
		UpdateTerrainHeightmap(back);
	}
	if (x0 == 0 && z0 == 0 && newHeight < h0 && leftfront)
	{
		leftfront->vertices[TERRAIN_TILES_X + TERRAIN_TILES_X * TERRAIN_VERTICES_X].y = fh0;
		UpdateTerrainHeightmap(leftfront);
	}
	if (x1 == TERRAIN_TILES_X && z0 == 0 && newHeight < h1 && rightfront)
	{
		rightfront->vertices[0 + TERRAIN_TILES_X * TERRAIN_VERTICES_X].y = fh1;
		UpdateTerrainHeightmap(rightfront);
	}
	if (x0 == 0 && z1 == TERRAIN_TILES_X && newHeight < h2 && leftback)
	{
		leftback->vertices[TERRAIN_TILES_X + 0 * TERRAIN_VERTICES_X].y = fh2;
		UpdateTerrainHeightmap(leftback);
	}
	if (x1 == TERRAIN_TILES_X && z1 == TERRAIN_TILES_X && newHeight < h3 && rightback)
	{
		rightback->vertices[0 + 0 * TERRAIN_VERTICES_X].y = fh3;
		UpdateTerrainHeightmap(rightback);
	}

	v0.y = fh0;
	v1.y = fh1;
	v2.y = fh2;
	v3.y = fh3;

	UpdateTerrainHeightmap(this);

	int nx0 = x0 - 1;
	int nx1 = x1 + 1;
	int nz0 = z0 - 1;
	int nz1 = z1 + 1;

	if (nx0 > 0 && nz0 > 0 && nx1 < TERRAIN_TILES_X && nz1 < TERRAIN_TILES_X)
	{
		RecalculateNormals(this, nx0, nz0, nx1, nz1);
		UpdateTerrainNormals(this);
	}
}

float Terrain::getTileHeight(int gridx, int gridz)
{
	int x0 = gridx;
	int x1 = gridx + 1;
	int z0 = gridz;
	int z1 = gridz + 1;

	vec3& v0 = vertices[x0 + z0 * TERRAIN_VERTICES_X];
	vec3& v1 = vertices[x1 + z0 * TERRAIN_VERTICES_X];
	vec3& v2 = vertices[x0 + z1 * TERRAIN_VERTICES_X];
	vec3& v3 = vertices[x1 + z1 * TERRAIN_VERTICES_X];

	int h0 = (int)SDL_roundf(v0.y * 2);
	int h1 = (int)SDL_roundf(v1.y * 2);
	int h2 = (int)SDL_roundf(v2.y * 2);
	int h3 = (int)SDL_roundf(v3.y * 2);

	int maxHeight = max(max(h0, h1), max(h2, h3));

	return maxHeight / 2.0f;
}
