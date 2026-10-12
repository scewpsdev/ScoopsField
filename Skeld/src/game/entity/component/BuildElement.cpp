#include "BuildElement.h"


void InitBuildElement(BuildElement* element, ivec4 tile, BuildElementType elementType)
{
	InitEntity((Entity*)element, ENTITY_TYPE_BUILD_ELEMENT);

	element->elementType = elementType;
	element->snapType = elementType == BUILD_ELEMENT_TYPE_WALL || elementType == BUILD_ELEMENT_TYPE_STAIRS ? SNAP_TYPE_ROTATE_ALONG : SNAP_TYPE_DEFAULT;

	element->tile = tile;
	element->position = (tile.xyz + vec3(0.5f, 0, 0.5f)) * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE);
	element->rotation = element->snapType == SNAP_TYPE_ROTATE_ALONG ? quat::FromAxisAngle(vec3::Up, -tile.w * PI * 0.5f) : quat::Identity;

	InitRigidBody(&element->collider, RIGID_BODY_STATIC, element->position, element->rotation, element);

	if (elementType == BUILD_ELEMENT_TYPE_FLOOR)
	{
		element->model = GetModel("entities/build_element/wood_floor/wood_floor.glb");

		AddBoxCollider(&element->collider, vec3(TERRAIN_TILE_SIZE, TERRAIN_TILE_SIZE * 0.05f, TERRAIN_TILE_SIZE), vec3(0), quat::Identity, ENTITY_FILTER_DEFAULT, ENTITY_FILTER_DEFAULT, false);

		element->numSnapPositions = 4;
		element->snapPositions[0] = { .rotation = 0, .normal = ivec3(0, 0, -1), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		element->snapPositions[1] = { .rotation = 1, .normal = ivec3(1, 0, 0), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		element->snapPositions[2] = { .rotation = 2, .normal = ivec3(0, 0, 1), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		element->snapPositions[3] = { .rotation = 3, .normal = ivec3(-1, 0, 0), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };

		for (int i = 0; i < element->numSnapPositions; i++)
		{
			vec3 samplePosition = element->position + element->rotation * element->snapPositions[i].normal * 0.5f * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE);
			BuildElement* overlapElement = nullptr;

			PhysicsHit hit = {};
			bool snapPositionFound = OverlapSphere(samplePosition, 0.001f, &hit, 1, ENTITY_FILTER_BUILD_SNAP_FLOOR);
			if (snapPositionFound)
			{
				Entity* overlapEntity = (Entity*)hit.body->userPtr;
				SDL_assert(overlapEntity->type == ENTITY_TYPE_BUILD_ELEMENT);
				overlapElement = (BuildElement*)overlapEntity;
			}

			if (overlapElement && overlapElement->elementType == element->elementType)
			{
				RemoveSnapPosition(overlapElement, hit.shape);
				element->snapPositions[i].shape = nullptr;
			}
			else
			{
				element->snapPositions[i].shape = AddBoxCollider(&element->collider, vec3(0.5f * SQRT2, 0.2f, 0.5f * SQRT2) * TERRAIN_TILE_SIZE, element->snapPositions[i].normal * 0.5f * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE), quat::FromAxisAngle(vec3::Up, PI * 0.25f), element->snapPositions[i].flags, 0, true);
			}
		}
	}
	else if (elementType == BUILD_ELEMENT_TYPE_WALL)
	{
		element->model = GetModel("entities/build_element/wood_wall/wood_wall.glb");

		AddBoxCollider(&element->collider, vec3(TERRAIN_TILE_SIZE, TERRAIN_TILE_SIZE, TERRAIN_TILE_SIZE * 0.05f), vec3(0, 0.5f * TERRAIN_TILE_SIZE, 0.5f * TERRAIN_TILE_SIZE), quat::Identity, ENTITY_FILTER_DEFAULT, ENTITY_FILTER_DEFAULT, false);

		element->numSnapPositions = 4;
		element->snapPositions[0] = { .rotation = 0, .normal = ivec3(0, -TERRAIN_STEPS_PER_UNIT, 0), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		element->snapPositions[1] = { .rotation = 0, .normal = ivec3(-1, 0, 0), .flags = ENTITY_FILTER_BUILD_SNAP_WALL };
		element->snapPositions[2] = { .rotation = 0, .normal = ivec3(0, TERRAIN_STEPS_PER_UNIT, 0), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		element->snapPositions[3] = { .rotation = 0, .normal = ivec3(1, 0, 0), .flags = ENTITY_FILTER_BUILD_SNAP_WALL };

		for (int i = 0; i < element->numSnapPositions; i++)
		{
			vec3 samplePosition = element->position + element->rotation * (vec3(0, 0.5f * TERRAIN_TILE_SIZE, 0.5f * TERRAIN_TILE_SIZE) + element->snapPositions[i].normal * 0.5f * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE));
			BuildElement* overlapElement = nullptr;

			PhysicsHit hit = {};
			bool snapPositionFound = OverlapSphere(samplePosition, 0.001f, &hit, 1, ENTITY_FILTER_BUILD_SNAP_WALL);
			if (snapPositionFound)
			{
				Entity* overlapEntity = (Entity*)hit.body->userPtr;
				SDL_assert(overlapEntity->type == ENTITY_TYPE_BUILD_ELEMENT);
				overlapElement = (BuildElement*)overlapEntity;
			}

			if (overlapElement && overlapElement->elementType == element->elementType)
			{
				RemoveSnapPosition(overlapElement, hit.shape);
				element->snapPositions[i].shape = nullptr;
			}
			else
			{
				element->snapPositions[i].shape = AddBoxCollider(&element->collider, vec3(0.5f * SQRT2, 0.5f * SQRT2, 0.2f) * TERRAIN_TILE_SIZE, vec3(0, 0.5f * TERRAIN_TILE_SIZE, 0.5f * TERRAIN_TILE_SIZE) + element->snapPositions[i].normal * 0.5f * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE), quat::FromAxisAngle(vec3::AxisZ, PI * 0.25f), element->snapPositions[i].flags, 0, true);
			}
		}
	}
	else if (elementType == BUILD_ELEMENT_TYPE_STAIRS)
	{
		element->model = GetModel("entities/build_element/wood_stairs/wood_stairs.glb");

		float stairsAngle = SDL_atan2f(3, 4);
		float stairsHeight = 0.75f;
		float stairsLength = SDL_sqrtf(1 * 1 + stairsHeight * stairsHeight);

		AddBoxCollider(&element->collider, vec3(TERRAIN_TILE_SIZE, TERRAIN_TILE_SIZE * 0.05f, TERRAIN_TILE_SIZE * stairsLength), vec3(0, 0.5f * stairsHeight, 0), quat::FromAxisAngle(vec3::Right, stairsAngle), ENTITY_FILTER_DEFAULT, ENTITY_FILTER_DEFAULT, false);

		element->numSnapPositions = 2;
		element->snapPositions[0] = { .rotation = 0, .normal = ivec3(0, TERRAIN_STEPS_PER_UNIT - 1, -1), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		//element->snapPositions[1] = { .rotation = 1, .normal = ivec3(1, 0, 0), .flags = ENTITY_FILTER_BUILD_SNAP_BOTH };
		element->snapPositions[1] = { .rotation = 0, .normal = ivec3(0, 0, 1), .flags = ENTITY_FILTER_BUILD_SNAP_ALL };
		//element->snapPositions[3] = { .rotation = 3, .normal = ivec3(-1, 0, 0), .flags = ENTITY_FILTER_BUILD_SNAP_BOTH };

		for (int i = 0; i < element->numSnapPositions; i++)
		{
			vec3 samplePosition = element->position + element->rotation * element->snapPositions[i].normal * 0.5f * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE);
			BuildElement* overlapElement = nullptr;

			PhysicsHit hit = {};
			bool snapPositionFound = OverlapSphere(samplePosition, 0.001f, &hit, 1, ENTITY_FILTER_BUILD_SNAP_FLOOR);
			if (snapPositionFound)
			{
				Entity* overlapEntity = (Entity*)hit.body->userPtr;
				SDL_assert(overlapEntity->type == ENTITY_TYPE_BUILD_ELEMENT);
				overlapElement = (BuildElement*)overlapEntity;
			}

			if (overlapElement && overlapElement->elementType == element->elementType)
			{
				RemoveSnapPosition(overlapElement, hit.shape);
				element->snapPositions[i].shape = nullptr;
			}
			else
			{
				element->snapPositions[i].shape = AddBoxCollider(&element->collider, vec3(0.5f * SQRT2, 0.2f, 0.5f * SQRT2) * TERRAIN_TILE_SIZE, element->snapPositions[i].normal * 0.5f * vec3(TERRAIN_TILE_SIZE, TERRAIN_STEP_HEIGHT, TERRAIN_TILE_SIZE), quat::FromAxisAngle(vec3::Up, PI * 0.25f), element->snapPositions[i].flags, 0, true);
			}
		}
	}
	else
	{
		SDL_assert(false);
	}
}

void DestroyBuildElement(BuildElement* element)
{
	DestroyRigidBody(&element->collider);
}

SnapPosition* GetSnapPosition(BuildElement* element, Shape* shape)
{
	for (int i = 0; i < element->numSnapPositions; i++)
	{
		if (element->snapPositions[i].shape == shape)
			return &element->snapPositions[i];
	}
	return nullptr;
}

void RemoveSnapPosition(BuildElement* element, Shape* shape)
{
	SnapPosition* snapPosition = GetSnapPosition(element, shape);
	SDL_assert(snapPosition);
	snapPosition->shape = nullptr;

	RemoveCollider(&element->collider, shape);
}

void UpdateBuildElement(BuildElement* element)
{
}

void RenderBuildElement(BuildElement* element)
{
	RenderModel(&game->renderer, element->model, mat4::Translate(element->position) * mat4::Rotate(element->rotation) * mat4::Scale(TERRAIN_TILE_SIZE), true);

	for (int i = 0; i < element->numSnapPositions; i++)
	{
		//RenderModel(&game->renderer, &game->cube,
		//	mat4::Translate(element->position + vec3(0.5f * TERRAIN_TILE_SIZE, 0, 0.5f * TERRAIN_TILE_SIZE) + element->snapPositions[i].normal * 0.5f * TERRAIN_TILE_SIZE) *
		//	mat4::Scale(0.5f * TERRAIN_TILE_SIZE * SQRT2, 0.2f * TERRAIN_TILE_SIZE, 0.5f * TERRAIN_TILE_SIZE * SQRT2) *
		//	mat4::Rotate(vec3::Up, PI * 0.25f));
	}
}
