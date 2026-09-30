#include "Tree.h"

#include "Core.h"

#include "game/player/action/Action.h"
#include "game/particle/ParticleSystem.h"

#include "renderer/Renderer.h"


void InitTree(Tree* tree, vec3 position, float rotation, float scale)
{
	InitEntity((Entity*)tree, ENTITY_TYPE_TREE);
	tree->position = position;
	tree->rotation = quat::FromAxisAngle(vec3::Up, rotation);
	tree->scale = vec3(scale);
	tree->model = GetModel("entities/object/tree/tree.glb");
	tree->shader = game->treeShader;
	tree->shadowShader = game->treeShadowShader;

	InitRigidBody(&tree->body, RIGID_BODY_STATIC, tree->position, quat::Identity, tree);
	AddCapsuleCollider(&tree->body, 0.2f * scale, 5.0f, vec3(0, 2.5f, 0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY_HITBOX, 0, false);
}

void DestroyTree(Tree* tree)
{
	DestroyRigidBody(&tree->body);
}

bool HitTree(Tree* tree, HitParams* hit, Entity* by)
{
	hit->wasBlocked = true;

	PlaySound(&game->hitWoodSound, 0, 1);
	// hit reaction swing
	// particles

	return true;
}

void UpdateTree(Tree* tree)
{
}

void RenderTree(Tree* tree)
{
	RenderModel(&game->renderer, tree->model, tree->shader, tree->shadowShader, nullptr, ModelMatrix((Entity*)tree), true, 0);
}
