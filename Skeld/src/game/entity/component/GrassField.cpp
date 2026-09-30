#include "GrassField.h"

#include "game/Terrain.h"


#define MAX_GRASS_BLADES (TERRAIN_TILES * 100)


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

	VertexBufferLayout layout = {};
	layout.numAttributes = 1;
	layout.attributes[0].location = 5;
	layout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	layout.perInstance = true;
	grass->instanceBuffer = CreateVertexBuffer(MAX_GRASS_BLADES, &layout, 0);

	grass->bladeData = (GrassBladeData*)SDL_malloc(MAX_GRASS_BLADES * sizeof(GrassBladeData));
	grass->numBlades = 0;
}

void DestroyGrassField(GrassField* grass)
{
	SDL_free(grass->bladeData);
	DestroyVertexBuffer(grass->instanceBuffer);
}

void UpdateGrassFieldData(GrassField* grass)
{
	if (grass->numBlades)
	{
		UpdateVertexBuffer(grass->instanceBuffer, 0, (const uint8_t*)grass->bladeData, grass->numBlades * sizeof(GrassBladeData), false, cmdBuffer);
	}
}

void UpdateGrassField(GrassField* grass)
{
}

void RenderGrassField(GrassField* grass)
{
	if (grass->numBlades)
	{
		RenderInstancedModel(&game->renderer, grass->model, game->grassShader, game->grassShadowShader, &grass->material, grass->instanceBuffer, grass->numBlades, ModelMatrix((Entity*)grass));
	}
}
