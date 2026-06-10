#include "Elevator.h"


void InitElevator(Elevator* elevator, vec3 position, quat rotation, float shaftHeight)
{
	InitEntity((Entity*)elevator, ENTITY_TYPE_ELEVATOR);

	elevator->position = position;
	elevator->rotation = rotation;

	elevator->shaftHeight = shaftHeight;

	elevator->toggled = false;
	elevator->buttonActive = false;

	elevator->model = GetModel("entities/object/elevator/elevator.glb");
	elevator->platform = GetModel("entities/object/elevator/elevator_platform.glb");
	elevator->button = GetModel("entities/object/elevator/elevator_button.glb");

	InitRigidBody(&elevator->body, RIGID_BODY_KINEMATIC, position, rotation, elevator);
	AddBoxCollider(&elevator->body, vec3(4, 0.5f, 4), vec3(0, -0.25f, 0), quat::Identity, ENTITY_FILTER_DEFAULT, ENTITY_FILTER_DEFAULT, false);

	InitRigidBody(&elevator->buttonBody, RIGID_BODY_KINEMATIC, position, rotation, elevator);
	AddBoxCollider(&elevator->buttonBody, vec3(1, 0.2f, 1), vec3(0, 0.1f, 0), quat::Identity, ENTITY_FILTER_DEFAULT, ENTITY_FILTER_DEFAULT, false);
}

void DestroyElevator(Elevator* elevator)
{
	DestroyRigidBody(&elevator->body);
}

void UpdateElevator(Elevator* elevator)
{
	if (!elevator->buttonActive)
	{
		PhysicsHit hits[16];
		if (OverlapBox(elevator->position + vec3(0, elevator->platformHeight + 0.25f, 0), vec3(1, 0.5f, 1), hits, 16, ENTITY_FILTER_PLAYER | ENTITY_FILTER_ENEMY | ENTITY_FILTER_RAGDOLL | ENTITY_FILTER_ITEM))
		{
			elevator->buttonActive = true;
			elevator->buttonActivatedTime = gameTime;

			// TODO sound
		}
	}

	const float activateDelay = 1.0f;
	if (elevator->buttonActivatedTime && gameTime - elevator->buttonActivatedTime > activateDelay)
	{
		elevator->buttonActivatedTime = 0;
		elevator->toggled = !elevator->toggled;
		elevator->moving = true;
	}

	float lastPlatformHeight = elevator->platformHeight;

	float platformTargetHeight = elevator->toggled ? elevator->shaftHeight : 0;
	float delta = platformTargetHeight - elevator->platformHeight;
	const float fadeDistance = 5;
	const float speed = 5;

	const float targetVelocity = SDL_fabsf(delta) > fadeDistance ? sign(delta) * speed : 0;
	elevator->velocity = moveTowards(elevator->velocity, targetVelocity, speed * 0.5f * deltaTime);

	elevator->platformHeight += elevator->velocity * deltaTime;
	if (elevator->moving && (!elevator->toggled && elevator->platformHeight < 0.01f || elevator->toggled && elevator->platformHeight > elevator->shaftHeight - 0.01f))
	{
		elevator->moving = false;
		elevator->platformHeight = clamp(elevator->platformHeight, 0, elevator->shaftHeight);
		elevator->velocity = 0;
		elevator->buttonDeactivatedTime = gameTime;
	}

	const float deactivateDelay = 1.0f;
	if (elevator->buttonDeactivatedTime && gameTime - elevator->buttonDeactivatedTime > deactivateDelay)
	{
		if (!OverlapBox(elevator->position + vec3(0, elevator->platformHeight + 0.25f, 0), vec3(1, 0.5f, 1), ENTITY_FILTER_PLAYER | ENTITY_FILTER_ENEMY | ENTITY_FILTER_RAGDOLL | ENTITY_FILTER_ITEM))
		{
			elevator->buttonDeactivatedTime = 0;
			elevator->buttonActive = false;

			// TODO sound
		}
	}

	float buttonTargetHeight = elevator->buttonActive ? -0.08f : 0;
	elevator->buttonHeight = moveTowards(elevator->buttonHeight, buttonTargetHeight, 0.08f * deltaTime);

	DebugText(0, 12, COLOR_WHITE, COLOR_BLACK, "%.2f", elevator->buttonHeight);

	if (elevator->moving)
	{
		SetRigidBodyTransform(&elevator->body, elevator->position + vec3(0, elevator->platformHeight, 0), elevator->rotation);

		PhysicsHit hits[16];
		int numHits = OverlapBox(elevator->position + vec3(0, elevator->platformHeight + 0.25f, 0), vec3(4, 0.5f, 4), hits, 16, ENTITY_FILTER_PLAYER | ENTITY_FILTER_ENEMY | ENTITY_FILTER_RAGDOLL | ENTITY_FILTER_ITEM);
		for (int i = 0; i < numHits; i++)
		{
			Entity* entity = (Entity*)hits[i].body->userPtr;
			if (entity->type == ENTITY_TYPE_PLAYER)
			{
				Player* player = (Player*)entity;
				MovePlayer(player, vec3(0, elevator->platformHeight - lastPlatformHeight, 0));
				player->grounded = true;
				player->lastGroundedTime = gameTime;
			}
		}
	}

	SetRigidBodyTransform(&elevator->buttonBody, elevator->position + vec3(0, elevator->platformHeight + elevator->buttonHeight, 0), elevator->rotation);
}

void RenderElevator(Elevator* elevator)
{
	mat4 transform = ModelMatrix((Entity*)elevator);
	RenderModel(&game->renderer, elevator->model, transform);
	RenderModel(&game->renderer, elevator->platform, mat4::Translate(0, elevator->platformHeight, 0) * transform);
	RenderModel(&game->renderer, elevator->button, mat4::Translate(0, elevator->platformHeight + elevator->buttonHeight, 0) * transform);
}
