#include "Player.h"


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
		InitAnimation(&player->weaponIdleAnim, "idle", &player->rightWeapon->moveset);
		InitAnimation(&player->weaponAttackAnim, "attack1", &player->rightWeapon->moveset);
	}
	else
	{
		player->rightWeapon = nullptr;
	}
}


void InitPlayer(Player* player, SDL_GPUCommandBuffer* cmdBuffer)
{
	LoadModel(&player->model, "res/models/viewmodel.glb.bin", cmdBuffer);
	InitAnimationState(&player->anim, &player->model);

	player->rightWeaponNode = GetNodeByName(&player->model, "weapon_r");

	InitAnimation(&player->idleAnim, "idle", &player->model, 0.001f, true);
	InitAnimation(&player->runAnim, "run", &player->model, 1, true);

	InitCharacterController(&player->controller, 0.2f, 2, 0.2f, vec3(0, 3, 3));

	InitActionManager(player->actions);

	SetRightWeapon(player, ITEM_TYPE_KINGS_SWORD);
}

void DestroyPlayer(Player* player)
{
	//DestroyModel(&player->model);
	//DestroyAnimationState(&player->anim);
}

static bool ActionAnimChannelFilter(Node* node, bool* right)
{
	if (*right) return !EndsWith(node->name, "_l");
	else return !EndsWith(node->name, "_r");
}

void UpdatePlayer(Player* player)
{
	if (GetMouseButtonDown(SDL_BUTTON_LEFT) && player->rightWeapon)
	{
		Action action;
		InitAttackAction(&action, player->rightWeapon);
		QueueAction(player->actions, action);
	}

	UpdateActionManager(player->actions);

	AnimationPlayback* moveAnimation = player->moving ? &player->runAnim : &player->idleAnim;
	moveAnimation->timer += deltaTime * moveAnimation->speed;

	AnimateModel(&player->model, &player->anim, moveAnimation->animation, moveAnimation->timer, moveAnimation->loop);

	if (player->rightWeapon)
	{
		Animation* weaponMoveAnimation = GetAnimationByName(&player->rightWeapon->moveset, "idle");

		bool right = true;
		BlendAnimation(&player->model, &player->anim, weaponMoveAnimation, moveAnimation->timer, moveAnimation->loop, 1, (AnimationChannelFilterCallback_t)ActionAnimChannelFilter, &right);
	}

	if (Action* currentAction = GetCurrentAction(player))
	{
		AnimationPlayback* actionAnimation = &currentAction->anim;
		actionAnimation->timer += deltaTime * actionAnimation->speed;

		bool right = true;
		BlendAnimation(&player->model, &player->anim, actionAnimation->animation, actionAnimation->timer, actionAnimation->loop, 1, (AnimationChannelFilterCallback_t)ActionAnimChannelFilter, &right);
	}

	ApplyAnimationToSkeleton(&player->model, &player->anim);

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
			float speed = GetKey(SDL_SCANCODE_LSHIFT) ? 20.0f : GetKey(SDL_SCANCODE_LALT) ? 3.0f : 10.0f;
			vec3 velocity = delta.normalized() * speed;
			displacement += velocity * deltaTime;
		}

		if (GetKeyDown(SDL_SCANCODE_SPACE) && player->grounded)
		{
			player->verticalVelocity = 5;
			player->grounded = false;
		}

		player->verticalVelocity += 0.5f * -10 * deltaTime;
		displacement += vec3(0, player->verticalVelocity, 0) * deltaTime;
		player->verticalVelocity += 0.5f * -10 * deltaTime;

		ControllerCollisionFlags collisionFlags = MoveCharacterController(&player->controller, displacement, 1);
		if (collisionFlags & CONTROLLER_COLLISION_DOWN)
		{
			player->verticalVelocity = 0;
			player->grounded = true;
		}
		else
		{
			player->grounded = false;
		}

		player->moving = delta.lengthSquared() > 0;

		player->position = GetCharacterControllerPosition(&player->controller);
		game->cameraPosition = player->position + vec3(0, 1.5f, 0);
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
			float speed = GetKey(SDL_SCANCODE_LSHIFT) ? 20.0f : GetKey(SDL_SCANCODE_LALT) ? 3.0f : 10.0f;
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
}

void RenderPlayer(Player* player)
{
	mat4 cameraTransform = mat4::Translate(game->cameraPosition) * mat4::Rotate(quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch));
	mat4 viewmodelTransform = cameraTransform * mat4::Rotate(vec3::Up, PI);

	RenderModel(&game->renderer, &player->model, &player->anim, viewmodelTransform);

	if (player->rightWeapon)
	{
		RenderModel(&game->renderer, &player->rightWeapon->model, &player->anim, viewmodelTransform * GetNodeTransform(&player->anim, player->rightWeaponNode));
	}

	Action* currentAction = GetCurrentAction(player);
	if (currentAction)
		DebugText(0, height / 16 - 2, "%d, %.2f", currentAction->type, currentAction->elapsedTime / currentAction->duration);
}
