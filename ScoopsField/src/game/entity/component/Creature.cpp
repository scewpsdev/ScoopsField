#include "Creature.h"

#include "Resource.h"

#include "game/entity/Entity.h"

#include "utils/StringUtils.h"


void InitCreature(Creature* creature, Entity* entity, const char* model, float lookDirection, int health)
{
	creature->lookDirection = lookDirection;

	creature->model = GetModel(model);
	InitAnimationState(&creature->anim, creature->model);

	InitAnimation(&creature->idleAnim, "idle", creature->model, 1, true, false);
	InitAnimation(&creature->runAnim, "run", creature->model, 1, true, false);

	InitRigidBody(&creature->body, RIGID_BODY_DYNAMIC, entity->position, quat::FromAxisAngle(vec3::Up, lookDirection));
	AddCapsuleCollider(&creature->body, 0.3f, 2, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY, ENTITY_FILTER_DEFAULT, false);
	SetRigidBodyAxisLock(&creature->body, RIGID_BODY_LOCK_ROTATION);
	creature->body.userPtr = entity;

	InitActionManager(creature->actions, creature->model);

	creature->health = health;
	creature->maxHealth = health;
}

void DestroyCreature(Creature* creature, Entity* entity)
{
	DestroyRigidBody(&creature->body);
}

static void OnDeath(Creature* creature)
{
	SetRigidBodyVelocity(&creature->body, vec3(0), vec3(0));

	RemoveColliders(&creature->body);

	SDL_assert(game->numSkeletonsRemaining > 0);
	game->numSkeletonsRemaining--;
}

bool HitCreature(Creature* creature, Entity* entity, HitParams* hit, Entity* by)
{
	int damage = (int)(hit->damage * hit->damageMultiplier);
	creature->health -= damage;

	PlaySound(&game->skeletonHitSounds[game->random.next() % 5], hit->position);

	if (creature->health <= 0)
	{
		EntityAction action;
		InitEntityDeathAction(&action);
		CancelAction(creature->actions, *entity);
		QueueAction(creature->actions, action, *entity);

		OnDeath(creature);
	}
	else
	{
		EntityAction action;
		InitEntityStaggerAction(&action, 1.0f);
		CancelAction(creature->actions, *entity);
		QueueAction(creature->actions, action, *entity);
	}

	return true;
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

static EntityAction* GetCurrentAction(Creature* creature)
{
	return QueuePeek(creature->actions.actions);
}

static void UpdateAI(Creature* creature, Entity* entity)
{
	Player* target = &game->player;
	if (EveryInterval(1, hash(creature)))
	{
		creature->targetPosition = target->position;
		bool success = CalculateNavmeshPath(&game->mapNavmesh, entity->position, creature->targetPosition, creature->currentPath, creature->currentPathLength);
		SDL_assert(success);
	}

	vec3 toTarget = creature->targetPosition - entity->position;
	float distanceToTarget = toTarget.length();
	toTarget /= distanceToTarget;

	vec3 walkDir = toTarget;
	if (creature->currentPathLength > 0)
	{
		NavmeshNode* targetNode = &game->mapNavmesh.nodes[creature->currentPath[1]];
		walkDir = (targetNode->position - entity->position).normalized();
	}
	walkDir *= vec3(1, 0, 1);

	quat toTargetRotation = quat::LookAt(walkDir, vec3::Up);
	float toTargetAngle = toTargetRotation.getAngle() * sign(toTargetRotation.getAxis().y);
	creature->lookDirection = lerpAngle(creature->lookDirection, toTargetAngle, 5 * deltaTime);

	if (distanceToTarget > 0.5f)
	{
		float speed = 3;
		if (EntityAction* currentAction = GetCurrentAction(creature))
			speed *= currentAction->walkSpeed;

		vec3 velocity = walkDir * speed;
		SetRigidBodyVelocity(&creature->body, velocity, vec3(0));

		creature->moving = true;
	}
	else
	{
		SetRigidBodyVelocity(&creature->body, vec3(0), vec3(0));

		creature->moving = false;
	}

	if ((target->position - entity->position).length() < 1)
	{
		if (creature->actions.actions.size == 0)
		{
			EntityAction action;
			InitEntityAttackAction(&action, "attack1", 0);
			QueueAction(creature->actions, action, *entity);
		}
	}

	//GetRigidBodyTransform(&creature->body, &entity->position, nullptr);
	//AddRigidBodyAcceleration(&skeleton->body, vec3(0, -10, 0));
}

void UpdateCreature(Creature* creature, Entity* entity)
{
	if (creature->health > 0)
	{
		//UpdateAI(skeleton);
	}

	UpdateActionManager(creature->actions, *entity);

	AnimationPlayback* moveAnimation = creature->moving ? &creature->runAnim : &creature->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(creature->model, &creature->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop, nullptr, nullptr);

	if (EntityAction* currentAction = GetCurrentAction(creature))
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;
		actionAnimation->timer += deltaTime * actionAnimation->speed;

		bool right = true;
		BlendAnimation(creature->model, &creature->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, 1, !currentAction->fullBody ? (AnimationChannelFilterCallback_t)UpperBodyAnimFilter : nullptr);
	}

	ApplyAnimationToSkeleton(creature->model, &creature->anim);

	if (creature->health > 0)
	{
		GetRigidBodyTransform(&creature->body, &entity->position, nullptr);
	}

	mat4 transform = mat4::Translate(entity->position) * mat4::Rotate(vec3::Up, creature->lookDirection);
	if (creature->health <= 0 && !FrustumCulling(creature->model->boundingSphere, transform, game->frustumPlanes))
	{
		entity->removed = true;
	}
}

void RenderCreature(Creature* creature, Entity* entity)
{
	mat4 transform = mat4::Translate(entity->position) * mat4::Rotate(vec3::Up, creature->lookDirection);

	RenderModel(&game->renderer, creature->model, &creature->anim, transform);

#if _DEBUG
	for (int i = 0; i < creature->currentPathLength; i++)
	{
		RenderModel(&game->renderer, &game->cube, nullptr, mat4::Translate(game->mapNavmesh.nodes[creature->currentPath[i]].position));
	}
#endif
}






void InitSkeleton(Entity* skeleton, const vec3& position, float rotation, int health)
{
	InitEntity(skeleton, ENTITY_TYPE_SKELETON);

	skeleton->position = position;

	InitCreature(&skeleton->creature, skeleton, "entities/skeleton/skeleton.glb", rotation, health);
}
