#include "GrassField.h"

#include "game/Terrain.h"


void InitGrassField(GrassField* grass, Terrain* terrain)
{
	InitEntity((Entity*)grass, ENTITY_TYPE_GRASS_FIELD);

	grass->position = vec3(terrain->tilex * TERRAIN_SIZE, 0, terrain->tilez * TERRAIN_SIZE);
	grass->terrain = terrain;

	grass->model = GetModel("models/grass_blade.glb");
	grass->material = {};
	grass->material.textures[0] = terrain->heightmap;
	grass->material.samplers[0] = TEXTURE_SAMPLER_LINEAR_CLAMPED;
	grass->material.vertexSampler[0] = true;
	grass->material.numTextures++;
}

void DestroyGrassField(GrassField* grass)
{
}

void UpdateGrassFieldData(GrassField* grass)
{
}

void UpdateGrassField(GrassField* grass)
{
}

static float DistanceToTerrain(Terrain* terrain, vec3 position)
{
	float x0 = terrain->tilex * TERRAIN_SIZE;
	float x1 = terrain->tilex * TERRAIN_SIZE + TERRAIN_SIZE;
	float z0 = terrain->tilez * TERRAIN_SIZE;
	float z1 = terrain->tilez * TERRAIN_SIZE + TERRAIN_SIZE;

	if (position.x >= x0 && position.x <= x1 &&
		position.z >= z0 && position.z <= z1)
		return 0;

	float distx = position.x < x0 ? x0 - position.x : position.x > x1 ? position.x - x1 : 0;
	float distz = position.z < z0 ? z0 - position.z : position.z > z1 ? position.z - z1 : 0;

	return max(distx, distz);
}

void RenderGrassField(GrassField* grass)
{
	float distance = DistanceToTerrain(grass->terrain, game->cameraPosition);
	float lodDistance = 0.5f * TERRAIN_SIZE;
	int lod = (int)log2f(max(SDL_ceilf(distance / lodDistance), 1.0f));

	int maxLod = 1;
	if (lod <= maxLod)
	{
		int numGrassBlades = TERRAIN_TILES * MAX_GRASS_BLADES / ipow(4, lod);
		grass->material.vertexShaderData[0] = (float)lod;
		//RenderInstancedModel(&game->renderer, grass->model, game->grassShader, game->grassShadowShader, &grass->material, game->grassInstances, numGrassBlades, ModelMatrix((Entity*)grass));

		Mesh* mesh = &grass->model->meshes[0];
		VertexBuffer* buffers[2] = {mesh->positionBuffer, game->grassInstances};
		mat4 transform = ModelMatrix((Entity*)grass);
		RenderMesh(&game->renderer,
			buffers, 2,
			nullptr,
			3, numGrassBlades,
			{}, {},
			nullptr, 0, vec4(0), 0,
			grass->material.textures, grass->material.samplers, grass->material.vertexSampler, grass->material.numTextures,
			game->grassShader, game->grassShadowShader,
			transform, 0);
	}
}
