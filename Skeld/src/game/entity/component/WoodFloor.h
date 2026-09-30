#pragma once

#include "game/entity/EntityBase.h"

#include "model/Model.h"

#include "physics/RigidBody.h"


struct WoodFloor : EntityBase
{
	RigidBody collider;
};


void InitWoodFloor(WoodFloor* floor, vec3 position);
void DestroyWoodFloor(WoodFloor* floor);

void UpdateWoodFloor(WoodFloor* floor);
void RenderWoodFloor(WoodFloor* floor);
