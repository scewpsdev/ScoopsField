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

#include "game/item/Item.h"

#include "Navmesh.h"


struct GameState
{
	bool mouseLocked;
	vec3 cameraPosition;
	quat cameraRotation;
	float cameraNear /*, cameraFar*/;

	mat4 projection, view, pv;
	vec4 frustumPlanes[6];

	Renderer renderer;
	Renderer2D guiRenderer;

	Random random;

	ItemDatabase items;

	Player player;

#define MAX_ENTITIES 256
	Pool<Entity, MAX_ENTITIES> entities;

	mat4 playerSpawn;

	int round;
	int points;
	float roundStartTimer;
	int numSkeletonsRemaining;
	float gameOverTimer;

	Model mapModel;
	Navmesh mapNavmesh;
	RigidBody mapCollider;

	Model cube;

	Sound testSound;
	Sound ambientSound;
	uint32_t ambientSource;
	Sound stepSounds[6];
	Sound landSound;
	Sound swingSounds[3];
	Sound slashHitSounds[2];
	Sound skeletonHitSounds[5];
	Sound exhaustedSounds[2];

	Texture* crosshair;
	Texture* crosshairInteract;
	Texture* vignette;
	Texture* roundCounter;
	Texture* digits;
};


extern GameState* game;
extern PhysicsState* physics;
extern float gameTime;
extern float deltaTime;
