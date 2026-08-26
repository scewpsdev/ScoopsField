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
}

void RenderPortal(Portal* portal)
{
	RenderPortal(&game->renderer, portal);
}
