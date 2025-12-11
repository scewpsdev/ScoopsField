#include "Player.h"


static Animation* GetAnimationByName(Model* model, const char* name)
{
	for (int i = 0; i < model->numAnimations; i++)
	{
		if (SDL_strcmp(model->animations[i].name, name) == 0)
			return &model->animations[i];
	}
	return nullptr;
}

static void InitAnimation(AnimationPlayback* animation, const char* name, Player* player, float speed = 1.0f, bool loop = true, bool mirror = false)
{
	SDL_strlcpy(animation->name, name, 32);
	animation->speed = speed;
	animation->loop = loop;
	animation->mirror = mirror;
	animation->animation = GetAnimationByName(&player->model, name);
}

static void SetAnimation(Player* player, AnimationPlayback* animation)
{
	player->currentAnim = animation;
}


void InitPlayer(Player* player, SDL_GPUCommandBuffer* cmdBuffer)
{
	LoadModel(&player->model, "res/models/viewmodel.glb.bin", cmdBuffer);
	InitAnimationState(&player->anim, &player->model);

	InitAnimation(&player->idleAnim, "idle", player, 0.001f);
	InitAnimation(&player->runAnim, "run", player);
	SetAnimation(player, &player->idleAnim);

	InitCharacterController(&player->controller, 0.2f, 2, 0.2f, vec3(0, 3, 3));
}

void DestroyPlayer(Player* player)
{
	//DestroyModel(&player->model);
	//DestroyAnimationState(&player->anim);
}

void UpdatePlayer(Player* player)
{
	SetAnimation(player, player->moving ? &player->runAnim : &player->idleAnim);
	player->animationTimer += deltaTime * player->currentAnim->speed;

	AnimateModel(&player->model, &player->anim, player->currentAnim->animation, player->animationTimer, player->currentAnim->loop);

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
	RenderModel(&game->renderer, &player->model, &player->anim, mat4::Translate(game->cameraPosition) * mat4::Rotate(quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch)) * mat4::Rotate(vec3::Up, PI));
}
