#include "WoodFloor.h"


void InitWoodFloor(WoodFloor* floor, vec3 position)
{
	InitEntity((Entity*)floor, ENTITY_TYPE_WOOD_FLOOR);
	floor->position = position;

	InitRigidBody(&floor->collider, RIGID_BODY_STATIC, position, quat::Identity, floor);
	AddBoxCollider(&floor->collider, vec3(TERRAIN_TILE_SIZE, TERRAIN_TILE_SIZE * 0.05f, TERRAIN_TILE_SIZE), vec3(0.5f * TERRAIN_TILE_SIZE, 0, 0.5f * TERRAIN_TILE_SIZE), quat::Identity, ENTITY_FILTER_DEFAULT, ENTITY_FILTER_DEFAULT, false);
}

void DestroyWoodFloor(WoodFloor* floor)
{
	DestroyRigidBody(&floor->collider);
}

void UpdateWoodFloor(WoodFloor* floor)
{
}

void RenderWoodFloor(WoodFloor* floor)
{
	RenderModel(&game->renderer, &game->cube, mat4::Translate(floor->position) * mat4::Scale(TERRAIN_TILE_SIZE, TERRAIN_TILE_SIZE * 0.05f, TERRAIN_TILE_SIZE) * mat4::Translate(0.5f, 0.5f, 0.5f));
}
