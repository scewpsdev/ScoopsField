#pragma once

#include "game/entity/EntityBase.h"
#include "model/Model.h"

#include "math/Vector.h"

#include <SDL3/SDL.h>


struct Terrain;

struct GrassBladeData
{
	vec4 position;
};

struct GrassField : EntityBase
{
	Terrain* terrain;

	GrassBladeData* bladeData;
	int numBlades;

	VertexBuffer* instanceBuffer;
	Material material;
};


void InitGrassField(GrassField* grass, Terrain* terrain);
void DestroyGrassField(GrassField* grass);

void UpdateGrassFieldData(GrassField* grass);

void UpdateGrassField(GrassField* grass);
void RenderGrassField(GrassField* grass);
