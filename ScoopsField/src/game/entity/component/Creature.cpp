#include "Creature.h"

#include "Resource.h"

#include "game/entity/Entity.h"

#include "utils/StringUtils.h"


#include "CreatureLoader.cpp"


void InitCreature(Creature* creature, const char* model, float lookDirection, int health)
{
	creature->lookDirection = lookDirection;

	creature->model = GetModel(model);
	InitAnimationState(&creature->anim, creature->model);

	InitAnimation(&creature->idleAnim, "run", creature->model, 3, true, false);
	InitAnimation(&creature->runAnim, "run", creature->model, 1, true, false);

	InitRigidBody(&creature->body, RIGID_BODY_DYNAMIC, creature->position, quat::FromAxisAngle(vec3::Up, lookDirection), creature);
	AddCapsuleCollider(&creature->body, 0.3f, 2, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_ENEMY, ENTITY_FILTER_DEFAULT, false);
	SetRigidBodyAxisLock(&creature->body, RIGID_BODY_LOCK_ROTATION);

	InitHashMap(&creature->hitboxes);

	InitActionManager(creature->actions, creature->model);

	creature->health = health;
	creature->maxHealth = health;
}

void DestroyCreature(Creature* creature)
{
	DestroyRigidBody(&creature->body);

	for (int i = 0; i < creature->hitboxes.capacity; i++)
	{
		if (creature->hitboxes.slots[i].state == SLOT_USED)
		{
			RigidBody* bone = &creature->hitboxes.slots[i].value;
			DestroyRigidBody(bone);
		}
	}
}

static Node* GetNodeForHitbox(Creature* creature, RigidBody* hitbox)
{
	SDL_assert(creature->hitboxes.numUsedSlots);

	for (int i = 0; i < creature->model->numNodes; i++)
	{
		Node* node = &creature->model->nodes[i];
		uint32_t h = hash(node->name);
		if (hitbox == HashMapGet(&creature->hitboxes, h))
			return node;
	}

	return nullptr;
}

static void OnDeath(Creature* creature, HitParams* hit)
{
	/*
	SetRigidBodyVelocity(&creature->body, vec3(0), vec3(0));

	RemoveColliders(&creature->body);

	if (creature->hitboxes.numUsedSlots)
	{
		for (int i = 0; i < creature->model->numNodes; i++)
		{
			Node* node = &creature->model->nodes[i];
			uint32_t h = hash(node->name);
			if (RigidBody* hitbox = HashMapGet(&creature->hitboxes, h))
			{
				RemoveColliders(hitbox);
			}
		}
	}
	*/

	// spawn ragdoll

	Ragdoll* ragdoll = (Ragdoll*)CreateEntity();
	InitRagdoll(ragdoll, creature);

	RigidBody* ragdollBody = nullptr;
	for (int i = 0; i < creature->hitboxes.capacity; i++)
	{
		if (creature->hitboxes.slots[i].state == SLOT_USED && &creature->hitboxes.slots[i].value == hit->body)
		{
			ragdollBody = &ragdoll->bones.slots[i].value;
			SDL_assert(ragdoll->bones.slots[i].state == SLOT_USED);
			SDL_assert(ragdoll->bones.slots[i].key == creature->hitboxes.slots[i].key);
			break;
		}
	}

	if (ragdollBody)
	{
		AddRigidBodyImpulse(ragdollBody, hit->impulse);
	}

	creature->removed = true;

	//SDL_assert(game->numSkeletonsRemaining > 0);
	//game->numSkeletonsRemaining--;
}

bool HitCreature(Creature* creature, HitParams* hit, Entity* by)
{
	if (creature->health <= 0)
		return false;

	int damage = (int)(hit->damage * hit->damageMultiplier);
	creature->health -= damage;

	PlaySound(creature->hitSound, hit->position);

	if (creature->health <= 0)
	{
		if (GetAnimationByName(creature->model, "death"))
		{
			EntityAction action;
			InitEntityDeathAction(&action);
			CancelAction(creature->actions, *(Entity*)creature);
			QueueAction(creature->actions, action, *(Entity*)creature);
		}

		OnDeath(creature, hit);
	}
	else
	{
		if (GetAnimationByName(creature->model, "stagger"))
		{
			EntityAction action;
			InitEntityStaggerAction(&action, 1.0f);
			CancelAction(creature->actions, *(Entity*)creature);
			QueueAction(creature->actions, action, *(Entity*)creature);
		}
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

static void UpdateAI(Creature* creature)
{
	Player* target = &game->player;
	if (EveryInterval(1, hash(creature)))
	{
		creature->targetPosition = target->position;
		bool success = CalculateNavmeshPath(&game->mapNavmesh, creature->position, creature->targetPosition, creature->currentPath, creature->currentPathLength);
		SDL_assert(success);
	}

	vec3 toTarget = creature->targetPosition - creature->position;
	float distanceToTarget = toTarget.length();
	toTarget /= distanceToTarget;

	vec3 walkDir = toTarget;
	if (creature->currentPathLength > 0)
	{
		NavmeshNode* targetNode = &game->mapNavmesh.nodes[creature->currentPath[1]];
		walkDir = (targetNode->position - creature->position).normalized();
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

	if ((target->position - creature->position).length() < 1)
	{
		if (creature->actions.actions.size == 0)
		{
			EntityAction action;
			InitEntityAttackAction(&action, "attack1", 0);
			QueueAction(creature->actions, action, *(Entity*)creature);
		}
	}

	//GetRigidBodyTransform(&creature->body, &entity->position, nullptr);
	//AddRigidBodyAcceleration(&skeleton->body, vec3(0, -10, 0));
}

void UpdateCreature(Creature* creature)
{
	if (creature->health > 0)
	{
		//UpdateAI(skeleton);
	}

	UpdateActionManager(creature->actions, *(Entity*)creature);

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
		GetRigidBodyTransform(&creature->body, &creature->position, nullptr);
	}

	mat4 transform = mat4::Translate(creature->position) * mat4::Rotate(vec3::Up, creature->lookDirection);

	if (creature->hitboxes.numUsedSlots)
	{
		for (int i = 0; i < creature->model->numNodes; i++)
		{
			Node* node = &creature->model->nodes[i];
			uint32_t h = hash(node->name);
			if (RigidBody* hitbox = HashMapGet(&creature->hitboxes, h))
			{
				mat4 nodeTransform = transform * GetNodeTransform(&creature->anim, node);
				SetRigidBodyTransform(hitbox, nodeTransform.translation(), nodeTransform.rotation());
			}
		}
	}

	if (creature->health <= 0 && !FrustumCulling(creature->model->boundingSphere, transform, game->frustumPlanes))
	{
		creature->removed = true;
	}
}

void RenderCreature(Creature* creature)
{
	mat4 transform = mat4::Translate(creature->position) * mat4::Rotate(vec3::Up, creature->lookDirection);

	RenderModel(&game->renderer, creature->model, &creature->anim, transform);

#if _DEBUG
	for (int i = 0; i < creature->currentPathLength; i++)
	{
		RenderModel(&game->renderer, &game->cube, nullptr, mat4::Translate(game->mapNavmesh.nodes[creature->currentPath[i]].position));
	}
#endif
}






void InitSkeleton(Entity* skeleton, const vec3& position, float rotation)
{
	InitEntity(skeleton, ENTITY_TYPE_CREATURE);
	skeleton->position = position;

	InitCreature(&skeleton->creature, "entities/skeleton/skeleton.glb", rotation, 100);

	skeleton->creature.hitSound = &game->skeletonHitSound;
}

void InitKnight(Entity* creature, const vec3& position, float rotation)
{
	InitEntity(creature, ENTITY_TYPE_CREATURE);
	creature->position = position;

	InitCreature(&creature->creature, "entities/creature/knight/knight.glb", rotation, 200);
	LoadCreatureHitbox(&creature->creature, "entities/creature/knight/knight.rfs");

	creature->creature.hitSound = &game->hitArmorSound;
}
