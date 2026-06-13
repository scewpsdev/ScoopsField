#include "Creature.h"

#include "Resource.h"

#include "game/entity/Entity.h"

#include "model/Model.h"
#include "model/Animation.h"

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
	AddCapsuleCollider(&creature->body, 0.3f, 2, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_ENEMY, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY, false);
	SetRigidBodyAxisLock(&creature->body, RIGID_BODY_LOCK_ROTATION);

	InitHashMap(&creature->hitboxes);

	InitActionManager(creature->actions, creature->model);

	creature->health = health;
	creature->maxHealth = health;
	creature->walkSpeed = 4;
	creature->turnSpeed = 5;
	creature->damage = 10;
	creature->eyePosition = vec3(0, 1.5f, 0);
	creature->detectionRange = 20;
	creature->detectionAngle = PI * 0.75f;
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

	DestroyAnimationState(&creature->anim);
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

	Ragdoll* ragdoll = (Ragdoll*)CreateEntity();
	InitRagdoll(ragdoll, creature);
	creature->ragdoll = ragdoll;

	RigidBody* ragdollBody = hitNode ? HashMapGet(&ragdoll->bones, hash(hitNode->name)) : nullptr;
	if (ragdollBody)
	{
		AddRigidBodyImpulse(ragdollBody, hit->impulse);
	}

	creature->removed = true;

	//SDL_assert(game->numSkeletonsRemaining > 0);
	//game->numSkeletonsRemaining--;
}

static bool IsEntityVisible(Creature* creature, Entity* entity);

bool HitCreature(Creature* creature, HitParams* hit, Entity* by)
{
	if (creature->health <= 0)
		return false;

	SDL_assert(hit->damageType != DAMAGE_TYPE_NONE);

	float damage = hit->damage;

	Node* hitNode = GetNodeForHitbox(creature, hit->body);
	bool isHeadshot = SDL_strcmp(hitNode->name, "Head") == 0;
	hit->wasHeadshot = isHeadshot;
	if (isHeadshot)
		damage *= 2;

	creature->health -= (int)damage;

	PlaySound(creature->hitSound, hit->position);

	SDL_assert(hit->impulse.lengthSquared() != 0);

	ParticleEffect* hitParticles = (ParticleEffect*)CreateEntity();
	LoadParticleEffect(hitParticles, "effects/impact/spark.rfs", hit->position, quat::LookAt(hit->impulse, vec3::Up));
	hitParticles->destroyOnFinish = true;

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
		const char* staggerAnimation = isHeadshot ? "stagger_headshot" : "stagger";

		if (GetAnimationByName(creature->model, staggerAnimation))
		{
			EntityAction action;
			InitEntityStaggerAction(&action, 1.0f, staggerAnimation);
			CancelAction(creature->actions, *(Entity*)creature);
			QueueAction(creature->actions, action, *(Entity*)creature);
		}

		Entity* attacker = by;
		if (attacker->type == ENTITY_TYPE_PROJECTILE)
		{
			Projectile* projectile = (Projectile*)attacker;
			attacker = projectile->shooter;
		}

		if ((attacker->type == ENTITY_TYPE_PLAYER || attacker->type == ENTITY_TYPE_CREATURE) && !creature->target)
		{
			if (IsEntityVisible(creature, attacker))
				creature->target = attacker;
			else
			{
				vec3 origin = creature->position + creature->eyePosition;
				vec3 target = attacker->position + vec3(0, 1, 0);
				vec3 dir = target - origin;
				float d = dir.length();

				PhysicsHit hits[16];
				int numHits = Raycast(origin, dir / d, d, hits, 16, ENTITY_FILTER_DEFAULT);
				for (int i = 0; i < numHits; i++)
				{
					creature->targetPosition = hits[i].position;
					break;
				}
			}
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

static bool ExcludeRootAnimFilter(Node* node, void* userPtr)
{
	return SDL_strcmp(node->name, "Root") != 0;
}

mat4 GetRightWeaponTransform(Creature* creature)
{
	mat4 transform = mat4::Translate(creature->position) * mat4::Rotate(vec3::Up, creature->lookDirection + PI);
	return transform * GetNodeTransform(&creature->anim, creature->rightWeaponNode);
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

static void UpdateAttackAI(Creature* creature, float distanceToTarget, float toTargetAngle)
{
	float localAngle = mod(toTargetAngle - creature->lookDirection + PI, 2 * PI) - PI;

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

static bool IsEntityVisible(Creature* creature, Entity* entity)
{
	vec3 origin = creature->position + creature->eyePosition;
	vec3 target = entity->position + vec3(0, 1, 0);
	return !Linecast(origin, target, ENTITY_FILTER_DEFAULT);
}

static bool IsInViewCone(Creature* creature, Entity* entity, vec3 toTarget, float distanceToTarget)
{
	quat toTargetRotation = quat::LookAt(toTarget * vec3(1, 0, 1), vec3::Up);
	float toTargetAngle = toTargetRotation.getAngle() * sign(toTargetRotation.getAxis().y);

	float localAngle = mod(toTargetAngle - creature->lookDirection + PI, 2 * PI) - PI;

	return distanceToTarget <= creature->detectionRange && localAngle >= -0.5f * creature->detectionAngle && localAngle <= 0.5f * creature->detectionAngle;
}

static bool IsEntityAudible(Creature* creature, Entity* entity, float distanceToTarget)
{
	const float hearingRange = 2.0f;
	float effectiveHearingRange = hearingRange * (entity->type == ENTITY_TYPE_PLAYER && ((Player*)entity)->ducked ? 0.25f : 1);
	return distanceToTarget < effectiveHearingRange;
}

static void UpdateAI(Creature* creature)
{
	creature->fsu = vec3(0);

	if (!creature->target)
	{
		Entity* potentialTarget = (Entity*)&game->player;

		vec3 toTarget = potentialTarget->position - creature->position;
		float distanceToTarget = toTarget.length();

		if (IsEntityVisible(creature, potentialTarget))
		{
			if (IsInViewCone(creature, potentialTarget, toTarget, distanceToTarget) || IsEntityAudible(creature, potentialTarget, distanceToTarget))
				creature->target = potentialTarget;
		}
	}

	if (creature->target)
	{
		vec3 toTarget = creature->target->position - creature->position;
		float distanceToTarget = toTarget.length();

		quat toTargetRotation = quat::LookAt(toTarget * vec3(1, 0, 1), vec3::Up);
		float toTargetAngle = toTargetRotation.getAngle() * sign(toTargetRotation.getAxis().y);

		if (IsEntityVisible(creature, creature->target))
		{
			creature->targetPosition = creature->target->position;

			UpdateAttackAI(creature, distanceToTarget, toTargetAngle);
		}
		else
		{
			creature->searchEntity = creature->target;
			creature->target = nullptr;
		}
	}

	if (creature->targetPosition != vec3::Zero && !creature->target && creature->searchEntity)
	{
		if (IsEntityVisible(creature, creature->searchEntity) && (creature->searchEntity->position - creature->position).length() < creature->detectionRange)
		{
			creature->target = creature->searchEntity;
			creature->searchEntity = nullptr;
		}
	}

	if (creature->targetPosition != vec3::Zero)
	{
		/*
		if (EveryInterval(1, hash(creature)))
		{
			creature->targetPosition = target->position;
			bool success = CalculateNavmeshPath(&game->mapNavmesh, creature->position, creature->targetPosition, creature->currentPath, creature->currentPathLength);
			SDL_assert(success);
		}
		*/

		vec3 toTargetPosition = creature->targetPosition - creature->position;
		float distanceToTargetPosition = toTargetPosition.length();
		toTargetPosition /= distanceToTargetPosition;

		/*
		if (creature->currentPathLength > 0)
		{
			NavmeshNode* targetNode = &game->mapNavmesh.nodes[creature->currentPath[1]];
			walkDir = (targetNode->position - creature->position).normalized();
		}
		*/

		quat toTargetPositionRot = quat::LookAt(toTargetPosition * vec3(1, 0, 1), vec3::Up);
		float toTargetAngle = toTargetPositionRot.getAngle() * sign(toTargetPositionRot.getAxis().y);
		creature->targetDirection = toTargetAngle;

		if (distanceToTargetPosition > 0.5f)
		{
			creature->fsu = vec3(0, 0, -1);
			creature->moving = true;
		}
		else
		{
			creature->moving = false;

			creature->targetPosition = vec3(0);
			creature->searchEntity = nullptr;
		}
	}
}

static void UpdateAnimationBlending(Creature* creature, Animation* currentAnimation, float timer, bool loop)
{
	if (currentAnimation != creature->lastAnim && creature->lastAnim)
	{
		creature->blendStart = gameTime;
		creature->blendAnim = creature->lastAnim;
		creature->blendAnimTimer = creature->lastAnimTimer;
		creature->blendAnimLoop = creature->lastAnimLoop;
		creature->blendDuration = 0.2f;
	}

	if (creature->blendStart)
	{
		float blendProgress = creature->blendDuration ? (gameTime - creature->blendStart) / creature->blendDuration : 1;
		if (blendProgress >= 1)
		{
			creature->blendStart = 0;
		}
		else
		{
			BlendAnimation(creature->model, &creature->anim, creature->blendAnim, creature->blendAnimTimer, creature->blendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ExcludeRootAnimFilter, nullptr);
		}
	}

	creature->lastAnim = currentAnimation;
	creature->lastAnimTimer = timer;
	creature->lastAnimLoop = loop;
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


	AnimationPlayback* currentAnimation = nullptr;
	if (EntityAction* currentAction = GetCurrentAction(creature))
		currentAnimation = &currentAction->anim;
	else
		currentAnimation = creature->moving ? &creature->runAnim : &creature->idleAnim;

	currentAnimation->timer += deltaTime * currentAnimation->speed;

	AnimateModel(creature->model, &creature->anim, currentAnimation->animation, currentAnimation->timer, currentAnimation->loop, nullptr, nullptr);

	/*
	if (EntityAction* currentAction = GetCurrentAction(creature))
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;
		actionAnimation->timer += deltaTime * actionAnimation->speed;

		bool right = true;
		BlendAnimation(creature->model, &creature->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, 1, !currentAction->fullBody ? (AnimationChannelFilterCallback_t)UpperBodyAnimFilter : nullptr);
	}
	*/

	UpdateRootMotion(creature);

	UpdateAnimationBlending(creature, currentAnimation->animation, currentAnimation->timer, currentAnimation->loop);

	ApplyAnimationToSkeleton(creature->model, &creature->anim);

	float walkSpeed = creature->walkSpeed;
	if (EntityAction* currentAction = GetCurrentAction(creature))
		walkSpeed *= currentAction->walkSpeed;
	vec3 walkVelocity = creature->fsu * walkSpeed;
	walkVelocity = quat::FromAxisAngle(vec3::Up, creature->lookDirection) * walkVelocity;
	walkVelocity += creature->rootMotionVelocity;

	vec3 currentVelocity;
	GetRigidBodyVelocity(&creature->body, &currentVelocity, nullptr);

	SetRigidBodyVelocity(&creature->body, vec3(walkVelocity.x, currentVelocity.y, walkVelocity.z), vec3(0));

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

	for (int i = 0; i < creature->model->numNodes; i++)
	{
		Node* node = &creature->model->nodes[i];
		if (SDL_strcmp(node->name, "Weapon") == 0)
		{
			mat4 weaponTransform = GetRightWeaponTransform(creature);
			RenderModelNode(&game->renderer, creature->model, node, nullptr, nullptr, weaponTransform, weaponTransform, 0);
		}
	}

#if _DEBUG
	for (int i = 0; i < creature->currentPathLength; i++)
	{
		RenderModel(&game->renderer, &game->cube, nullptr, mat4::Translate(game->mapNavmesh.nodes[creature->currentPath[i]].position));
	}
#endif
}



static void AddAttackSound(EntityAttack* attack, Sound* sound, int frame, float volume, float speed)
{
	SDL_assert(attack->numSounds < MAX_ATTACK_SOUNDS);
	AttackSound* attackSound = &attack->sounds[attack->numSounds++];
	attackSound->sound = sound;
	attackSound->time = frame / 24.0f;
	attackSound->volume = volume;
	attackSound->speed = speed;
	attackSound->pan = 0;
}

static void AddAttackEffect(EntityAttack* attack, const char* path, float time, vec3 localPosition)
{
	SDL_assert(attack->numEffects < MAX_ATTACK_EFFECTS);
	AttackEffect* attackEffect = &attack->effects[attack->numEffects++];
	attackEffect->path = path;
	attackEffect->time = time;
	attackEffect->localPosition = localPosition;
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

	if (attack->damageWindow.x)
		AddAttackSound(attack, &game->swingSound, damageWindow.x, 1, 0.5f);

	return attack;
}

void InitSkeleton(Entity* skeleton, const vec3& position, float rotation)
{
	InitEntity(skeleton, ENTITY_TYPE_CREATURE);
	skeleton->position = position;

	InitCreature(&skeleton->creature, "entities/skeleton/skeleton.glb", rotation, 100);

	skeleton->creature.hitSound = &game->hitSkeletonSound;
}

void InitKnight(Creature* creature, const vec3& position, float rotation)
{
	InitEntity((Entity*)creature, ENTITY_TYPE_CREATURE);
	creature->position = position;

	InitCreature(creature, "entities/creature/knight/knight.glb", rotation, 300);
	LoadCreatureHitbox(creature, "entities/creature/knight/knight.rfs");

	creature->damage = 20;
	creature->weaponRange = 1.5f;

	EntityAttack* slam = AddAttack(creature, "slam", "attack_slam", true, 1, 1, ivec2(15, 23), 40, vec2(0, 2.5f), vec2(-0.5f * PI, 0.5f * PI), "slash", 0.8f);
	AddAttackSound(slam, &game->stepSound, 15, 2, 1);
	//AddAttackSound(slam, &game->armorSound, 15, 1, 1);
	AddAttackSound(slam, &game->armorSound, 20, 1, 1);
	AddAttackSound(slam, &game->armorSound, 40, 1, 1);

	EntityAttack* slash = AddAttack(creature, "slash", "attack_slash_backhand", false, 1, 1, ivec2(25, 33), 48, vec2(0, 3.5f));
	AddAttackSound(slash, &game->stepSound, 5, 2, 1);
	AddAttackSound(slash, &game->stepSound, 10, 2, 1);
	//AddAttackSound(slash, &game->armorSound, 5, 1, 1);
	//AddAttackSound(slash, &game->armorSound, 10, 1, 1);
	AddAttackSound(slash, &game->armorSound, 23, 1, 1);
	AddAttackSound(slash, &game->armorSound, 55, 1, 1);

	EntityAttack* backstep = AddAttack(creature, "backstep", "attack_backstep", true, 1, 1, ivec2(0), 0, vec2(0, 1.5f));
	AddAttackSound(backstep, &game->stepSound, 12, 2, 1);

	EntityAttack* turnaround = AddAttack(creature, "turnaround", "attack_turnaround", true, 1, 1, ivec2(0), 0, vec2(0, 5), vec2(0.5f * PI, -0.5f * PI));
	AddAttackSound(turnaround, &game->armorSound, 4, 1, 1);
	AddAttackSound(turnaround, &game->stepSound, 10, 2, 1);

	creature->hitSound = &game->hitArmorSound;
}
