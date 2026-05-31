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
	float cameraFov;

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

	Model mapModel;
	Navmesh mapNavmesh;
	RigidBody mapCollider;

	Model cube;

	Sound ambientSound;
	uint32_t ambientSource;
	Sound stepSound;
	Sound landSound;
	Sound exhaustedSound;
	Sound swingSound;
	Sound slashHitSound;
	Sound skeletonHitSound;
	Sound hitArmorSound;

	Texture* crosshair;
	Texture* crosshairInteract;
	Texture* vignette;
	Texture* roundCounter;
	Texture* digits;

	GraphicsPipeline* magicProjectileShader;
	GraphicsPipeline* trailShader;

	Material trailMaterial;

	ReflectionProbe reflectionProbe;
};


extern GameState* game;
extern PhysicsState* physics;
extern float gameTime;
extern float deltaTime;
