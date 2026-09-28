#include "Tree.h"

#include "Application.h"

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
	AddCapsuleCollider(&tree->body, 0.75f * scale, 10.0f, vec3(0, 5.0f, 0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY_HITBOX, 0, false);
}

void DestroyTree(Tree* tree)
{
	DestroyRigidBody(&tree->body);
}

bool HitTree(Tree* tree, HitParams* hit, Entity* by)
{
	SDL_Log("hit tree");
	return true;
}

void UpdateTree(Tree* tree)
{
}

void RenderTree(Tree* tree)
{
	RenderModel(&game->renderer, tree->model, tree->shader, tree->shadowShader, nullptr, ModelMatrix((Entity*)tree), true, 0);
}
