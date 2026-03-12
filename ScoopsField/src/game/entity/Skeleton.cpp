#include "Skeleton.h"

#include "renderer/Renderer.h"

#include "game/Game.h"


static EntityAction* GetCurrentAction(SkeletonEntity* skeleton)
{
	return QueuePeek(skeleton->actions.actions);
}

static void OnDeath(SkeletonEntity* skeleton)
{
	SetRigidBodyVelocity(&skeleton->body, vec3(0), vec3(0));

	RemoveColliders(&skeleton->body);

	SDL_assert(game->numSkeletonsRemaining > 0);
	game->numSkeletonsRemaining--;
}

static void OnSkeletonHit(SkeletonEntity* skeleton, HitParams hit, Entity* by)
{
	int damage = (int)(hit.damage * hit.damageMultiplier);
	skeleton->health -= damage;

	PlaySound(&game->skeletonHitSounds[game->random.next() % 5], hit.position);

	if (skeleton->health <= 0)
	{
		EntityAction action;
		InitEntityDeathAction(&action);
		CancelAction(skeleton->actions, *skeleton);
		QueueAction(skeleton->actions, action, *skeleton);

		OnDeath(skeleton);
	}
	else
	{
		EntityAction action;
		InitEntityStaggerAction(&action, 1.0f);
		CancelAction(skeleton->actions, *skeleton);
		QueueAction(skeleton->actions, action, *skeleton);
	}
}

void InitSkeleton(SkeletonEntity* skeleton, const vec3& position, float rotation, int health)
{
	InitEntity(skeleton, ENTITY_TYPE_SKELETON);
	skeleton->hitCallback = (EntityHitCallback_t)OnSkeletonHit;

	skeleton->position = position;
	skeleton->rotation = rotation;

	skeleton->model = GetModel("entities/skeleton/skeleton.glb");
	InitAnimationState(&skeleton->anim, skeleton->model);

	InitAnimation(&skeleton->idleAnim, "idle", skeleton->model, 1, true, false);
	InitAnimation(&skeleton->runAnim, "run", skeleton->model, 1, true, false);

	InitRigidBody(&skeleton->body, RIGID_BODY_DYNAMIC, position, quat::FromAxisAngle(vec3::Up, rotation));
	AddCapsuleCollider(&skeleton->body, 0.3f, 2, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY, 1, false);
	SetRigidBodyAxisLock(&skeleton->body, RIGID_BODY_LOCK_ROTATION);
	skeleton->body.userPtr = skeleton;

	InitActionManager(skeleton->actions, skeleton->model);

	skeleton->health = health;
	skeleton->maxHealth = health;
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

static void UpdateAI(SkeletonEntity* skeleton)
{
	Player* target = &game->player;
	if (EveryInterval(1, hash(skeleton)))
	{
		skeleton->targetPosition = target->position;
		bool success = CalculateNavmeshPath(&game->mapNavmesh, skeleton->position, skeleton->targetPosition, skeleton->currentPath, skeleton->currentPathLength);
		SDL_assert(success);
	}

	vec3 toTarget = skeleton->targetPosition - skeleton->position;
	float distanceToTarget = toTarget.length();
	toTarget /= distanceToTarget;

	vec3 walkDir = toTarget;
	if (skeleton->currentPathLength > 0)
	{
		NavmeshNode* targetNode = &game->mapNavmesh.nodes[skeleton->currentPath[1]];
		walkDir = (targetNode->position - skeleton->position).normalized();
	}
	walkDir *= vec3(1, 0, 1);

	quat toTargetRotation = quat::LookAt(walkDir, vec3::Up);
	float toTargetAngle = toTargetRotation.getAngle() * sign(toTargetRotation.getAxis().y);
	skeleton->rotation = lerpAngle(skeleton->rotation, toTargetAngle, 5 * deltaTime);

	if (distanceToTarget > 0.5f)
	{
		float speed = 3;
		if (EntityAction* currentAction = GetCurrentAction(skeleton))
			speed *= currentAction->walkSpeed;

		vec3 velocity = walkDir * speed;
		SetRigidBodyVelocity(&skeleton->body, velocity, vec3(0));

		skeleton->moving = true;
	}
	else
	{
		SetRigidBodyVelocity(&skeleton->body, vec3(0), vec3(0));

		skeleton->moving = false;
	}

	if ((target->position - skeleton->position).length() < 1)
	{
		if (skeleton->actions.actions.size == 0)
		{
			EntityAction action;
			InitEntityAttackAction(&action, "attack1", 0);
			QueueAction(skeleton->actions, action, *skeleton);
		}
	}

	GetRigidBodyTransform(&skeleton->body, &skeleton->position, nullptr);
	//AddRigidBodyAcceleration(&skeleton->body, vec3(0, -10, 0));
}

void UpdateSkeleton(SkeletonEntity* skeleton)
{
	if (skeleton->health > 0)
	{
		//UpdateAI(skeleton);
	}

	UpdateActionManager(skeleton->actions, *skeleton);

	AnimationPlayback* moveAnimation = skeleton->moving ? &skeleton->runAnim : &skeleton->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(skeleton->model, &skeleton->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop, nullptr, nullptr);

	if (EntityAction* currentAction = GetCurrentAction(skeleton))
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;
		actionAnimation->timer += deltaTime * actionAnimation->speed;

		bool right = true;
		BlendAnimation(skeleton->model, &skeleton->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, 1, !currentAction->fullBody ? (AnimationChannelFilterCallback_t)UpperBodyAnimFilter : nullptr);
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

#if _DEBUG
	for (int i = 0; i < skeleton->currentPathLength; i++)
	{
		RenderModel(&game->renderer, &game->cube, nullptr, mat4::Translate(game->mapNavmesh.nodes[skeleton->currentPath[i]].position));
	}
#endif
}
