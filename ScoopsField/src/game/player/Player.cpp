#include "Player.h"


#define HEALTH_REGEN_HIT_DELAY 5.0f
#define HIT_RECOVERY_DURATION 0.25f


static Action* GetCurrentAction(Player* player)
{
	return QueuePeek(player->actions.actions);
}

static void SetRightWeapon(Player* player, ItemType itemType)
{
	SDL_assert(itemType < ITEM_TYPE_LAST);

	if (itemType != ITEM_TYPE_NONE)
	{
		player->rightWeapon = &game->items.items[itemType];
	}
	else
	{
		player->rightWeapon = nullptr;
	}
}

mat4 GetRightWeaponTransform(Player* player)
{
	mat4 cameraTransform = mat4::Translate(game->cameraPosition) * mat4::Rotate(quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch));
	mat4 viewmodelTransform = cameraTransform * mat4::Rotate(vec3::Up, PI);
	return viewmodelTransform * GetNodeTransform(&player->anim, player->rightWeaponNode);
}


void InitPlayer(Player* player, SDL_GPUCommandBuffer* cmdBuffer)
{
	InitEntity(player, ENTITY_TYPE_PLAYER);

	player->position = vec3(0, 3, 3);

	LoadModel(&player->model, "res/models/viewmodel.glb.bin", false, cmdBuffer);
	InitAnimationState(&player->anim, &player->model);

	player->rightWeaponNode = GetNodeByName(&player->model, "weapon_r");
	player->rightShoulderNode = GetNodeByName(&player->model, "clavicle_r");
	player->leftShoulderNode = GetNodeByName(&player->model, "clavicle_l");

	InitAnimation(&player->idleAnim, "idle", &player->model, 0.002f, true);

	InitCharacterController(&player->controller, 0.3f, 2, 0.2f, player->position);
	InitRigidBody(&player->kinematicBody, RIGID_BODY_KINEMATIC, player->position, quat::Identity);
	AddCapsuleCollider(&player->kinematicBody, 0.2f, 2.0f, vec3(0, 1, 0), quat::Identity, ENTITY_FILTER_PLAYER, ENTITY_FILTER_ENEMY, false);
	player->kinematicBody.userPtr = player;

	InitActionManager(player->actions, &player->model);

	player->walkSpeed = 4.0f;

	player->health = 100;
	player->maxHealth = 100;

	player->stamina = 1.0f;
	player->exhausted = false;

	SetRightWeapon(player, ITEM_TYPE_KINGS_SWORD);
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
	const float stepFrequency = 0.6f;
	viewmodelWalkAnim.x = 0.03f * SDL_sinf(player->distanceWalked * stepFrequency * PI);
	viewmodelWalkAnim.y = 0.015f * -SDL_fabsf(SDL_cosf(player->distanceWalked * stepFrequency * PI));
	//viewmodelWalkAnim *= 1 - Mathf.Smoothstep(1.0f, 1.5f, movementSpeed);
	viewmodelWalkAnim *= 1 - SDL_expf(-player->controllerVelocity.xz().length());
	//viewmodelWalkAnim *= (sprinting && runAnim.layers[1 + 0] != null && runAnim.layers[1 + 0].animationName == "run" || movement.isMoving && walkAnim.layers[1 + 0] != null && walkAnim.layers[1 + 0].animationName == "walk") ? 0 : 1;
	yawSway += viewmodelWalkAnim.x;
	sway.y += viewmodelWalkAnim.y;

	// Vertical speed animation
	float verticalSpeedAnimDst = player->controllerVelocity.y;
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

	if (GetMouseButton(SDL_BUTTON_LEFT) && player->rightWeapon && player->stamina > 0)
	{
		if (!currentAction || currentAction->elapsedTime > currentAction->followUpCancelTime)
		{
			int attackIdx = currentAction && currentAction->type == ACTION_TYPE_ATTACK && currentAction->attack.weapon == player->rightWeapon ? currentAction->attack.attackIdx + 1 : 0;

			Action action;
			InitAttackAction(&action, player->rightWeapon, &player->rightWeapon->weapon.attacks[attackIdx % player->rightWeapon->weapon.numAttacks], attackIdx);
			QueueAction(player->actions, action, *player);
		}
	}

	UpdateActionManager(player->actions, *player);

	AnimationPlayback* moveAnimation = &player->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(&player->model, &player->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop);

	if (player->rightWeapon)
	{
		Animation* weaponMoveAnimation = GetAnimationByName(&player->rightWeapon->moveset, "idle");

		bool right = true;
		BlendAnimation(&player->model, &player->anim, weaponMoveAnimation, moveAnimation->timer, moveAnimation->loop, 1, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
	}

	if (currentAction)
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;
		actionAnimation->timer += deltaTime * actionAnimation->speed;

		bool right = true;
		BlendAnimation(&player->model, &player->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, 1, (AnimationChannelFilterCallback_t)ArmAnimChannelFilter, &right);
	}

	mat4 rightViewBob = CalculateViewBobbing(player, 0);
	mat4& rightShoulderTransform = GetNodeTransform(&player->anim, player->rightShoulderNode);
	rightShoulderTransform = rightViewBob * rightShoulderTransform;

	mat4& rightWeaponTransform = GetNodeTransform(&player->anim, player->rightWeaponNode);
	rightWeaponTransform = rightViewBob * rightWeaponTransform;

	mat4& leftShoulderTransform = GetNodeTransform(&player->anim, player->leftShoulderNode);
	leftShoulderTransform = CalculateViewBobbing(player, 1) * leftShoulderTransform;

	ApplyAnimationToSkeleton(&player->model, &player->anim);

	if (GetKeyDown(SDL_SCANCODE_F5))
		player->cameraMode = CAMERA_MODE_FIRST_PERSON ? CAMERA_MODE_FREE : CAMERA_MODE_FIRST_PERSON;

	player->sprinting = false;
	if (player->cameraMode == CAMERA_MODE_FIRST_PERSON)
	{
		quat playerRotation = quat::FromAxisAngle(vec3::Up, player->rotation);

		vec3 delta = vec3::Zero;
		if (GetKey(SDL_SCANCODE_A)) delta += playerRotation.left();
		if (GetKey(SDL_SCANCODE_D)) delta += playerRotation.right();
		if (GetKey(SDL_SCANCODE_S)) delta += playerRotation.back();
		if (GetKey(SDL_SCANCODE_W)) delta += playerRotation.forward();

		vec3 displacement = vec3::Zero;
		if (delta.lengthSquared() > 0)
		{
			player->sprinting = GetKey(SDL_SCANCODE_LSHIFT) && player->stamina > 0 && !player->exhausted;
			float speed = (player->sprinting ? 2 : GetKey(SDL_SCANCODE_LALT) ? 0.25f : 1) * player->walkSpeed;
			if (player->sprinting)
				player->stamina -= 0.15f * deltaTime;
			if (currentAction)
				speed *= currentAction->moveSpeed;

			vec3 velocity = delta.normalized() * speed;
			displacement += velocity * deltaTime;
		}

		if (GetKeyDown(SDL_SCANCODE_SPACE) && player->grounded)
		{
			const float jumpPower = 7;
			player->verticalVelocity = jumpPower;
			player->grounded = false;
		}

		const float gravity = -20;
		player->verticalVelocity += 0.5f * gravity * deltaTime;
		displacement += vec3(0, player->verticalVelocity, 0) * deltaTime;
		player->verticalVelocity += 0.5f * gravity * deltaTime;

		player->distanceWalked += displacement.length();
		player->controllerVelocity = displacement / deltaTime;
		ControllerCollisionFlags collisionFlags = MoveCharacterController(&player->controller, displacement, ENTITY_FILTER_DEFAULT);
		if (collisionFlags & CONTROLLER_COLLISION_DOWN)
		{
			player->verticalVelocity = 0;
			player->grounded = true;
			if (player->controllerVelocity.y < -1)
			{
				player->lastLandedTime = gameTime;
			}
		}
		else
		{
			player->grounded = false;
		}

		player->moving = delta.lengthSquared() > 0;

		player->position = GetCharacterControllerPosition(&player->controller);
		SetRigidBodyTransform(&player->kinematicBody, player->position, quat::Identity);

		game->cameraPosition = player->position + vec3::Up * 1.5f;
	}
	else
	{
		quat cameraRotation = quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch);

		vec3 delta = vec3::Zero;
		if (GetKey(SDL_SCANCODE_A)) delta += cameraRotation.left();
		if (GetKey(SDL_SCANCODE_D)) delta += cameraRotation.right();
		if (GetKey(SDL_SCANCODE_S)) delta += cameraRotation.back();
		if (GetKey(SDL_SCANCODE_W)) delta += cameraRotation.forward();
		if (GetKey(SDL_SCANCODE_SPACE)) delta += vec3::Up;
		if (GetKey(SDL_SCANCODE_LCTRL)) delta += vec3::Down;

		if (delta.lengthSquared() > 0)
		{
			float speed = (GetKey(SDL_SCANCODE_LSHIFT) ? 2 : GetKey(SDL_SCANCODE_LALT) ? 0.25f : 1) * player->walkSpeed;
			vec3 velocity = delta.normalized() * speed;
			vec3 displacement = velocity * deltaTime;
			game->cameraPosition += displacement;
		}

		player->moving = delta.lengthSquared() > 0;
	}

	if (game->mouseLocked)
	{
		player->rotation -= app->mouseDelta.x * 0.001f;
		game->cameraPitch -= app->mouseDelta.y * 0.001f;

		game->cameraYaw = player->rotation;
	}

	if (player->health < player->maxHealth && player->lastHit && gameTime - player->lastHit > HEALTH_REGEN_HIT_DELAY)
	{
		if (EveryInterval(0.1f, hash(player)))
			player->health++;
	}

	if (player->stamina <= 0)
		player->exhausted = true;
	else if (player->stamina >= 0.5f)
		player->exhausted = false;

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

	if (player->rightWeapon)
	{
		mat4 weaponTransform = viewmodelTransform * GetNodeTransform(&player->anim, player->rightWeaponNode);
		RenderModel(&game->renderer, &player->rightWeapon->model, &player->anim, weaponTransform);
	}

	// vignette
	{
		float vignetteStrength = 1 - player->health / (float)player->maxHealth;
		if (player->lastHit && gameTime - player->lastHit < 5)
			vignetteStrength += SDL_expf(-(gameTime - player->lastHit) * 2) * 0.5f;
		vignetteStrength = min(vignetteStrength, 1);
		GUIPanel(0, 0, width, height, game->vignette, vec4(1, 0, 0, vignetteStrength));
	}

	// stamina
	if (player->stamina < 1)
	{
		int w = width / 2;
		int h = 5;
		int x = width / 2 - w / 2;
		int y = height - 20;
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
