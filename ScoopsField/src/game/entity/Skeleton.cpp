#include "Skeleton.h"

#include "renderer/Renderer.h"

#include "game/Game.h"


static EntityAction* GetCurrentAction(SkeletonEntity* skeleton)
{
	return QueuePeek(skeleton->actions.actions);
}

static void OnDeath(SkeletonEntity* skeleton)
{
	RemoveColliders(&skeleton->body);

	SDL_assert(game->numSkeletonsRemaining > 0);
	game->numSkeletonsRemaining--;
}

static void OnSkeletonHit(SkeletonEntity* skeleton, HitParams hit, Entity* by)
{
	int damage = (int)(hit.damage * hit.damageMultiplier);
	skeleton->health -= damage;
	if (skeleton->health <= 0)
	{
		EntityAction action;
		InitEntityDeathAction(&action);
		QueueAction(skeleton->actions, action, *skeleton);

		OnDeath(skeleton);
	}
	else
	{
		EntityAction action;
		InitEntityStaggerAction(&action, 1.0f);
		QueueAction(skeleton->actions, action, *skeleton);
	}
}

void InitSkeleton(SkeletonEntity* skeleton, const vec3& position, float rotation)
{
	InitEntity(skeleton, ENTITY_TYPE_SKELETON);
	skeleton->hitCallback = (EntityHitCallback_t)OnSkeletonHit;

	skeleton->position = position;
	skeleton->rotation = rotation;

	skeleton->model = GetModel("entities/skeleton/skeleton.glb");
	InitAnimationState(&skeleton->anim, skeleton->model);

	InitAnimation(&skeleton->idleAnim, "idle", skeleton->model, 1, true);
	InitAnimation(&skeleton->runAnim, "run", skeleton->model, 1, true);

	InitRigidBody(&skeleton->body, RIGID_BODY_DYNAMIC, position, quat::FromAxisAngle(vec3::Up, rotation));
	AddCapsuleCollider(&skeleton->body, 0.3f, 2, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY, 1, false);
	SetRigidBodyAxisLock(&skeleton->body, RIGID_BODY_LOCK_ROTATION);
	skeleton->body.userPtr = skeleton;

	InitActionManager(skeleton->actions, skeleton->model);

	skeleton->health = 100;
	skeleton->maxHealth = 100;
}

void DestroySkeleton(SkeletonEntity* skeleton)
{
	DestroyRigidBody(&skeleton->body);
	*skeleton = {};
}

static bool UpperBodyAnimFilter(Node* node, void* userPtr)
{
	bool isUpperBody =
		StartsWith(node->name, "Spine") ||
		StartsWith(node->name, "Chest") ||
		StartsWith(node->name, "Neck") ||
		StartsWith(node->name, "Head") ||
		StartsWith(node->name, "Jaw") ||
		StartsWith(node->name, "Shoulder") ||
		StartsWith(node->name, "Arm") ||
		StartsWith(node->name, "Hand") ||
		StartsWith(node->name, "Thumb") ||
		StartsWith(node->name, "Index") ||
		StartsWith(node->name, "Middle") ||
		StartsWith(node->name, "Ring") ||
		StartsWith(node->name, "Pinky") ||
		StartsWith(node->name, "Weapon");
	return isUpperBody;
}

void UpdateSkeleton(SkeletonEntity* skeleton)
{
	if (skeleton->actions.actions.size == 0)
	{
		EntityAction action;
		InitEntityAttackAction(&action, "attack1", 0);
		QueueAction(skeleton->actions, action, *skeleton);
	}

	UpdateActionManager(skeleton->actions, *skeleton);

	AnimationPlayback* moveAnimation = skeleton->moving ? &skeleton->runAnim : &skeleton->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(skeleton->model, &skeleton->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop);

	if (EntityAction* currentAction = GetCurrentAction(skeleton))
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;
		actionAnimation->timer += deltaTime * actionAnimation->speed;

		bool right = true;
		BlendAnimation(skeleton->model, &skeleton->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, 1, (AnimationChannelFilterCallback_t)UpperBodyAnimFilter);
	}

	ApplyAnimationToSkeleton(skeleton->model, &skeleton->anim);

	if (skeleton->health > 0)
	{
		GetRigidBodyTransform(&skeleton->body, &skeleton->position, nullptr);
	}

	mat4 transform = mat4::Translate(skeleton->position) * mat4::Rotate(vec3::Up, skeleton->rotation);
	if (skeleton->health <= 0 && !FrustumCulling(skeleton->model->boundingSphere, transform, game->frustumPlanes))
	{
		skeleton->removed = true;
	}
}

void RenderSkeleton(SkeletonEntity* skeleton)
{
	mat4 transform = mat4::Translate(skeleton->position) * mat4::Rotate(vec3::Up, skeleton->rotation);

	RenderModel(&game->renderer, skeleton->model, &skeleton->anim, transform);
}
