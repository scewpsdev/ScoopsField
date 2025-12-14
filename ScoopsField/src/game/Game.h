#pragma once

#include "renderer/Renderer.h"
#include "renderer/DebugTextRenderer.h"
#include "renderer/Renderer2D.h"

#include "audio/Audio.h"

#include "physics/Physics.h"
#include "physics/RigidBody.h"
#include "physics/CharacterController.h"

#include "math/Vector.h"

#include "utils/Random.h"
#include "utils/Pool.h"

#include "game/player/Player.h"
#include "game/entity/Skeleton.h"

#include "game/item/Item.h"

#include "Navmesh.h"


struct GameState
{
	bool mouseLocked;
	vec3 cameraPosition;
	float cameraPitch, cameraYaw;
	float cameraNear, cameraFar;

	mat4 projection, view, pv;
	vec4 frustumPlanes[6];

	Renderer renderer;
	Renderer2D guiRenderer;

	Random random;

	ItemDatabase items;

	Player player;

#define MAX_SKELETON_ENTITIES 64
	Pool<SkeletonEntity, MAX_SKELETON_ENTITIES> skeletons;

	int round;
	int points;
#define ROUND_START_DELAY 5.0f
	float roundStartTimer;
	int numSkeletonsRemaining;
	float gameOverTimer;

	Model mapModel;
	Navmesh mapNavmesh;
	RigidBody mapCollider;

	Model cube;
	Sound testSound;

	Texture* crosshair;
	Texture* vignette;
	Texture* roundCounter;
	Texture* digits;
};


extern GameState* game;
extern PhysicsState* physics;
extern float gameTime;
extern float deltaTime;
