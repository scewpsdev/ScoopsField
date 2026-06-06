#include "Creature.h"

#include "Resource.h"

#include "game/entity/Entity.h"

#include "utils/StringUtils.h"


#include "CreatureLoader.cpp"


void InitCreature(Creature* creature, const char* model, float lookDirection, int health)
{
	creature->lookDirection = lookDirection;

	creature->model = GetModel(model);

	creature->rightWeaponNode = GetNodeByName(creature->model, "Weapon.R");
	creature->rootNode = GetNodeByName(creature->model, "Root");

	creature->lastRootNodeTransform = mat4::Identity;

	InitAnimationState(&creature->anim, creature->model);

	InitAnimation(&creature->idleAnim, "idle", creature->model, 1, true, false);
	InitAnimation(&creature->runAnim, "run", creature->model, 1.6f, true, false);

	InitRigidBody(&creature->body, RIGID_BODY_DYNAMIC, creature->position, quat::FromAxisAngle(vec3::Up, lookDirection), creature);
	AddCapsuleCollider(&creature->body, 0.3f, 2, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_ENEMY, ENTITY_FILTER_DEFAULT, false);
	SetRigidBodyAxisLock(&creature->body, RIGID_BODY_LOCK_ROTATION);

	InitHashMap(&creature->hitboxes);

	InitActionManager(creature->actions, creature->model);

	creature->health = health;
	creature->maxHealth = health;
	creature->walkSpeed = 4;
	creature->turnSpeed = 5;
	creature->damage = 10;
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

static void OnDeath(Creature* creature, HitParams* hit, Node* hitNode)
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

	RigidBody* ragdollBody = hitNode ? HashMapGet(&ragdoll->bones, hash(hitNode->name)) : nullptr;
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

	Node* hitNode = GetNodeForHitbox(creature, hit->body);

	if (creature->health <= 0)
	{
		if (GetAnimationByName(creature->model, "death"))
		{
			EntityAction action;
			InitEntityDeathAction(&action);
			CancelAction(creature->actions, *(Entity*)creature);
			QueueAction(creature->actions, action, *(Entity*)creature);
		}

		OnDeath(creature, hit, hitNode);
	}
	else
	{
		const char* staggerAnimation = SDL_strcmp(hitNode->name, "Head") == 0 ? "stagger_headshot" : "stagger";

		if (GetAnimationByName(creature->model, staggerAnimation))
		{
			EntityAction action;
			InitEntityStaggerAction(&action, 1.0f, staggerAnimation);
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

static EntityAttack* GetAttackByName(Creature* creature, const char* name)
{
	for (int i = 0; i < creature->numAttacks; i++)
	{
		if (SDL_strcmp(creature->attacks[i].name, name) == 0)
			return &creature->attacks[i];
	}
	return nullptr;
}

static bool AttackRequirementsMet(Creature* creature, EntityAttack* attack, float distance, float localAngle)
{
	bool requirementsMet = true;
	requirementsMet = requirementsMet && distance >= attack->rangeTriggerWindow.x && distance <= attack->rangeTriggerWindow.y;
	requirementsMet = requirementsMet && (attack->angleTriggerWindow.y > attack->angleTriggerWindow.x ? localAngle >= attack->angleTriggerWindow.x && localAngle <= attack->angleTriggerWindow.y : localAngle >= attack->angleTriggerWindow.x || localAngle <= attack->angleTriggerWindow.y);
	return requirementsMet;
}

static void UpdateAI(Creature* creature)
{
	Player* target = &game->player;
	creature->targetPosition = target->position;
	/*
	if (EveryInterval(1, hash(creature)))
	{
		creature->targetPosition = target->position;
		bool success = CalculateNavmeshPath(&game->mapNavmesh, creature->position, creature->targetPosition, creature->currentPath, creature->currentPathLength);
		SDL_assert(success);
	}
	*/

	vec3 toTarget = creature->targetPosition - creature->position;
	float distanceToTarget = toTarget.length();
	toTarget /= distanceToTarget;

	vec3 walkDir = toTarget;
	/*
	if (creature->currentPathLength > 0)
	{
		NavmeshNode* targetNode = &game->mapNavmesh.nodes[creature->currentPath[1]];
		walkDir = (targetNode->position - creature->position).normalized();
	}
	*/
	walkDir *= vec3(1, 0, 1);

	quat toTargetRotation = quat::LookAt(walkDir, vec3::Up);
	float toTargetAngle = toTargetRotation.getAngle() * sign(toTargetRotation.getAxis().y);
	creature->targetDirection = toTargetAngle;

	if (distanceToTarget > 0.5f)
	{
		creature->fsu = vec3(0, 0, -1);
		creature->moving = true;
	}
	else
	{
		creature->moving = false;
	}

	float localAngle = mod(toTargetAngle - creature->lookDirection + PI, 2 * PI) - PI;

	DebugText(0, 10, "%.2f, %.2f", distanceToTarget, localAngle);

	EntityAttack* attack = nullptr;
	int attackIdx = 0;

	if (EntityAction* action = GetCurrentAction(creature))
	{
		if (action->type == ENTITY_ACTION_TYPE_ATTACK && action->followUpCancelTime && action->elapsedTime >= action->followUpCancelTime && creature->actions.actions.size == 1)
		{
			EntityAttackAction* attackAction = &action->attack;
			if (attackAction->attack->followUp)
			{
				float f = hash(action->startTime) / (float)UINT32_MAX;
				if (f < attackAction->attack->followUpChance)
				{
					EntityAttack* followUp = GetAttackByName(creature, attackAction->attack->followUp);
					bool requirementsMet = AttackRequirementsMet(creature, followUp, distanceToTarget, localAngle);
					if (requirementsMet)
						attack = followUp;
				}
			}
		}
	}
	else
	{
		List<EntityAttack*, MAX_ENTITY_ATTACKS> possibleAttacks;
		InitList(&possibleAttacks);

		for (int i = 0; i < creature->numAttacks; i++)
		{
			EntityAttack* attack = &creature->attacks[i];

			bool requirementsMet = AttackRequirementsMet(creature, attack, distanceToTarget, localAngle) && attack->firstAttack;
			if (requirementsMet && i == 3 && fabsf(localAngle) < 0.5f * PI)
			{
				__debugbreak();
				AttackRequirementsMet(creature, attack, distanceToTarget, localAngle) && attack->firstAttack;
			}
			if (requirementsMet)
				possibleAttacks.add(attack);
		}

		if (possibleAttacks.size)
		{
			attack = possibleAttacks[game->random.next() % possibleAttacks.size];
		}
	}

	if (attack)
	{
		EntityAction action;
		InitEntityAttackAction(&action, attack, attackIdx);
		QueueAction(creature->actions, action, *(Entity*)creature);
	}
}

static void UpdateRootMotion(Creature* creature)
{
	creature->rootMotionVelocity = vec3::Zero;
	creature->rootMotionAngle = 0;

	mat4& rootNodeTransform = GetNodeTransform(&creature->anim, creature->rootNode);
	vec3 rootMotionTranslation = rootNodeTransform.translation() - creature->lastRootNodeTransform.translation();
	quat rootMotionRotation = creature->lastRootNodeTransform.rotation().conjugated() * rootNodeTransform.rotation();
	//mat4 rootMotionDelta = rootNodeTransform * creature->lastRootNodeTransform.inverted();
	creature->lastRootNodeTransform = rootNodeTransform;
	rootNodeTransform = mat4::Identity;

	if (EntityAction* currentAction = GetCurrentAction(creature))
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;

		if (currentAction->rootMotion && actionAnimation->animation == creature->lastActionAnimation
			&& SDL_fmodf(actionAnimation->timer, actionAnimation->animation->duration) >= SDL_fmodf(creature->lastActionAnimationTimer, actionAnimation->animation->duration))
		{
			//SDL_assert(rootMotionDelta.translation().length() < 0.3f);

			vec3 rootMotion = quat::FromAxisAngle(vec3::Up, PI + creature->lookDirection - creature->currentRootMotionRotation) * rootMotionTranslation;
			//quat rootMotionRot = rootMotionDelta.rotation();
			float rootMotionAngle = rootMotionRotation.getAngle() * sign(rootMotionRotation.getAxis().y);
			//DebugText(0, 11, "%.2f", creature->currentRootMotionRotation);
			//DebugText(0, 12, "%.2f, %.2f, %.2f", rootMotion.x, rootMotion.y, rootMotion.z);

			float dt = gameTime - creature->lastRootMotionUpdate;

			creature->rootMotionVelocity = rootMotion / dt;
			creature->rootMotionAngle = rootMotionAngle;

			// we need this because the root motion translation direction should not be affected by a root motion rotation during this animation
			// since it is applied independently in blender. otherwise the translation will constantly change direction as the rotation is turning the character.
			// we may want to rotate the character manually which does affect translation direction
			creature->currentRootMotionRotation += rootMotionAngle;
		}
		else
		{
			creature->currentRootMotionRotation = 0;
		}

		creature->lastActionAnimation = actionAnimation->animation;
		creature->lastActionAnimationTimer = actionAnimation->timer;
	}

	creature->lastRootMotionUpdate = gameTime;
}

void UpdateCreature(Creature* creature)
{
	if (creature->health > 0)
	{
		UpdateAI(creature);
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

	UpdateRootMotion(creature);

	ApplyAnimationToSkeleton(creature->model, &creature->anim);

	float walkSpeed = creature->walkSpeed;
	if (EntityAction* currentAction = GetCurrentAction(creature))
		walkSpeed *= currentAction->walkSpeed;
	vec3 walkVelocity = creature->fsu * walkSpeed;
	walkVelocity = quat::FromAxisAngle(vec3::Up, creature->lookDirection) * walkVelocity;
	walkVelocity += creature->rootMotionVelocity;
	SetRigidBodyVelocity(&creature->body, walkVelocity, vec3(0));

	float turnSpeed = creature->turnSpeed;
	if (EntityAction* currentAction = GetCurrentAction(creature))
		turnSpeed *= currentAction->turnSpeed;
	creature->lookDirection = moveTowardsAngle(creature->lookDirection, creature->targetDirection, turnSpeed * deltaTime);
	creature->lookDirection += creature->rootMotionAngle;

	if (creature->health > 0)
	{
		GetRigidBodyTransform(&creature->body, &creature->position, nullptr);
		//creature->rotation = quat::FromAxisAngle(vec3::Up, creature->lookDirection);
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
				mat4 nodeTransform = transform * mat4::Rotate(vec3::Up, PI) * GetNodeTransform(&creature->anim, node);
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
	mat4 transform = mat4::Translate(creature->position) * mat4::Rotate(vec3::Up, creature->lookDirection + PI);
	RenderModel(&game->renderer, creature->model, &creature->anim, transform);

	// render weapon
	for (int i = 0; i < creature->model->numNodes; i++)
	{
		Node* node = &creature->model->nodes[i];
		if (SDL_strcmp(node->name, "Weapon") == 0)
		{
			mat4 weaponTransform = transform * GetNodeTransform(&creature->anim, creature->rightWeaponNode);
			RenderModelNode(&game->renderer, creature->model, node, nullptr, nullptr, weaponTransform, weaponTransform);
		}
	}

#if _DEBUG
	for (int i = 0; i < creature->currentPathLength; i++)
	{
		RenderModel(&game->renderer, &game->cube, nullptr, mat4::Translate(game->mapNavmesh.nodes[creature->currentPath[i]].position));
	}
#endif
}




static EntityAttack* AddAttack(Creature* creature, const char* name, const char* animation, bool firstAttack, float animationSpeed, float damageMultiplier, ivec2 damageWindow, int cancelFrame, vec2 rangeTriggerWindow = vec2(0, 5), vec2 angleTriggerWindow = vec2(-0.5f * PI, 0.5f * PI), const char* followUp = nullptr, float followUpChance = 1.0f)
{
	int attackID = creature->numAttacks++;
	EntityAttack* attack = &creature->attacks[attackID];
	attack->name = name;
	attack->animation = animation;
	attack->animationSpeed = animationSpeed;
	attack->firstAttack = firstAttack;
	attack->damageWindow = damageWindow / 24.0f / animationSpeed;
	attack->rangeTriggerWindow = rangeTriggerWindow;
	attack->angleTriggerWindow = angleTriggerWindow;
	attack->followUpCancelTime = cancelFrame / 24.0f / animationSpeed;
	attack->damageMultiplier = damageMultiplier;
	attack->followUp = followUp;
	attack->followUpChance = followUpChance;

	//AddAttackSound(attack, &game->swingSound, attack->damageWindow.x, 1, 1, (attackID % 2 * -2 + 1) * 0.2f);

	return attack;
}

void InitSkeleton(Entity* skeleton, const vec3& position, float rotation)
{
	InitEntity(skeleton, ENTITY_TYPE_CREATURE);
	skeleton->position = position;

	InitCreature(&skeleton->creature, "entities/skeleton/skeleton.glb", rotation, 100);

	skeleton->creature.hitSound = &game->skeletonHitSound;
}

void InitKnight(Creature* creature, const vec3& position, float rotation)
{
	InitEntity((Entity*)creature, ENTITY_TYPE_CREATURE);
	creature->position = position;

	InitCreature(creature, "entities/creature/knight/knight.glb", rotation, 200);
	LoadCreatureHitbox(creature, "entities/creature/knight/knight.rfs");

	creature->damage = 20;
	creature->weaponRange = 1.2f;

	AddAttack(creature, "slam", "attack_slam", true, 1, 1, ivec2(15, 23), 40, vec2(1, 4), vec2(-0.5f * PI, 0.5f * PI), "slash", 0.5f);
	AddAttack(creature, "slash", "attack_slash_backhand", false, 1, 1, ivec2(25, 33), 48, vec2(1, 4));
	AddAttack(creature, "backstep", "attack_backstep", true, 1, 1, ivec2(0), 0, vec2(0, 2));
	AddAttack(creature, "turnaround", "attack_turnaround", true, 1, 1, ivec2(0), 0, vec2(0, 5), vec2(0.5f * PI, -0.5f * PI));

	creature->hitSound = &game->hitArmorSound;
}
