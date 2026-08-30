#include "Portal.h"


void InitPortal(Portal* portal, vec3 position, quat rotation)
{
	InitEntity((Entity*)portal, ENTITY_TYPE_PORTAL);

	portal->position = position;
	portal->rotation = rotation;

	portal->model = GetModel("entities/object/portal/portal.glb");
}

void DestroyPortal(Portal* portal)
{
}

void UpdatePortal(Portal* portal)
{
	Player* player = &game->player;

	mat4 transform = ModelMatrix((Entity*)portal);
	vec3 relativeToPortal = transform.inverted() * game->cameraPosition;

	if (relativeToPortal.x >= -1 && relativeToPortal.x <= 1 && relativeToPortal.y >= -1 && relativeToPortal.y <= 1 && relativeToPortal.z < 0 && relativeToPortal.z > -1)
	{
		mat4 otherTransform = ModelMatrix((Entity*)portal->destination);
		mat4 playerTransform = ModelMatrix((Entity*)player);

		mat4 portalDelta = otherTransform * mat4::Rotate(vec3::Up, PI) * transform.inverted();
		mat4 newTransform = portalDelta * playerTransform;

		vec3 newPosition = newTransform.translation();

		TeleportPlayer(player, newPosition);
		((EntityBase*)player)->rotation = newTransform.rotation();
		player->rotation += portalDelta.rotation().eulers().y;
		//player->pitch = eulers.x;
		player->yaw += portalDelta.rotation().eulers().y;
		player->velocity = (portalDelta * vec4(player->velocity, 0)).xyz;

		game->cameraPosition = portalDelta * game->cameraPosition;
		game->cameraRotation = portalDelta.rotation() * game->cameraRotation;
	}
}

void RenderPortal(Portal* portal)
{
	RenderPortal(&game->renderer, portal);
}
