#pragma once

#include "game/entity/EntityBase.h"

#include "model/Model.h"

#include "physics/RigidBody.h"


struct SnapPosition
{
	int rotation;
	ivec3 normal;
	Shape* shape;
	uint32_t flags;
};

enum BuildElementType
{
	BUILD_ELEMENT_TYPE_NONE = 0,

	BUILD_ELEMENT_TYPE_FLOOR,
	BUILD_ELEMENT_TYPE_WALL,
	BUILD_ELEMENT_TYPE_STAIRS,

	BUILD_ELEMENT_TYPE_COUNT
};

enum SnapType
{
	SNAP_TYPE_DEFAULT,
	SNAP_TYPE_ROTATE_ALONG,
};

struct BuildElement : EntityBase
{
	BuildElementType elementType;
	SnapType snapType;
	ivec4 tile;

	RigidBody collider;

	SnapPosition snapPositions[4];
	int numSnapPositions;
};


void InitBuildElement(BuildElement* element, ivec4 tile, BuildElementType elementType);
void DestroyBuildElement(BuildElement* element);

SnapPosition* GetSnapPosition(BuildElement* element, Shape* shape);
void RemoveSnapPosition(BuildElement* element, Shape* shape);

void UpdateBuildElement(BuildElement* element);
void RenderBuildElement(BuildElement* element);
