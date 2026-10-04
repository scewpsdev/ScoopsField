#pragma once

#include "game/entity/EntityBase.h"
#include "model/Model.h"

#include "math/Vector.h"

#include <SDL3/SDL.h>


#define MAX_GRASS_BLADES 128 // max grass blades per tile
#define MAX_GRASS_LOD 3


struct Terrain;

struct GrassField : EntityBase
{
	Terrain* terrain;

	Model* lodModel;
	Model* lodModel2;
	Material material;
};


void InitGrassField(GrassField* grass, Terrain* terrain);
void DestroyGrassField(GrassField* grass);

void UpdateGrassFieldData(GrassField* grass);

void UpdateGrassField(GrassField* grass);
void RenderGrassField(GrassField* grass);
