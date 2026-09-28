#pragma once

#include "game/entity/EntityBase.h"

#include "model/Model.h"

#include "physics/RigidBody.h"


struct Tree : EntityBase
{
	RigidBody body;
};


void InitTree(Tree* tree, vec3 position, float rotation, float scale);
void DestroyTree(Tree* tree);

bool HitTree(Tree* tree, HitParams* hit, Entity* by);

void UpdateTree(Tree* tree);
void RenderTree(Tree* tree);
