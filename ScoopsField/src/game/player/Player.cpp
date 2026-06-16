#include "Player.h"

#include "math/Math.h"


#define HEALTH_REGEN_HIT_DELAY 5.0f
#define HIT_RECOVERY_DURATION 0.25f
#define STEP_FREQUENCY 0.6f
#define PLAYER_REACH 2.5f
#define CAMERA_HEIGHT 1.49f
#define CAMERA_HEIGHT_DUCKED 0.94f
#define CONTROLLER_HEIGHT 1.7f
#define CONTROLLER_HEIGHT_DUCKED (CONTROLLER_HEIGHT + (CAMERA_HEIGHT_DUCKED - CAMERA_HEIGHT))
//#define CAMERA_HEIGHT_DUCKED (CAMERA_HEIGHT - (CONTROLLER_HEIGHT - CONTROLLER_HEIGHT_DUCKED))


Action* GetCurrentAction(Player* player)
{
	return QueuePeek(player->actions.actions);
}

static void SetRightWeapon(Player* player, int loadout, Item* weapon)
{
	if (player->rightWeapons[loadout] != weapon)
	{
		Item* lastWeapon = player->rightWeapons[loadout];

		if (lastWeapon && lastWeapon->model.numAnimations)
			DestroyAnimationState(&player->rightWeaponAnim);

		player->rightWeapons[loadout] = weapon;

		if (loadout == player->currentLoadout)
		{
			if (weapon)
			{
				if (weapon->model.numAnimations)
					InitAnimationState(&player->rightWeaponAnim, &weapon->model);

				Action action;
				InitEquipAction(&action, weapon);
				QueueAction(player->actions, action, *player);
			}
			else if (lastWeapon)
			{
				//Action action;
				//InitUnequipAction(&action, lastWeapon);
				//QueueAction(player->actions, action, *player);
			}
		}
	}
}

static void SetLeftWeapon(Player* player, int loadout, Item* weapon)
{
	if (player->leftWeapons[loadout] != weapon)
	{
		Item* lastWeapon = player->leftWeapons[loadout];

		//if (lastWeapon && lastWeapon->model.numAnimations)
		//	DestroyAnimationState(&player->leftWeaponAnim);

		player->leftWeapons[loadout] = weapon;

		if (loadout == player->currentLoadout)
		{
			if (weapon)
			{
				//if (lastWeapon->model.numAnimations)
				//	DestroyAnimationState(&player->leftWeaponAnim);
				//if (weapon->model.numAnimations)
				//	InitAnimationState(&player->leftWeaponAnim, &weapon->model);

				Action action;
				InitEquipAction(&action, weapon);
				QueueAction(player->actions, action, *player);
			}
			else if (lastWeapon)
			{
				Action action;
				InitUnequipAction(&action, lastWeapon);
				QueueAction(player->actions, action, *player);
			}
		}
	}
}

Item* GetRightWeapon(Player* player)
{
	return player->rightWeapons[player->currentLoadout];
}

Item* GetLeftWeapon(Player* player)
{
	return player->leftWeapons[player->currentLoadout];
}

Item* GetRightApparentWeapon(Player* player)
{
	Item* rightWeapon = GetRightWeapon(player);
	Item* leftWeapon = GetLeftWeapon(player);

	if (rightWeapon)
	{
		return !rightWeapon->twoHanded && leftWeapon && leftWeapon->twoHanded ? leftWeapon : rightWeapon;
	}
	else
	{
		return leftWeapon && leftWeapon->twoHanded ? leftWeapon : nullptr;
	}
}

Item* GetLeftApparentWeapon(Player* player)
{
	Item* rightWeapon = GetRightWeapon(player);
	Item* leftWeapon = GetLeftWeapon(player);

	if (leftWeapon)
	{
		return rightWeapon && rightWeapon->twoHanded ? rightWeapon : leftWeapon;
	}
	else
	{
		return rightWeapon && rightWeapon->twoHanded ? rightWeapon : nullptr;
	}
}

quat GetCameraRotation(Player* player)
{
	if (GetKey(SDL_SCANCODE_O))
		return quat::Identity;
	return quat::FromAxisAngle(vec3::Up, player->yaw) * quat::FromAxisAngle(vec3::Right, player->pitch);
}

mat4 GetCameraTransform(Player* player)
{
	return mat4::Translate(game->cameraPosition) * mat4::Rotate(GetCameraRotation(player));
}

mat4 GetRightWeaponTransform(Player* player)
{
	//mat4 cameraTransform = mat4::Translate(game->cameraPosition) * mat4::Rotate(GetCameraRotation(player));
	mat4 viewmodelTransform = mat4::Translate(player->position) * mat4::Rotate(vec3::Up, player->rotation + PI);
	return viewmodelTransform * GetNodeTransform(&player->bodyAnim, player->rightWeaponNode);
}

mat4 GetLeftWeaponTransform(Player* player)
{
	//mat4 cameraTransform = mat4::Translate(game->cameraPosition) * mat4::Rotate(GetCameraRotation(player));
	mat4 viewmodelTransform = mat4::Translate(player->position) * mat4::Rotate(vec3::Up, player->rotation + PI);
	return viewmodelTransform * GetNodeTransform(&player->bodyAnim, player->leftWeaponNode);
}

void SwitchLoadout(Player* player, int loadout)
{
	if (player->currentLoadout != loadout)
	{
		if (player->rightWeapons[loadout])
		{
			if (player->rightWeapons[player->currentLoadout] && player->rightWeapons[player->currentLoadout]->model.numAnimations)
				DestroyAnimationState(&player->rightWeaponAnim);
			if (player->rightWeapons[loadout]->model.numAnimations)
				InitAnimationState(&player->rightWeaponAnim, &player->rightWeapons[loadout]->model);

			Action action;
			InitEquipAction(&action, player->rightWeapons[loadout], loadout);
			QueueAction(player->actions, action, *player);
		}
		else if (player->rightWeapons[player->currentLoadout])
		{
			Action action;
			InitUnequipAction(&action, player->rightWeapons[player->currentLoadout], loadout);
			QueueAction(player->actions, action, *player);
		}
	}
}


#include "SourceMovement.cpp"


void InitPlayer(Player* player, SDL_GPUCommandBuffer* cmdBuffer, vec3 position, float rotation)
{
	SDL_memset(player, 0, sizeof(Player));
	InitEntity((Entity*)player, ENTITY_TYPE_PLAYER);

	player->position = position;
	player->rotation = rotation;
	player->yaw = rotation;

	player->model = GetModel("models/viewmodel.glb");
	//LoadModel(player->model, "res/models/viewmodel.glb.bin", false, cmdBuffer);
	InitAnimationState(&player->anim, player->model);

	player->bodyModel = GetModel("models/bodymodel_.glb");
	//LoadModel(player->bodyModel, "res/models/bodymodel_.glb.bin", false, cmdBuffer);
	InitAnimationState(&player->bodyAnim, player->bodyModel);

	player->rightWeaponNode = GetNodeByName(player->bodyModel, "weapon_r");
	player->leftWeaponNode = GetNodeByName(player->bodyModel, "weapon_l");
	player->rootNode = GetNodeByName(player->model, "root");

	player->rightShoulderNode = GetNodeByName(player->bodyModel, "clavicle_r");
	player->leftShoulderNode = GetNodeByName(player->bodyModel, "clavicle_l");
	player->neckNode = GetNodeByName(player->bodyModel, "neck_01");
	player->spineNode = GetNodeByName(player->bodyModel, "spine_03");
	player->spine2Node = GetNodeByName(player->bodyModel, "spine_02");
	player->pelvisNode = GetNodeByName(player->bodyModel, "pelvis");

	player->lastRootNodeTransform = mat4::Identity;

	InitAnimation(&player->idleAnim, "idle", player->model, 0.005f, true, false);
	InitAnimation(&player->bodyIdleAnim, "idle", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodyRunAnim, "run", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodyStrafeAnim, "strafe", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodyDuckAnim, "duck", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodySneakAnim, "sneak", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodySneakStrafeAnim, "sneak_strafe", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodyFallAnim, "fall", player->bodyModel, 1.0f, true, false);
	InitAnimation(&player->bodyFallDuckAnim, "fall_duck", player->bodyModel, 1.0f, true, false);

	InitCharacterController(&player->controller, 0.3f, CONTROLLER_HEIGHT, 0.2f, player->position, player);
	InitRigidBody(&player->kinematicBody, RIGID_BODY_KINEMATIC, player->position, quat::Identity, player);
	AddCapsuleCollider(&player->kinematicBody, 0.2f, 1.5f, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_PLAYER, ENTITY_FILTER_ENEMY | ENTITY_FILTER_RAGDOLL, false);

	InitActionManager(player->actions, player->model, player->bodyModel);

	player->walkSpeed = 4.0f;

	player->health = 100;
	player->maxHealth = 100;

	player->stamina = 1.0f;
	player->exhausted = false;

	SetRightWeapon(player, 0, GetItem(ITEM_DARKWOOD_STAFF));
	SetRightWeapon(player, 1, GetItem(ITEM_KINGS_SWORD));
	SetRightWeapon(player, 2, GetItem(ITEM_SHORTBOW));
	//SetLeftWeapon(player, 1, GetItem(ITEM_WOODEN_SHIELD));
}

void DestroyPlayer(Player* player)
{
	DestroyCharacterController(&player->controller);
	DestroyRigidBody(&player->kinematicBody);

	DestroyAnimationState(&player->anim);
	DestroyAnimationState(&player->bodyAnim);

	if (player->rightWeapons[player->currentLoadout] && player->rightWeapons[player->currentLoadout]->model.numAnimations)
		DestroyAnimationState(&player->rightWeaponAnim);

	//DestroyModel(player->model);
	//DestroyAnimationState(&player->anim);
}

void MovePlayer(Player* player, vec3 delta)
{
	MoveCharacterController(&player->controller, delta, ENTITY_FILTER_DEFAULT | ENTITY_FILTER_ENEMY);
}

void TeleportPlayer(Player* player, vec3 position)
{
	SetCharacterControllerPosition(&player->controller, position);
	player->position = position;
}

static void OnDeath(Player* player)
{
	//SDL_assert(game->gameOverTimer == -1);
	//game->gameOverTimer = 0;
}

bool HitPlayer(Player* player, HitParams* hit, Entity* by)
{
	if (player->health <= 0)
		return false;
	if (player->lastHit && gameTime - player->lastHit < HIT_RECOVERY_DURATION)
		return false;

	SDL_assert(hit->damageType != DAMAGE_TYPE_NONE);

	float damage = hit->damage;
	if (player->blockItem)
	{
		const float blockStaminaCost = 0.2f;

		bool wasBlocked = player->stamina >= blockStaminaCost || player->parry;

		if (wasBlocked)
			damage = 0;

		if (!player->parry)
			player->stamina -= blockStaminaCost;

		Sound* sound = player->parry ? &game->hitParrySound : &game->hitBlockSound;
		PlaySound(sound, 0, 1);

		SDL_assert(hit->impulse.lengthSquared() != 0);

		mat4 weaponTransform = GetRightWeaponTransform(player);
		vec3 direction = weaponTransform.rotation().up();
		vec3 origin = weaponTransform.translation() + player->blockItem->weapon.damageRange.x * direction;

		float projectedHitDistance = dot(direction, hit->position - origin);
		projectedHitDistance = clamp(projectedHitDistance, player->blockItem->weapon.damageRange.x, player->blockItem->weapon.damageRange.y);
		vec3 projectedHitPosition = origin + direction * projectedHitDistance;

		ParticleEffect* hitParticles = (ParticleEffect*)CreateEntity();
		LoadParticleEffect(hitParticles, "effects/impact/spark.rfs", projectedHitPosition, quat::LookAt(hit->impulse, vec3::Up));
		hitParticles->parent = (Entity*)player;
		hitParticles->parentLocalTransform = ModelMatrix((Entity*)player).inverted() * ModelMatrix((Entity*)hitParticles);
		hitParticles->destroyOnFinish = true;

		player->lastBlockTime = gameTime;
		player->lastBlockParry = player->parry;
		player->lastBlockStagger = !wasBlocked;

		vec3 impulse = quat::FromAxisAngle(vec3::Up, player->yaw) * vec3(0, 0, 1) * 5;
		player->velocity += impulse;

		Action* currentAction = GetCurrentAction(player);
		SDL_assert(currentAction && currentAction->type == ACTION_TYPE_ATTACK);
		float recoverAnim = wasBlocked ? BLOCK_STAGGER_DURATION : GUARD_BREAK_STAGGER_DURATION;
		currentAction->followUpCancelTime = max(currentAction->followUpCancelTime, currentAction->elapsedTime + recoverAnim);

		hit->wasBlocked = wasBlocked;
		hit->wasParried = player->parry;
	}
	else
	{
		CancelAction(player->actions, *player);
		ClearQueuedAction(player->actions);

		Action staggerAction = {};
		InitStaggerAction(&staggerAction, 0.5f);
		QueueAction(player->actions, staggerAction, *player);
	}

	if (damage)
	{
		player->health -= (int)damage;
		player->lastHit = gameTime;

		if (player->health <= 0)
		{
			//

			OnDeath(player);
		}
		else
		{
			//
		}
	}

	return true;
}

void OnProjectileHit(Player* player, bool headshot)
{
	player->lastProjectileHit = gameTime;
	player->lastProjectileHitHeadshot = headshot;
}

bool GiveItem(Player* player, Item* item)
{
	for (int i = 0; i < NUM_LOADOUTS; i++)
	{
		if (!player->rightWeapons[i])
		{
			SetRightWeapon(player, i, item);
			return true;
		}
	}
	SDL_assert(player->rightWeapons[player->currentLoadout]);
	if (DropItem(player, player->rightWeapons[player->currentLoadout]))
	{
		SetRightWeapon(player, player->currentLoadout, item);
		return true;
	}
	return false;
}

bool DropItem(Player* player, Item* item)
{
	for (int i = 0; i < NUM_LOADOUTS; i++)
	{
		if (player->rightWeapons[i] == item)
		{
			ItemEntity* itemEntity = (ItemEntity*)CreateEntity();
			mat4 weaponTransform = GetRightWeaponTransform(player);
			vec3 position = weaponTransform.translation(); // game->cameraPosition + GetCameraRotation().forward();
			quat rotation = weaponTransform.rotation();
			InitItemEntity(itemEntity, item, position, rotation);

			vec3 velocity = GetCameraRotation(player).forward() * 5;
			vec3 angularVelocity = game->random.nextVector3(-2, 2);
			SetRigidBodyVelocity(&itemEntity->body, velocity, angularVelocity);

			SetRightWeapon(player, i, nullptr);

			return true;
		}
	}
	return false;
}

static bool ArmAnimChannelFilter(Node* node, bool* right)
{
	bool arm = StartsWith(node->name, "clavicle")
		|| StartsWith(node->name, "upperarm")
		|| StartsWith(node->name, "lowerarm")
		|| StartsWith(node->name, "hand")
		|| StartsWith(node->name, "thumb")
		|| StartsWith(node->name, "index")
		|| StartsWith(node->name, "middle")
		|| StartsWith(node->name, "ring")
		|| StartsWith(node->name, "pinky")
		|| StartsWith(node->name, "weapon");
	if (arm)
	{
		if (*right) return !EndsWith(node->name, "_l");
		else return EndsWith(node->name, "_l"); /*!EndsWith(node->name, "_r")*/;
	}
	return false;
}

static mat4 CalculateViewBobbing(Player* player, int side)
{
	vec3 sway = vec3::Zero;
	float yawSway = 0;
	float pitchSway = 0;
	float rollSway = 0;

	float swayScale = GetCurrentAction(player) ? GetCurrentAction(player)->idleAnimStrength : 1;

	// Idle animation
	float idleProgress = gameTime * PI * 2 / 8.0f;
	float idleAnimation = (SDL_cosf(idleProgress) * 0.5f - 0.5f) * 0.03f;
	sway.y += idleAnimation;

	float noiseProgress = gameTime * PI * 2 * (side == 0 ? 1 / 6.0f : 1.1f / 6.0f) + (side == 0 ? 0 : 100);
	vec3 noise = vec3(Simplex1f(noiseProgress * 0.2f), Simplex1f(-noiseProgress * 0.2f), Simplex1f(100 + noiseProgress * 0.2f)) * 0.015f * swayScale;
	sway += noise;

	// Walk animation
	vec2 viewmodelWalkAnim = vec2::Zero;
	viewmodelWalkAnim.x = 0.5f * SDL_sinf(player->distanceWalked * STEP_FREQUENCY * PI);
	viewmodelWalkAnim.y = 0.015f * -SDL_fabsf(SDL_cosf(player->distanceWalked * STEP_FREQUENCY * PI));
	//viewmodelWalkAnim *= 1 - Mathf.Smoothstep(1.0f, 1.5f, movementSpeed);
	viewmodelWalkAnim *= 1 - SDL_expf(-player->velocity.xz().length());
	//viewmodelWalkAnim *= (sprinting && runAnim.layers[1 + 0] != null && runAnim.layers[1 + 0].animationName == "run" || movement.isMoving && walkAnim.layers[1 + 0] != null && walkAnim.layers[1 + 0].animationName == "walk") ? 0 : 1;
	yawSway += viewmodelWalkAnim.x;
	sway.y += viewmodelWalkAnim.y;

	// Vertical speed animation
	float verticalSpeedAnimDst = player->velocity.y;
	verticalSpeedAnimDst = clamp(verticalSpeedAnimDst, -5.0f, 5.0f);
	player->viewBobVerticalSpeedAnim = mix(player->viewBobVerticalSpeedAnim, verticalSpeedAnimDst * 0.1f, 5 * deltaTime);
	pitchSway += player->viewBobVerticalSpeedAnim;

	// Land bob animation
	if (player->lastLandedTime)
	{
		float timeSinceLanding = gameTime - player->lastLandedTime;
		float landBob = (1.0f - SDL_powf(0.5f, timeSinceLanding * 4.0f)) * SDL_powf(0.1f, timeSinceLanding * 4.0f) * 0.5f;
		sway.y -= landBob;
	}

	if (player->lastBlockTime)
	{
		float timeSinceBlock = gameTime - player->lastBlockTime;
		float strength = player->lastBlockStagger ? 6.0f : 3.0f;
		float speed = player->lastBlockStagger ? 0.3f : 1.0f;
		float animation = (1.0f - SDL_powf(0.5f, timeSinceBlock * speed * 4.0f)) * SDL_powf(0.1f, timeSinceBlock * speed * 4.0f) * strength;
		sway.z -= animation;
		sway.y -= 0.5f * animation;
	}

	// Look sway
	float swayYawDst = player->yaw; // -0.0015f * Input.cursorMove.x;
	float swayPitchDst = player->pitch; // -0.0015f * Input.cursorMove.y;
	float swayRollDst = player->yaw; // -0.0015f * Input.cursorMove.x;
	player->viewBobLookSwayAnim = mix(player->viewBobLookSwayAnim, vec3(swayPitchDst, swayYawDst, swayRollDst), 1 - SDL_powf(0.5f, 5 * deltaTime));
	pitchSway -= player->viewBobLookSwayAnim.x - swayPitchDst;
	yawSway -= player->viewBobLookSwayAnim.y - swayYawDst;
	rollSway -= player->viewBobLookSwayAnim.z - swayRollDst;

	sway *= swayScale;
	pitchSway *= swayScale * 0.1f;
	yawSway *= swayScale * 0.1f;
	rollSway *= swayScale * 0.1f;

	return mat4::Translate(sway) * mat4::Rotate(vec3::Up, yawSway) * mat4::Rotate(vec3::Right, pitchSway) * mat4::Rotate(vec3::Back, rollSway);
}

static void AnimateAxisBlendSpace(Model* model, AnimationState* animationState, Player* player, AnimationPlayback* idleAnim, AnimationPlayback* forwardAnim, AnimationPlayback* sideAnim, float blend)
{
	/*
	if (player->ducked || player->duckTimer != -1)
	{
		if (player->moving)
		{
			vec3 fsu = quat::FromAxisAngle(vec3::Up, player->rotation).conjugated() * player->velocity;

			if (SDL_fabsf(fsu.z) > SDL_fabsf(fsu.x))
			{
				bodyMoveAnimation = &player->bodySneakAnim;
				bodyMoveAnimation->speed = fsu.z < 0 ? 1.0f : -1.0f;
			}
			else
			{
				bodyMoveAnimation = &player->bodySneakStrafeAnim;
				bodyMoveAnimation->speed = fsu.x > 0 ? 1.0f : -1.0f;
			}

			bodyMoveAnimation->speed *= fsu.length() / 3.0f * (49.0f / 24) * 1.5f;
		}
		else
		{
			bodyMoveAnimation = &player->bodyDuckAnim;
		}
	}
	else
	{
		if (player->moving)
		{
			vec3 fsu = quat::FromAxisAngle(vec3::Up, player->rotation).conjugated() * player->velocity;

			if (SDL_fabsf(fsu.z) > SDL_fabsf(fsu.x))
			{
				bodyMoveAnimation = &player->bodyRunAnim;
				bodyMoveAnimation->speed = fsu.z < 0 ? 1.0f : -1.0f;
			}
			else
			{
				bodyMoveAnimation = &player->bodyStrafeAnim;
				bodyMoveAnimation->speed = fsu.x > 0 ? 1.0f : -1.0f;
			}

			bodyMoveAnimation->speed *= fsu.length() / 3.0f;
		}
		else
		{
			bodyMoveAnimation = &player->bodyIdleAnim;
		}
	}
	*/



	SDL_assert(model->numNodes > 0);

	ClearHashMap(&animationState->channelMap);
	for (int i = 0; i < model->numNodes; i++)
	{
		Node* node = &model->nodes[i];
		int channelID = GetAnimationChannelWithName(idleAnim->animation, node->name);
		if (channelID != -1)
		{
			HashMapAdd(&animationState->channelMap, node, channelID);
		}
	}

	vec2 velocity = player->velocity.xz();
	float speed = velocity.length();

	for (int i = 0; i < animationState->channelMap.capacity; i++)
	{
		auto* slot = &animationState->channelMap.slots[i];
		if (slot->state == SLOT_USED)
		{
			Node* node = slot->key;
			int channelID = slot->value;
			const mat4& a = animationState->nodeTransforms[node->id];
			mat4 b;

			mat4 idle = AnimateNode(node, channelID, idleAnim->animation, idleAnim->timer, idleAnim->loop);
			mat4 forward = AnimateNode(node, channelID, forwardAnim->animation, forwardAnim->timer, forwardAnim->loop);
			mat4 side = AnimateNode(node, channelID, sideAnim->animation, sideAnim->timer, sideAnim->loop);

			float moveAmount = clamp(speed * 0.5f, 0, 1);

			if (moveAmount > 0)
			{
				vec2 dir = abs((velocity / speed).rotate(player->rotation));
				float t = dir.angle() / (PI * 0.5f);
				mat4 move = interpolate(side, forward, t);

				if (moveAmount == 1)
					b = move;
				else
					b = interpolate(idle, move, moveAmount);
			}
			else
			{
				b = idle;
			}

			if (blend < 1)
				animationState->nodeTransforms[node->id] = interpolate(a, b, blend);
			else
				animationState->nodeTransforms[node->id] = b;
		}
	}
}

static void UpdateRootMotion(Player* player)
{
	player->rootMotion = vec3::Zero;
	player->rootMotionAngle = 0;

	mat4& rootNodeTransform = GetNodeTransform(&player->anim, player->rootNode);
	vec3 rootMotionTranslation = rootNodeTransform.translation() - player->lastRootNodeTransform.translation();
	quat rootMotionRotation = player->lastRootNodeTransform.rotation().conjugated() * rootNodeTransform.rotation();
	player->lastRootNodeTransform = rootNodeTransform;
	rootNodeTransform = mat4::Identity;

	if (Action* currentAction = GetCurrentAction(player))
	{
		AnimationPlayback* actionAnimation = &currentAction->bodyAnim;

		if (currentAction->rootMotion && actionAnimation->animation == player->lastActionAnimation
			&& SDL_fmodf(actionAnimation->timer, actionAnimation->animation->duration) >= SDL_fmodf(player->lastActionAnimationTimer, actionAnimation->animation->duration))
		{
			vec3 rootMotion = quat::FromAxisAngle(vec3::Up, PI + player->rotation - player->currentRootMotionRotation) * rootMotionTranslation;
			float rootMotionAngle = rootMotionRotation.getAngle() * sign(rootMotionRotation.getAxis().y);

			float dt = gameTime - player->lastRootMotionUpdate;

			player->rootMotion = rootMotion;
			player->rootMotionAngle = rootMotionAngle;

			// we need this because the root motion translation direction should not be affected by a root motion rotation during this animation
			// since it is applied independently in blender. otherwise the translation will constantly change direction as the rotation is turning the character.
			// we may want to rotate the character manually which does affect translation direction
			player->currentRootMotionRotation += rootMotionAngle;
		}
		else
		{
			player->currentRootMotionRotation = 0;
		}

		player->lastActionAnimation = actionAnimation->animation;
		player->lastActionAnimationTimer = actionAnimation->timer;
	}

	player->lastRootMotionUpdate = gameTime;
}

void UpdatePlayer(Player* player)
{
	if (player->health <= 0)
	{
		game->cameraPosition.y = mix(game->cameraPosition.y, player->position.y + 0.2f, 5 * deltaTime);
		player->pitch = mix(player->pitch, 0.0f, 5 * deltaTime);
		return;
	}

	if (player->cameraMode == CAMERA_MODE_FIRST_PERSON)
	{
		if (game->mouseLocked)
		{
			player->yaw -= app->mouseDelta.x * 0.001f;
			player->pitch -= app->mouseDelta.y * 0.001f;

			player->pitch = clamp(player->pitch, -0.5f * PI, 0.5f * PI);
		}

		player->upperBodyTurn = player->yaw - player->rotation;

		if (!player->moving && SDL_fabsf(player->upperBodyTurn) > 0.5f * PI)
		{
			player->resetUpperBodyTurn = true;

			if (!GetCurrentAction(player))
			{
				Action action = {};
				InitTurnAction(&action, sign(player->yaw - player->rotation));
				QueueAction(player->actions, action, *player);
			}
		}

		if (player->moving || GetCurrentAction(player) && GetCurrentAction(player)->lockPlayerRotation || player->resetUpperBodyTurn)
		{
			player->upperBodyTurn = moveTowardsAngle(player->upperBodyTurn, 0.0f, 0.5f * PI * 4 * deltaTime);
			player->rotation = player->yaw - player->upperBodyTurn;

			if (player->resetUpperBodyTurn && SDL_fabsf(player->upperBodyTurn) < 0.1f)
				player->resetUpperBodyTurn = false;
		}

		/*
		bool lockTurn = player->upperBodyTurn < 0.1f && (player->resetUpperBodyTurn || player->moving);
		if (lockTurn)
		{
			player->upperBodyTurn = 0;
			player->resetUpperBodyTurn = false;
			player->rotation = player->yaw;
		}
		else
		{

		}

		if (!player->moving && SDL_fabsf(player->upperBodyTurn) > 0.5f * PI && !(GetCurrentAction(player) && GetCurrentAction(player)->lockPlayerRotation))
		{
			if (!GetCurrentAction(player))
				; // turn animation

			player->resetUpperBodyTurn = true;
			player->upperBodyTurn = player->yaw - player->rotation;
		}
		*/

		//if (!(GetCurrentAction(player) && GetCurrentAction(player)->lockPlayerRotation))
		//	player->rotation = player->yaw;

		if (!(GetCurrentAction(player) && GetCurrentAction(player)->fullBodyAnim))
		{
			SourceMovement(player, player->rootMotion);
			player->rootMotion = vec3::Zero;

			player->cameraHeight = player->ducked ? CAMERA_HEIGHT_DUCKED :
				player->duckTimer != -1 ? min(player->cameraHeight, mix(CAMERA_HEIGHT, CAMERA_HEIGHT_DUCKED, player->duckTimer / DUCK_TRANSITION)) :
				player->grounded ? mix(player->cameraHeight, CAMERA_HEIGHT, 10 * deltaTime) :
				CAMERA_HEIGHT;
		}
	}

	((EntityBase*)player)->rotation = quat::FromAxisAngle(vec3::Up, player->rotation);

	for (int i = 0; i < NUM_LOADOUTS; i++)
	{
		if (GetKeyDown((SDL_Scancode)(SDL_SCANCODE_1 + i)))
		{
			if (GetCurrentAction(player) && player->actions.actions.back()->type == ACTION_TYPE_EQUIP)
				CancelAction(player->actions, *player);
			SwitchLoadout(player, i);
			break;
		}
	}

	if (GetKeyDown(SDL_SCANCODE_G) && GetRightWeapon(player))
	{
		Action dropAction = {};
		InitDropAction(&dropAction);
		QueueAction(player->actions, dropAction, *player);
	}

	{
		player->interactTarget = nullptr;

		quat cameraRotation = GetCameraRotation(player);
		PhysicsHit hits[16];
		int numHits = Raycast(game->cameraPosition, cameraRotation.forward(), PLAYER_REACH, hits, 16, ENTITY_FILTER_INTERACTABLE);
		for (int i = 0; i < numHits; i++)
		{
			PhysicsHit* hit = &hits[i];
			if (Entity* entity = (Entity*)hit->body->userPtr)
			{
				player->interactTarget = entity;
				break;
			}
		}

		if (player->interactTarget)
		{
			if (!GetCurrentAction(player) && GetKeyDown(SDL_SCANCODE_E))
			{
				InteractEntity(player->interactTarget, (Entity*)player);
			}
		}
	}

	{
		Item* right = GetRightApparentWeapon(player);
		bool offHand = right != GetRightWeapon(player);
		if (GetMouseButtonDown(SDL_BUTTON_LEFT) && right && !offHand && player->stamina > 0)
		{
			if (player->actions.actions.size < player->actions.actions.capacity /* !currentAction || currentAction->elapsedTime > currentAction->followUpCancelTime*/)
			{
				Attack* nextAttack = nullptr;
				int attackIdx = 0;

				Action* currentAction = GetCurrentAction(player);
				if (currentAction && currentAction->type == ACTION_TYPE_ATTACK && currentAction->attack.weapon == right && currentAction->attack.attack->followUp)
				{
					nextAttack = GetAttackByName(currentAction->attack.weapon, currentAction->attack.attack->followUp);
					attackIdx = currentAction->attack.attackIdx + 1;
				}
				else if (player->lastBlockTime && gameTime - player->lastBlockTime < 0.5f && player->lastBlockParry && right->weapon.riposteAttack != -1)
				{
					nextAttack = &right->weapon.attacks[right->weapon.riposteAttack];
					CancelAction(player->actions, *player);
				}
				else if (player->sprinting && right->weapon.runningAttack != -1)
				{
					nextAttack = &right->weapon.attacks[right->weapon.runningAttack];
				}
				else
				{
					nextAttack = GetFirstAttack(right, false);
				}

				if (nextAttack)
				{
					Action action;
					InitAttackAction(&action, right, nextAttack, attackIdx, SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT);
					QueueAction(player->actions, action, *player);
				}
			}
		}
		if (GetMouseButtonDown(SDL_BUTTON_RIGHT) && !offHand && player->stamina > 0)
		{
			if (player->actions.actions.size < player->actions.actions.capacity /* !currentAction || currentAction->elapsedTime > currentAction->followUpCancelTime*/)
			{
				if (Attack* nextAttack = GetFirstAttack(right, true))
				{
					int attackIdx = 0;

					Action action;
					InitAttackAction(&action, right, nextAttack, attackIdx, SDL_BUTTON_RIGHT, SDL_BUTTON_LEFT);
					QueueAction(player->actions, action, *player);
				}
			}
		}
	}

	UpdateActionManager(player->actions, *player);

	AnimationPlayback* moveAnimation = &player->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(player->model, &player->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop, nullptr, nullptr);

	AnimationPlayback* bodyMoveAnimation = nullptr;

	if (player->grounded)
	{
		if (player->ducked || player->duckTimer != -1)
		{
			bodyMoveAnimation = &player->bodyDuckAnim;

			vec3 fsu = quat::FromAxisAngle(vec3::Up, player->yaw).conjugated() * player->velocity;
			player->bodySneakAnim.speed = (fsu.z < 0 ? 1.0f : -1.0f) * fsu.length() / 3.0f * (49.0f / 24) * 1.5f;
			player->bodySneakStrafeAnim.speed = (fsu.x > 0 ? 1.0f : -1.0f) * fsu.length() / 3.0f * (49.0f / 24) * 1.5f;

			player->bodySneakAnim.timer += deltaTime * player->bodySneakAnim.speed;
			player->bodySneakStrafeAnim.timer += deltaTime * player->bodySneakStrafeAnim.speed;
		}
		else
		{
			bodyMoveAnimation = &player->bodyIdleAnim;

			vec3 fsu = quat::FromAxisAngle(vec3::Up, player->yaw).conjugated() * player->velocity;
			player->bodyRunAnim.speed = (fsu.z < 0 ? 1.0f : -1.0f) * fsu.length() / 3.0f;
			player->bodyStrafeAnim.speed = (fsu.x > 0 ? 1.0f : -1.0f) * fsu.length() / 3.0f;

			player->bodyRunAnim.timer += deltaTime * player->bodyRunAnim.speed;
			player->bodyStrafeAnim.timer += deltaTime * player->bodyStrafeAnim.speed;
		}
	}
	else
	{
		if (player->ducked || player->duckTimer != -1)
			bodyMoveAnimation = &player->bodyFallDuckAnim;
		else
			bodyMoveAnimation = &player->bodyFallAnim;

		bodyMoveAnimation->timer += deltaTime * bodyMoveAnimation->speed;
	}

	Animation* rightAnimation = moveAnimation->animation;
	float rightAnimationTimer = moveAnimation->timer;
	bool rightAnimationLoop = moveAnimation->loop;
	float rightAnimationBlendDuration = 0.2f;

	Animation* leftAnimation = moveAnimation->animation;
	float leftAnimationTimer = moveAnimation->timer;
	bool leftAnimationLoop = moveAnimation->loop;
	float leftAnimationBlendDuration = 0.2f;

	Animation* bodyAnimation = bodyMoveAnimation->animation;
	float bodyAnimationTimer = bodyMoveAnimation->timer;
	bool bodyAnimationLoop = bodyMoveAnimation->loop;
	float bodyAnimationBlendDuration = 0.2f;

	Item* right = GetRightApparentWeapon(player);
	Item* left = GetLeftApparentWeapon(player);

	if (right)
	{
		rightAnimation = GetAnimationByName(&right->moveset, "idle");
		SDL_assert(rightAnimation);
	}

	if (left)
	{
		leftAnimation = GetAnimationByName(&left->moveset, "idle");
		SDL_assert(leftAnimation);
	}

	if (GetCurrentAction(player))
	{
		Action* currentAction = GetCurrentAction(player);
		if (currentAction->rightAnim.animation)
		{
			rightAnimation = currentAction->rightAnim.animation;
			rightAnimationTimer = currentAction->elapsedTime;
			rightAnimationLoop = false;
			rightAnimationBlendDuration = currentAction->rightAnimBlendDuration;
		}
		if (currentAction->leftAnim.animation)
		{
			leftAnimation = currentAction->leftAnim.animation;
			leftAnimationTimer = currentAction->elapsedTime;
			leftAnimationLoop = false;
			leftAnimationBlendDuration = currentAction->leftAnimBlendDuration;
		}
		if (currentAction->bodyAnim.animation)
		{
			bodyAnimation = currentAction->bodyAnim.animation;
			bodyAnimationTimer = currentAction->elapsedTime;
			bodyAnimationLoop = false;
			bodyAnimationBlendDuration = currentAction->bodyAnimBlendDuration;
		}

		/*
		AnimationPlayback* actionAnimation = &currentAction->rightAnim;
		//actionAnimation->timer += deltaTime * actionAnimation->speed;
		actionAnimation->timer = currentAction->elapsedTime;

		if (currentAction->twoHanded)
		{
			AnimateModel(player->model, &player->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, nullptr, nullptr);
		}
		else
		{
			bool right = true;
			AnimateModel(player->model, &player->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
		}
		*/
	}

	if (bodyAnimation)
	{
		if (bodyAnimation == player->bodyIdleAnim.animation)
			AnimateAxisBlendSpace(player->bodyModel, &player->bodyAnim, player, &player->bodyIdleAnim, &player->bodyRunAnim, &player->bodyStrafeAnim, 1);
		else if (bodyAnimation == player->bodyDuckAnim.animation)
			AnimateAxisBlendSpace(player->bodyModel, &player->bodyAnim, player, &player->bodyDuckAnim, &player->bodySneakAnim, &player->bodySneakStrafeAnim, 1);
		else
			AnimateModel(player->bodyModel, &player->bodyAnim, bodyAnimation, bodyAnimationTimer, bodyAnimationLoop, nullptr, nullptr);

		//AnimateModel(player->bodyModel, &player->bodyAnim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop, nullptr, nullptr);

		if (bodyAnimation != player->lastBodyAnim && player->lastBodyAnim)
		{
			player->bodyBlendStart = gameTime;
			player->bodyBlendAnim = player->lastBodyAnim;
			player->bodyBlendAnimTimer = player->lastBodyAnimTimer;
			player->bodyBlendAnimLoop = player->lastBodyAnimLoop;
			player->bodyBlendDuration = bodyAnimationBlendDuration;
		}

		if (player->bodyBlendStart)
		{
			float blendProgress = player->bodyBlendDuration ? (gameTime - player->bodyBlendStart) / player->bodyBlendDuration : 1;
			if (blendProgress >= 1)
			{
				player->bodyBlendStart = 0;
			}
			else
			{
				if ((bodyAnimation == player->bodyDuckAnim.animation || bodyAnimation == player->bodySneakAnim.animation || bodyAnimation == player->bodyFallDuckAnim.animation) &&
					(player->bodyBlendAnim == player->bodyIdleAnim.animation || player->bodyBlendAnim == player->bodyRunAnim.animation || player->bodyBlendAnim == player->bodyFallAnim.animation))
					blendProgress = remap(player->cameraHeight, CAMERA_HEIGHT, CAMERA_HEIGHT_DUCKED, 0, 1);
				else if ((bodyAnimation == player->bodyIdleAnim.animation || bodyAnimation == player->bodyRunAnim.animation || bodyAnimation == player->bodyFallAnim.animation) &&
					(player->bodyBlendAnim == player->bodyDuckAnim.animation || player->bodyBlendAnim == player->bodySneakAnim.animation || player->bodyBlendAnim == player->bodyFallDuckAnim.animation))
					blendProgress = remap(player->cameraHeight, CAMERA_HEIGHT, CAMERA_HEIGHT_DUCKED, 1, 0);

				if (player->bodyBlendAnim == player->bodyIdleAnim.animation)
					AnimateAxisBlendSpace(player->bodyModel, &player->bodyAnim, player, &player->bodyIdleAnim, &player->bodyRunAnim, &player->bodyStrafeAnim, 1 - blendProgress);
				else if (player->bodyBlendAnim == player->bodyDuckAnim.animation)
					AnimateAxisBlendSpace(player->bodyModel, &player->bodyAnim, player, &player->bodyDuckAnim, &player->bodySneakAnim, &player->bodySneakStrafeAnim, 1 - blendProgress);
				else
					BlendAnimation(player->bodyModel, &player->bodyAnim, player->bodyBlendAnim, player->bodyBlendAnimTimer, player->bodyBlendAnimLoop, 1 - blendProgress, nullptr, nullptr);
			}
		}

		player->lastBodyAnim = bodyAnimation;
		player->lastBodyAnimTimer = bodyAnimationTimer;
		player->lastBodyAnimLoop = bodyAnimationLoop;
	}
	if (rightAnimation)
	{
		bool right = true;
		//AnimateModel(player->model, &player->anim, rightAnimation, rightAnimationTimer, rightAnimationLoop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

		AnimateModel(player->bodyModel, &player->bodyAnim, rightAnimation, rightAnimationTimer, rightAnimationLoop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

		if (rightAnimation != player->lastRightAnim && player->lastRightAnim)
		{
			player->rightBlendStart = gameTime;
			player->rightBlendAnim = player->lastRightAnim;
			player->rightBlendAnimTimer = player->lastRightAnimTimer;
			player->rightBlendAnimLoop = player->lastRightAnimLoop;
			player->rightBlendDuration = rightAnimationBlendDuration;
		}

		if (player->rightBlendStart)
		{
			float blendProgress = player->rightBlendDuration ? (gameTime - player->rightBlendStart) / player->rightBlendDuration : 1;
			if (blendProgress >= 1)
			{
				player->rightBlendStart = 0;
			}
			else
			{
				//BlendAnimation(player->model, &player->anim, player->rightBlendAnim, player->rightBlendAnimTimer, player->rightBlendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

				BlendAnimation(player->bodyModel, &player->bodyAnim, player->rightBlendAnim, player->rightBlendAnimTimer, player->rightBlendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
			}
		}

		player->lastRightAnim = rightAnimation;
		player->lastRightAnimTimer = rightAnimationTimer;
		player->lastRightAnimLoop = rightAnimationLoop;
	}
	if (leftAnimation)
	{
		bool right = false;
		//AnimateModel(player->model, &player->anim, leftAnimation, leftAnimationTimer, leftAnimationLoop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

		AnimateModel(player->bodyModel, &player->bodyAnim, leftAnimation, leftAnimationTimer, leftAnimationLoop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

		if (leftAnimation != player->lastLeftAnim && player->lastLeftAnim)
		{
			player->leftBlendStart = gameTime;
			player->leftBlendAnim = player->lastLeftAnim;
			player->leftBlendAnimTimer = player->lastLeftAnimTimer;
			player->leftBlendAnimLoop = player->lastLeftAnimLoop;
			player->leftBlendDuration = leftAnimationBlendDuration;
		}

		if (player->leftBlendStart)
		{
			float blendProgress = player->leftBlendDuration ? (gameTime - player->leftBlendStart) / player->leftBlendDuration : 1;
			if (blendProgress > 1)
			{
				player->leftBlendStart = 0;
			}
			else
			{
				//BlendAnimation(player->model, &player->anim, player->leftBlendAnim, player->leftBlendAnimTimer, player->leftBlendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

				BlendAnimation(player->bodyModel, &player->bodyAnim, player->leftBlendAnim, player->leftBlendAnimTimer, player->leftBlendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
			}
		}

		player->lastLeftAnim = leftAnimation;
		player->lastLeftAnimTimer = leftAnimationTimer;
		player->lastLeftAnimLoop = leftAnimationLoop;
	}

	bool proceduralViewmodelAnim = !(GetCurrentAction(player) && GetCurrentAction(player)->fullBodyAnim);
	if (proceduralViewmodelAnim)
	{
		mat4 rightViewBob = CalculateViewBobbing(player, 0);
		mat4 leftViewBob = CalculateViewBobbing(player, GetLeftApparentWeapon(player) == GetLeftWeapon(player) ? 1 : 0);

		mat4& clavicleRightTransform = GetNodeTransform(&player->bodyAnim, player->rightShoulderNode);
		clavicleRightTransform = rightViewBob * clavicleRightTransform;

		mat4& clavicleLeftTransform = GetNodeTransform(&player->bodyAnim, player->leftShoulderNode);
		clavicleLeftTransform = leftViewBob * clavicleLeftTransform;

		mat4& rightWeaponTransform = GetNodeTransform(&player->bodyAnim, player->rightWeaponNode);
		rightWeaponTransform = rightViewBob * rightWeaponTransform;

		mat4& leftWeaponTransform = GetNodeTransform(&player->bodyAnim, player->leftWeaponNode);
		leftWeaponTransform = leftViewBob * leftWeaponTransform;
	}

	{
		// this translates the nodes from being relative to the viewmodel origin to being relative to the neck node, where the camera is positioned.
		// this difference is in global space, so we have to transform it to spine03 local space before applying it to the node transforms.
		vec3 neckViewmodelDifference = vec3(0, 0.0573f - (1.50345f - 1.4283f), -0.027168f - (0.014949f - 0.027168f))
			+ vec3(0, 0.03f, -0.03f); // correction factor to make it look closer to blender view

		// this transforms the aforementioned translation to being local to the spine node.
		mat4 inverseSpineRotation = mat4::Rotate(vec3::AxisX, 9.175f * Deg2Rad);

		mat4 viewmodelToBodymodel = inverseSpineRotation * mat4::Translate(GetNodeTransform(&player->bodyAnim, player->neckNode).translation() + neckViewmodelDifference);

		mat4& clavicleRightTransform = GetNodeTransform(&player->bodyAnim, player->rightShoulderNode);
		clavicleRightTransform = viewmodelToBodymodel * clavicleRightTransform;

		mat4& clavicleLeftTransform = GetNodeTransform(&player->bodyAnim, player->leftShoulderNode);
		clavicleLeftTransform = viewmodelToBodymodel * clavicleLeftTransform;

		mat4& rightWeaponTransform = GetNodeTransform(&player->bodyAnim, player->rightWeaponNode);
		rightWeaponTransform = viewmodelToBodymodel * rightWeaponTransform;

		mat4& leftWeaponTransform = GetNodeTransform(&player->bodyAnim, player->leftWeaponNode);
		leftWeaponTransform = viewmodelToBodymodel * leftWeaponTransform;
	}

	{
		mat4& spineTransform = GetNodeTransform(&player->bodyAnim, player->spineNode);
		spineTransform = mat4::Transform(spineTransform.translation() + vec3(0, max(SDL_sinf(-player->pitch), 0.0f) * 0.2f, 0), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * spineTransform.rotation());

		mat4& spine2Transform = GetNodeTransform(&player->bodyAnim, player->spine2Node);
		spine2Transform = mat4::Transform(spine2Transform.translation(), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * spine2Transform.rotation());

		mat4& neckTransform = GetNodeTransform(&player->bodyAnim, player->neckNode);
		neckTransform = mat4::Transform(neckTransform.translation(), neckTransform.rotation(), vec3(0.01f));

		mat4& spine1Transform = GetNodeTransform(&player->bodyAnim, GetNodeByName(player->bodyModel, "spine_01"));
		vec3 localUp = (CalculateNodeWorldTransform(&player->bodyAnim, player->pelvisNode).inverted() * vec4(0, 1, 0, 0)).xyz;
		spine1Transform = mat4::Rotate(localUp, player->yaw - player->rotation) * spine1Transform;

		/*
		mat4& neckTransform = GetNodeTransform(&player->bodyAnim, player->neckNode);
		neckTransform = mat4::Transform(neckTransform.translation(), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * neckTransform.rotation());
		//neckTransform = mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * neckTransform;
		neckTransform = mat4::Transform(neckTransform.translation(), neckTransform.rotation(), vec3(0.01f));

		mat4& clavicleRightTransform = GetNodeTransform(&player->bodyAnim, player->rightShoulderNode);
		//clavicleRightTransform = mat4::Transform(clavicleRightTransform.translation(), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * clavicleRightTransform.rotation());
		//clavicleRightTransform = mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * clavicleRightTransform;
		clavicleRightTransform = neckTransform * mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * neckTransform.inverted() * clavicleRightTransform;

		mat4& clavicleLeftTransform = GetNodeTransform(&player->bodyAnim, player->leftShoulderNode);
		//clavicleLeftTransform = mat4::Transform(clavicleLeftTransform.translation(), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * clavicleLeftTransform.rotation());
		//clavicleLeftTransform = mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * clavicleLeftTransform;
		clavicleLeftTransform = neckTransform * mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * neckTransform.inverted() * clavicleLeftTransform;

		mat4& weaponRightTransform = GetNodeTransform(&player->bodyAnim, player->rightWeaponNode);
		//weaponRightTransform = mat4::Transform(weaponRightTransform.translation(), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * weaponRightTransform.rotation());
		//weaponRightTransform = mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * weaponRightTransform;
		weaponRightTransform = neckTransform * mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * neckTransform.inverted() * weaponRightTransform;

		mat4& weaponLeftTransform = GetNodeTransform(&player->bodyAnim, player->leftWeaponNode);
		//weaponLeftTransform = mat4::Transform(weaponLeftTransform.translation(), quat::FromAxisAngle(vec3::AxisX, -player->pitch * 0.5f) * weaponLeftTransform.rotation());
		//weaponLeftTransform = mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * weaponLeftTransform;
		weaponLeftTransform = neckTransform * mat4::Rotate(vec3::AxisX, -player->pitch * 0.5f) * neckTransform.inverted() * weaponLeftTransform;
		*/
	}

	UpdateRootMotion(player);

	Item* rightWeapon = GetRightWeapon(player);
	if (rightWeapon && rightWeapon->model.numAnimations)
	{
		Action* currentAction = GetCurrentAction(player);
		if (currentAction && currentAction->rightItemAnimName)
		{
			currentAction->rightWeaponAnim.timer += deltaTime;
			AnimateModel(&rightWeapon->model, &player->rightWeaponAnim, currentAction->rightWeaponAnim.animation, currentAction->rightWeaponAnim.timer, currentAction->rightWeaponAnim.loop, nullptr, nullptr);
		}
		else if (Animation* defaultAnim = GetAnimationByName(&rightWeapon->model, "item_default"))
		{
			AnimateModel(&rightWeapon->model, &player->rightWeaponAnim, defaultAnim, gameTime, true, nullptr, nullptr);
		}

		ApplyAnimationToSkeleton(&rightWeapon->model, &player->rightWeaponAnim);
	}

	ApplyAnimationToSkeleton(player->model, &player->anim);
	ApplyAnimationToSkeleton(player->bodyModel, &player->bodyAnim);

	if (GetKeyDown(SDL_SCANCODE_F5))
		player->cameraMode = player->cameraMode == CAMERA_MODE_FIRST_PERSON ? CAMERA_MODE_FREE : CAMERA_MODE_FIRST_PERSON;

	if (player->cameraMode == CAMERA_MODE_FIRST_PERSON)
	{
		if (!(GetCurrentAction(player) && GetCurrentAction(player)->fullBodyAnim))
		{
			game->cameraPosition = player->position + vec3::Up * player->cameraHeight;

			float landBob = 0.0f;
			if (player->lastLandedTime)
			{
				float timeSinceLanding = gameTime - player->lastLandedTime;
				landBob = (1.0f - SDL_powf(0.5f, timeSinceLanding * 4.0f)) * SDL_powf(0.1f, timeSinceLanding * 4.0f) * 5.5f;
			}
			game->cameraPosition.y -= landBob;

			game->cameraRotation = quat::FromAxisAngle(vec3::Up, player->yaw) * quat::FromAxisAngle(vec3::Right, player->pitch);

			//if (GetKey(SDL_SCANCODE_O))
			{
				game->cameraPosition = player->position + quat::FromAxisAngle(vec3::Up, player->rotation + PI) * (GetNodeTransform(&player->bodyAnim, player->neckNode).translation() + vec3(0, 0, 0.03f));
				game->cameraRotation = GetNodeTransform(&player->bodyAnim, player->neckNode).rotation();
				game->cameraRotation = quat::Identity;
				game->cameraRotation = quat::FromAxisAngle(vec3::Up, player->yaw) * quat::FromAxisAngle(vec3::Right, player->pitch);
			}
		}
		else
		{
			mat4 cameraNodeTransform = GetNodeTransform(&player->bodyAnim, player->neckNode);
			game->cameraPosition = player->position + quat::FromAxisAngle(vec3::Up, player->rotation) * cameraNodeTransform.translation();
			//game->cameraRotation = quat::FromAxisAngle(vec3::Up, PI) * cameraNodeTransform.rotation();
			game->cameraRotation = quat::FromAxisAngle(vec3::Up, player->rotation) * quat::FromAxisAngle(vec3::Right, player->pitch) * quat::FromAxisAngle(vec3::Up, player->yaw - player->rotation);
		}

		SetAudioListener(game->cameraPosition, game->cameraRotation);

		if (player->health < player->maxHealth && player->lastHit && gameTime - player->lastHit > HEALTH_REGEN_HIT_DELAY)
		{
			if (EveryInterval(0.1f, hash(player)))
				player->health++;
		}

		if (player->stamina <= 0 && !player->exhausted)
		{
			player->exhausted = true;
			PlaySound(&game->exhaustedSound, 0.4f);
		}
		else if (player->stamina >= 0.3f)
		{
			player->exhausted = false;
		}

		if (player->stamina < 1.0f && player->actions.actions.size == 0 && !player->sprinting && !player->blockItem)
		{
			player->stamina = min(player->stamina + 0.15f * deltaTime, 1.0f);
		}
	}
	else
	{
		if (game->mouseLocked)
		{
			player->yaw -= app->mouseDelta.x * 0.001f;
			player->pitch -= app->mouseDelta.y * 0.001f;

			player->pitch = clamp(player->pitch, -0.5f * PI, 0.5f * PI);
		}

		quat cameraRotation = quat::FromAxisAngle(vec3::Up, player->yaw) * quat::FromAxisAngle(vec3::Right, player->pitch);
		game->cameraRotation = cameraRotation;

		vec3 delta = vec3::Zero;
		if (GetKey(SDL_SCANCODE_A)) delta += cameraRotation.left();
		if (GetKey(SDL_SCANCODE_D)) delta += cameraRotation.right();
		if (GetKey(SDL_SCANCODE_S)) delta += cameraRotation.back();
		if (GetKey(SDL_SCANCODE_W)) delta += cameraRotation.forward();
		if (GetKey(SDL_SCANCODE_SPACE)) delta += vec3::Up;
		if (GetKey(SDL_SCANCODE_LCTRL)) delta += vec3::Down;

		if (delta.lengthSquared() > 0)
		{
			float speed = (GetKey(SDL_SCANCODE_LSHIFT) ? 50 : GetKey(SDL_SCANCODE_LALT) ? 1 : 5) * player->walkSpeed;
			vec3 velocity = delta.normalized() * speed;
			vec3 displacement = velocity * deltaTime;
			game->cameraPosition += displacement;
		}

		player->moving = delta.lengthSquared() > 0;
	}
}

void RenderPlayer(Player* player)
{
	mat4 bodyTransform = mat4::Translate(player->position) * mat4::Rotate(vec3::Up, player->rotation + PI);
	mat4 scaleToCamera = game->view.inverted() * mat4::Scale(0.5f) * game->view;
	bodyTransform = scaleToCamera * bodyTransform;
	RenderModel(&game->renderer, player->bodyModel, &player->bodyAnim, bodyTransform);

	//RenderModel(&game->renderer, player->bodyModel, &player->bodyAnim, mat4::Translate(-1, 0, -1) * mat4::Rotate(vec3::Up, player->rotation));

	//mat4 cameraTransform = GetCameraTransform(player);

	mat4 viewmodelTransform = mat4::Translate(player->position) * mat4::Rotate(vec3::Up, player->rotation + PI);

	/*
	if (!(GetCurrentAction(player) && GetCurrentAction(player)->fullBodyAnim))
	{
		viewmodelTransform = cameraTransform * mat4::Rotate(vec3::Up, PI);
	}
	else
	{
		viewmodelTransform = bodyTransform * GetNodeTransform(&player->bodyAnim, player->bodyCameraNode);
	}

	RenderModel(&game->renderer, player->model, &player->anim, viewmodelTransform);
	*/

	Item* rightWeapon = GetRightWeapon(player);
	Item* leftWeapon = GetLeftWeapon(player);

	Action* currentAction = GetCurrentAction(player);
	if (currentAction)
	{
		if (currentAction->overrideRightWeapon)
			rightWeapon = currentAction->rightWeapon;
		if (currentAction->overrideLeftWeapon)
			leftWeapon = currentAction->leftWeapon;
	}

	if (rightWeapon)
	{
		mat4 weaponTransform = viewmodelTransform * GetNodeTransform(&player->bodyAnim, player->rightWeaponNode);
		weaponTransform = scaleToCamera * weaponTransform;
		RenderModel(&game->renderer, &rightWeapon->model, rightWeapon->model.numAnimations ? &player->rightWeaponAnim : nullptr, weaponTransform);
	}
	if (leftWeapon)
	{
		mat4 weaponTransform = viewmodelTransform * GetNodeTransform(&player->bodyAnim, player->leftWeaponNode);
		weaponTransform = scaleToCamera * weaponTransform;
		RenderModel(&game->renderer, &leftWeapon->model, nullptr, weaponTransform);
	}

	// exhaustion vignette
	{
		float vignetteStrength = player->exhausted ? max(0.3f - player->stamina, 0.0f) / 0.3f : SDL_powf(clamp(remap(player->stamina, 0.3f, 0, 0, 1), 0, 1), 3);
		vignetteStrength *= 0.5f;

		vec3 color = ARGBToVector(0xFF6090FF).rgb;

		GUIPanel(0, 0, app->width, app->height, game->vignette, vec4(color, vignetteStrength));
	}
	// health vignette
	{
		float vignetteStrength = max(0.5f - player->health / (float)player->maxHealth, 0.0f) * 2;
		if (player->lastHit && gameTime - player->lastHit < 5)
			vignetteStrength += SDL_expf(-(gameTime - player->lastHit) * 2) * 0.5f;
		vignetteStrength = min(vignetteStrength, 1.0f);
		vignetteStrength *= 0.5f;

		vec3 color = vec3(1, 0, 0);

		GUIPanel(0, 0, app->width, app->height, game->vignette, vec4(color, vignetteStrength));
	}

	// stamina
	if (player->stamina < 1)
	{
		int w = app->width / 2;
		int h = 5;
		int x = app->width / 2 - w / 2;
		int y = app->height - 20;
		GUIPanel(x, y, w, h, vec4(0.1f, 0.1f, 0.1f, 0.5f));

		vec4 color = player->exhausted ? mix(vec4(1), vec4(1, 0, 0, 1), SDL_sinf(gameTime * 7) * 0.5f + 0.5f) : vec4(1);

		GUIPanel(x, y, (int)(w * max(player->stamina, 0.0f)), h, color);
	}

	// crosshair
	{
		if (player->interactTarget)
		{
			GUIPanel(app->width / 2 - game->crosshairInteract->info.width / 4, app->height / 2 - game->crosshairInteract->info.height / 4, game->crosshairInteract->info.width / 2, game->crosshairInteract->info.height / 2, game->crosshairInteract, vec4::One);
		}
		else
		{
			//GUIPanel(app->width / 2 - game->crosshair->info.width / 2, app->height / 2 - game->crosshair->info.height / 2, game->crosshair);
		}
	}

	// hitmarker
	{
		float showDuration = 0.2f;
		if (player->lastProjectileHit && gameTime - player->lastProjectileHit < showDuration)
		{
			float progress = (gameTime - player->lastProjectileHit) / showDuration;
			ivec2 size = (ivec2)(mix(1.0f, 1.5f, progress) * vec2((float)game->hitmarker->info.width, (float)game->hitmarker->info.height));
			int x = app->width / 2 - size.x / 2;
			int y = app->height / 2 - size.y / 2;

			vec4 color = vec4(1);
			if (player->lastProjectileHitHeadshot)
				color.rgb = vec3(1, 0.4f, 0.4f);
			color.a = 1 - progress * progress;

			GUIPanel(x, y, size.x, size.y, game->hitmarker, color);
		}
	}

	// blockmarker
	{
		float showDuration = 0.2f;
		if (player->lastBlockTime && gameTime - player->lastBlockTime < showDuration)
		{
			float progress = (gameTime - player->lastBlockTime) / showDuration;
			ivec2 size = (ivec2)(mix(0.5f, 0.75f, progress) * vec2((float)game->hitmarker->info.width, (float)game->hitmarker->info.height));
			int x = app->width / 2 - size.x / 2;
			int y = app->height / 2 - size.y / 2;

			vec4 color = vec4(1);
			if (player->lastBlockParry)
				color.rgb = vec3(1, 0.4f, 0.4f);
			color.a = 1 - progress * progress;

			GUIPanel(x, y, size.x, size.y, game->blockmarker, color);
		}
	}
}
