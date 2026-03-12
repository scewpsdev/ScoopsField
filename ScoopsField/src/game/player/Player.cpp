#include "Player.h"


#define HEALTH_REGEN_HIT_DELAY 5.0f
#define HIT_RECOVERY_DURATION 0.25f
#define STEP_FREQUENCY 0.6f


static Action* GetCurrentAction(Player* player)
{
	return QueuePeek(player->actions.actions);
}

static void SetRightWeapon(Player* player, int loadout, Item* weapon)
{
	if (player->rightWeapons[loadout] != weapon)
	{
		player->rightWeapons[loadout] = weapon;

		if (loadout == player->currentLoadout && weapon)
		{
			Action action;
			InitEquipAction(&action, weapon);
			QueueAction(player->actions, action, *player);
		}
	}
}

static void SetLeftWeapon(Player* player, int loadout, Item* weapon)
{
	if (player->leftWeapons[loadout] != weapon)
	{
		player->leftWeapons[loadout] = weapon;

		if (loadout == player->currentLoadout && weapon)
		{
			Action action;
			InitEquipAction(&action, weapon);
			QueueAction(player->actions, action, *player);
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

mat4 GetRightWeaponTransform(Player* player)
{
	mat4 cameraTransform = mat4::Translate(game->cameraPosition) * mat4::Rotate(quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch));
	mat4 viewmodelTransform = cameraTransform * mat4::Rotate(vec3::Up, PI);
	return viewmodelTransform * GetNodeTransform(&player->anim, player->rightWeaponNode);
}

void SwitchLoadout(Player* player, int loadout)
{
	if (player->currentLoadout != loadout)
	{
		Action action;
		InitEquipAction(&action, player->rightWeapons[loadout], loadout);
		QueueAction(player->actions, action, *player);
	}
}


#include "SourceMovement.cpp"


void InitPlayer(Player* player, SDL_GPUCommandBuffer* cmdBuffer)
{
	*player = {};

	InitEntity(player, ENTITY_TYPE_PLAYER);

	player->position = vec3(0, 3, 3);

	LoadModel(&player->model, "res/models/viewmodel.glb.bin", false, cmdBuffer);
	InitAnimationState(&player->anim, &player->model);

	player->rightWeaponNode = GetNodeByName(&player->model, "weapon_r");
	player->leftWeaponNode = GetNodeByName(&player->model, "weapon_l");
	player->rightShoulderNode = GetNodeByName(&player->model, "clavicle_r");
	player->leftShoulderNode = GetNodeByName(&player->model, "clavicle_l");

	InitAnimation(&player->idleAnim, "idle", &player->model, 0.005f, true, false);

	InitCharacterController(&player->controller, 0.3f, 1.5f, 0.2f, player->position);
	InitRigidBody(&player->kinematicBody, RIGID_BODY_KINEMATIC, player->position, quat::Identity);
	AddCapsuleCollider(&player->kinematicBody, 0.2f, 1.5f, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_PLAYER, ENTITY_FILTER_ENEMY, false);
	player->kinematicBody.userPtr = player;

	InitActionManager(player->actions, &player->model);

	player->walkSpeed = 4.0f;

	player->health = 100;
	player->maxHealth = 100;

	player->stamina = 1.0f;
	player->exhausted = false;

	SetRightWeapon(player, 0, GetItem(ITEM_TYPE_LONGSWORD));
	SetRightWeapon(player, 1, GetItem(ITEM_TYPE_KINGS_SWORD));
	SetLeftWeapon(player, 1, GetItem(ITEM_TYPE_WOODEN_SHIELD));
}

void DestroyPlayer(Player* player)
{
	DestroyCharacterController(&player->controller);
	//DestroyModel(&player->model);
	//DestroyAnimationState(&player->anim);
}

static bool ArmAnimChannelFilter(Node* node, bool* right)
{
	if (*right) return !EndsWith(node->name, "_l");
	else return !EndsWith(node->name, "_r");
}

static mat4 CalculateViewBobbing(Player* player, int side)
{
	vec3 sway = vec3::Zero;
	float yawSway = 0;
	float pitchSway = 0;
	float rollSway = 0;

	float swayScale = /*actionManager.currentAction != null ? actionManager.currentAction.swayAmount :*/ 1;

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
	player->viewBobVerticalSpeedAnim = mix(player->viewBobVerticalSpeedAnim, verticalSpeedAnimDst * 0.0075f, 5 * deltaTime);
	pitchSway += player->viewBobVerticalSpeedAnim;

	// Land bob animation
	if (player->lastLandedTime)
	{
		float timeSinceLanding = gameTime - player->lastLandedTime;
		float landBob = (1.0f - SDL_powf(0.5f, timeSinceLanding * 4.0f)) * SDL_powf(0.1f, timeSinceLanding * 4.0f) * 0.5f;
		sway.y -= landBob;
	}

	/// Look sway
	float swayYawDst = game->cameraYaw; // -0.0015f * Input.cursorMove.x;
	float swayPitchDst = game->cameraPitch; // -0.0015f * Input.cursorMove.y;
	float swayRollDst = game->cameraYaw; // -0.0015f * Input.cursorMove.x;
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

void UpdatePlayer(Player* player)
{
	if (player->health <= 0)
	{
		game->cameraPosition.y = mix(game->cameraPosition.y, player->position.y + 0.2f, 5 * deltaTime);
		game->cameraPitch = mix(game->cameraPitch, 0.0f, 5 * deltaTime);
		return;
	}

	Action* currentAction = GetCurrentAction(player);

	for (int i = 0; i < NUM_LOADOUTS; i++)
	{
		if (GetKeyDown((SDL_Scancode)(SDL_SCANCODE_1 + i)))
		{
			if (currentAction && player->actions.actions.back()->type == ACTION_TYPE_EQUIP)
				CancelAction(player->actions, *player);
			SwitchLoadout(player, i);
			break;
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
				if (currentAction && currentAction->type == ACTION_TYPE_ATTACK && currentAction->attack.weapon == right)
				{
					nextAttack = GetNextAttack(currentAction->attack.attack, currentAction->attack.weapon);
					attackIdx = currentAction->attack.attackIdx + 1;
				}
				else if (player->sprinting && right->weapon.runningAttack != -1)
				{
					nextAttack = &right->weapon.attacks[right->weapon.runningAttack];
				}
				else
				{
					nextAttack = &right->weapon.attacks[0];
				}

				Action action;
				InitAttackAction(&action, right, nextAttack, attackIdx);
				QueueAction(player->actions, action, *player);
			}
		}
	}

	UpdateActionManager(player->actions, *player);
	currentAction = GetCurrentAction(player);

	AnimationPlayback* moveAnimation = &player->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(&player->model, &player->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop, nullptr, nullptr);

	Animation* rightAnimation = moveAnimation->animation;
	float rightAnimationTimer = moveAnimation->timer;
	bool rightAnimationLoop = moveAnimation->loop;
	float rightAnimationBlendDuration = 0.2f;

	Animation* leftAnimation = moveAnimation->animation;
	float leftAnimationTimer = moveAnimation->timer;
	bool leftAnimationLoop = moveAnimation->loop;
	float leftAnimationBlendDuration = 0.2f;

	Item* right = GetRightApparentWeapon(player);
	Item* left = GetLeftApparentWeapon(player);

	if (right)
	{
		rightAnimation = GetAnimationByName(&right->moveset, "idle");

		/*
		if (right->twoHanded)
		{
			AnimateModel(&player->model, &player->anim, weaponMoveAnimation, moveAnimation->timer, moveAnimation->loop, nullptr, nullptr);
		}
		else
		{
			bool right = true;
			BlendAnimation(&player->model, &player->anim, weaponMoveAnimation, moveAnimation->timer, moveAnimation->loop, 1, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
		}
		*/
	}

	if (left)
	{
		leftAnimation = GetAnimationByName(&left->moveset, "idle");
	}

	if (currentAction)
	{
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

		/*
		AnimationPlayback* actionAnimation = &currentAction->rightAnim;
		//actionAnimation->timer += deltaTime * actionAnimation->speed;
		actionAnimation->timer = currentAction->elapsedTime;

		if (currentAction->twoHanded)
		{
			AnimateModel(&player->model, &player->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, nullptr, nullptr);
		}
		else
		{
			bool right = true;
			AnimateModel(&player->model, &player->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
		}
		*/
	}

	if (rightAnimation)
	{
		bool right = true;
		AnimateModel(&player->model, &player->anim, rightAnimation, rightAnimationTimer, rightAnimationLoop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

		if (rightAnimation != player->lastRightAnim && player->lastRightAnim && rightAnimationBlendDuration)
		{
			player->rightBlendStart = gameTime;
			player->rightBlendAnim = player->lastRightAnim;
			player->rightBlendAnimTimer = player->lastRightAnimTimer;
			player->rightBlendAnimLoop = player->lastRightAnimLoop;
			player->rightBlendDuration = rightAnimationBlendDuration;
		}

		if (player->rightBlendStart)
		{
			float blendProgress = (gameTime - player->rightBlendStart) / player->rightBlendDuration;
			if (blendProgress > 1.0f)
			{
				player->rightBlendStart = 0;
			}
			else
			{
				BlendAnimation(&player->model, &player->anim, player->rightBlendAnim, player->rightBlendAnimTimer, player->rightBlendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
			}
		}

		player->lastRightAnim = rightAnimation;
		player->lastRightAnimTimer = rightAnimationTimer;
		player->lastRightAnimLoop = rightAnimationLoop;
	}
	if (leftAnimation)
	{
		bool right = false;
		AnimateModel(&player->model, &player->anim, leftAnimation, leftAnimationTimer, leftAnimationLoop, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);

		if (leftAnimation != player->lastLeftAnim && player->lastLeftAnim && leftAnimationBlendDuration)
		{
			player->leftBlendStart = gameTime;
			player->leftBlendAnim = player->lastLeftAnim;
			player->leftBlendAnimTimer = player->lastLeftAnimTimer;
			player->leftBlendAnimLoop = player->lastLeftAnimLoop;
			player->leftBlendDuration = leftAnimationBlendDuration;
		}

		if (player->leftBlendStart)
		{
			float blendProgress = (gameTime - player->leftBlendStart) / player->leftBlendDuration;
			if (blendProgress > 1.0f)
			{
				player->leftBlendStart = 0;
			}
			else
			{
				BlendAnimation(&player->model, &player->anim, player->leftBlendAnim, player->leftBlendAnimTimer, player->leftBlendAnimLoop, 1 - blendProgress, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
			}
		}

		player->lastLeftAnim = leftAnimation;
		player->lastLeftAnimTimer = leftAnimationTimer;
		player->lastLeftAnimLoop = leftAnimationLoop;
	}

	mat4 rightViewBob = CalculateViewBobbing(player, 0);
	mat4& rightShoulderTransform = GetNodeTransform(&player->anim, player->rightShoulderNode);
	rightShoulderTransform = rightViewBob * rightShoulderTransform;

	mat4& rightWeaponTransform = GetNodeTransform(&player->anim, player->rightWeaponNode);
	rightWeaponTransform = rightViewBob * rightWeaponTransform;

	mat4 leftViewBob = CalculateViewBobbing(player, GetLeftApparentWeapon(player) == GetLeftWeapon(player) ? 1 : 0);
	mat4& leftShoulderTransform = GetNodeTransform(&player->anim, player->leftShoulderNode);
	leftShoulderTransform = leftViewBob * leftShoulderTransform;

	mat4& leftWeaponTransform = GetNodeTransform(&player->anim, player->leftWeaponNode);
	leftWeaponTransform = leftViewBob * leftWeaponTransform;

	ApplyAnimationToSkeleton(&player->model, &player->anim);

	if (GetKeyDown(SDL_SCANCODE_F5))
		player->cameraMode = CAMERA_MODE_FIRST_PERSON ? CAMERA_MODE_FREE : CAMERA_MODE_FIRST_PERSON;

	SourceMovement(player);

	if (game->mouseLocked)
	{
		player->rotation -= app->mouseDelta.x * 0.001f;
		game->cameraPitch -= app->mouseDelta.y * 0.001f;

		game->cameraYaw = player->rotation;
	}

	SetAudioListener(game->cameraPosition, game->cameraPitch, game->cameraYaw);

	if (player->health < player->maxHealth && player->lastHit && gameTime - player->lastHit > HEALTH_REGEN_HIT_DELAY)
	{
		if (EveryInterval(0.1f, hash(player)))
			player->health++;
	}

	if (player->stamina <= 0 && !player->exhausted)
	{
		player->exhausted = true;
		PlaySound(&game->exhaustedSounds[game->random.next() % 2], 0.4f);
	}
	else if (player->stamina >= 0.5f)
	{
		player->exhausted = false;
	}

	if (player->stamina < 1.0f && player->actions.actions.size == 0 && !player->sprinting)
	{
		player->stamina = min(player->stamina + 0.2f * deltaTime, 1);
	}
}

void RenderPlayer(Player* player)
{
	mat4 cameraTransform = mat4::Translate(game->cameraPosition) * mat4::Rotate(quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch));
	mat4 viewmodelTransform = cameraTransform * mat4::Rotate(vec3::Up, PI);

	RenderModel(&game->renderer, &player->model, &player->anim, viewmodelTransform);

	if (Item* rightWeapon = GetRightWeapon(player))
	{
		mat4 weaponTransform = viewmodelTransform * GetNodeTransform(&player->anim, player->rightWeaponNode);
		RenderModel(&game->renderer, &rightWeapon->model, &player->anim, weaponTransform);
	}
	if (Item* leftWeapon = GetLeftWeapon(player))
	{
		mat4 weaponTransform = viewmodelTransform * GetNodeTransform(&player->anim, player->leftWeaponNode);
		RenderModel(&game->renderer, &leftWeapon->model, &player->anim, weaponTransform);
	}

	// vignette
	{
		float vignetteStrength = 1 - player->health / (float)player->maxHealth;
		if (player->lastHit && gameTime - player->lastHit < 5)
			vignetteStrength += SDL_expf(-(gameTime - player->lastHit) * 2) * 0.5f;
		vignetteStrength = min(vignetteStrength, 1);

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

		GUIPanel(x, y, (int)(w * max(player->stamina, 0)), h, color);
	}
}

static void OnDeath(Player* player)
{
	SDL_assert(game->gameOverTimer == -1);
	game->gameOverTimer = 0;
}

void HitPlayer(Player* player, HitParams hit, Entity* by)
{
	if (player->health <= 0)
		return;
	if (player->lastHit && gameTime - player->lastHit < HIT_RECOVERY_DURATION)
		return;

	int damage = (int)(hit.damage * hit.damageMultiplier);
	player->health -= damage;
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
