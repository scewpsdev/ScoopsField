#pragma once

#include "renderer/Renderer.h"
#include "renderer/DebugTextRenderer.h"
#include "renderer/GUIRenderer.h"

#include "audio/Audio.h"

#include "physics/Physics.h"
#include "physics/RigidBody.h"
#include "physics/CharacterController.h"

#include "particle/ParticleSystem.h"

#include "math/Vector.h"

#include "utils/Random.h"
#include "utils/Pool.h"

#include "game/entity/Entity.h"

#include "game/item/Item.h"

#include "Navmesh.h"


struct GameState
{
	float gameTime;

	bool mouseLocked;
	vec3 cameraPosition;
	quat cameraRotation;
	float cameraNear /*, cameraFar*/;
	float cameraFov;

	mat4 projection, view, pv;
	vec4 frustumPlanes[6];

	Renderer renderer;
	GUIRenderer guiRenderer;

	Random random;

	ItemDatabase items;

	Player player;

#define MAX_ENTITIES 256
	Pool<Entity, MAX_ENTITIES> entities;

	ParticleSystem particles;

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
	Sound armorSound;
	Sound hitSlashSound;
	Sound hitSkeletonSound;
	Sound hitArmorSound;
	Sound hitArrowSound;
	Sound hitBlockSound;
	Sound hitParrySound;
	Sound hitShieldSound;
	Sound hitShieldParrySound;
	Sound stepBareSound, jumpBareSound, landBareSound;
	Sound fireSound;

	Texture* crosshair;
	Texture* crosshairInteract;
	Texture* hitmarker;
	Texture* blockmarker;
	Texture* vignette;
	Texture* roundCounter;
	Texture* digits;

	GraphicsPipeline* magicProjectileShader;
	GraphicsPipeline* trailShader;
	GraphicsPipeline* trailAdditiveShader;
	GraphicsPipeline* particleShader;
	GraphicsPipeline* particleAdditiveShader;

	ReflectionProbe reflectionProbes[8];
	int numReflectionProbes;
};


extern GameState* game;
extern PhysicsState* physics;
extern float gameTime;
extern float deltaTime;


Entity* CreateEntity();
