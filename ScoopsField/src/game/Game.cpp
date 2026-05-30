#include "graphics/VertexBuffer.h"

#include "model/Model.h"

#include "math/Vector.h"


#define ROUND_START_DELAY 500.0f
#define GAME_OVER_DELAY 5.0f


extern SDL_GPUDevice* device;


static bool EveryInterval(float seconds, uint32_t h)
{
	float time = gameTime + (h / (float)UINT32_MAX) * seconds;
	int iteration = (int)(time / seconds);
	int lastIteration = (int)((time - deltaTime) / seconds);
	return iteration != lastIteration || time - deltaTime < 0;
}


#include "item/Item.cpp"
#include "player/Player.cpp"
#include "entity/component/Creature.cpp"
#include "entity/component/ItemEntity.cpp"
#include "entity/component/RestingSpot.cpp"
#include "entity/component/Projectile.cpp"


static void ResetGame(bool destroy)
{
	if (destroy)
	{
		if (game->ambientSource)
		{
			StopSound(game->ambientSource);
			game->ambientSource = 0;
		}

		DestroyPlayer(&game->player);

		for (int i = 0; i < game->entities.capacity; i++)
		{
			if (game->entities.occupied[i])
			{
				Entity* entity = &game->entities.data[i];
				DestroyEntity(entity);
			}
		}
		ClearPool(&game->entities);
	}


	InitPool(&game->entities);

	//game->ambientSource = PlaySound(&game->ambientSound, 0.5f);

	game->cameraPosition = vec3(0, 0, 3);
	//game->cameraPitch = -0.4f * PI;
	//game->cameraYaw = 0.25f * PI;
	game->cameraNear = 0.01f;
	//game->cameraFar = 1000;

	game->mouseLocked = true;

	game->playerSpawn = mat4::Identity;

	for (int i = 0; i < game->mapModel.numNodes; i++)
	{
		Node* node = &game->mapModel.nodes[i];
		if (SDL_strncmp(node->name, "Spawn", 5) == 0)
		{
			// spawn
			game->playerSpawn = node->transform;
		}
		else if (SDL_strncmp(node->name, "Entity", 6) == 0)
		{
			// entity
			SDL_assert(SDL_strlen(node->name) == 6 || node->name[6] == ' ' && SDL_strlen(node->name) > 7);
			char entityType[32] = "";
			char* entityTypeStart = node->name + 7;
			char* entityTypeEnd = SDL_strchr(entityTypeStart, '#');
			SDL_memcpy(entityType, entityTypeStart, (int)(entityTypeEnd - entityTypeStart));

			if (SDL_strcmp(entityType, "carpet") == 0)
			{
				mat4 transform = node->transform;
				Entity* carpet = PoolAlloc(&game->entities);
				InitRestingSpot(carpet, transform.translation(), transform.rotation());
			}
		}
	}

	InitPlayer(&game->player, cmdBuffer, game->playerSpawn.translation(), game->playerSpawn.rotation().getAngle());

	Entity* skeleton = PoolAlloc(&game->entities);
	SDL_assert(skeleton);
	InitKnight(skeleton, vec3(0, 0, -5), 0);

	InitItemEntity(PoolAlloc(&game->entities), GetItem(ITEM_KINGS_SWORD), vec3(-2, 2, -2), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));
	InitItemEntity(PoolAlloc(&game->entities), GetItem(ITEM_LONGSWORD), vec3(0, 2, -2), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));
	InitItemEntity(PoolAlloc(&game->entities), GetItem(ITEM_DARKWOOD_STAFF), vec3(2, 2, -2), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));
}

void GameInit(SDL_GPUCommandBuffer* cmdBuffer)
{
	InitRenderer(&game->renderer, app->width, app->height, cmdBuffer);

	Renderer2DLayerInfo layerInfo = {};
	layerInfo.width = app->width;
	layerInfo.height = app->height;
	layerInfo.maxSprites = 1000;
	layerInfo.textureFormat = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	InitRenderer2D(&game->guiRenderer, 1, &layerInfo, cmdBuffer);

	InitRandom(&game->random, (uint32_t)SDL_GetTicks());

	LoadModel(&game->cube, "res/models/cube.glb.bin", false, cmdBuffer);

	//LoadModel(&game->mapModel, "res/maps/testmap/testmap.gltf.bin", true, cmdBuffer);
	//LoadModel(&game->mapModel, "res/maps/painted_world/painted_world.glb.bin", true, cmdBuffer);
	LoadModel(&game->mapModel, "res/maps/testmap/testmap.glb.bin", true, cmdBuffer);

	InitRigidBody(&game->mapCollider, RIGID_BODY_STATIC, vec3::Zero, quat::Identity);
	AddModelCollider(&game->mapCollider, &game->mapModel, vec3::Zero, quat::Identity, vec3::One, 1, 1, false);

	Model* navmeshModel = (Model*)BumpAllocatorMalloc(&memory->transientAllocator, sizeof(Model));
	LoadModel(navmeshModel, "res/maps/testmap/testmap_navmesh.glb.bin", true, cmdBuffer);
	InitNavmesh(&game->mapNavmesh, navmeshModel);

	LoadSound(&game->ambientSound, "res/sounds/ambience.ogg.bin");
	LoadSounds(&game->stepSound, "sounds/step", 6);
	LoadSound(&game->landSound, "res/sounds/land.ogg.bin");
	LoadSounds(&game->exhaustedSound, "sounds/exhausted", 2);
	LoadSounds(&game->swingSound, "sounds/swing", 3);
	LoadSounds(&game->slashHitSound, "sounds/hit_slash", 2);
	LoadSounds(&game->skeletonHitSound, "sounds/hit_rock", 5);
	LoadSounds(&game->hitArmorSound, "sounds/hit/hit_armor", 10);

	game->crosshair = LoadTexture("res/textures/ui/crosshair.png.bin", cmdBuffer);
	game->crosshairInteract = LoadTexture("res/textures/ui/crosshair_hand.png.bin", cmdBuffer);
	game->vignette = LoadTexture("res/textures/vignette.png.bin", cmdBuffer);
	game->roundCounter = LoadTexture("res/textures/counter.png.bin", cmdBuffer);
	game->digits = LoadTexture("res/textures/digits.png.bin", cmdBuffer);

	game->magicProjectileShader = CreateForwardGraphicsPipeline(
		LoadGraphicsShader("res/shaders/entity/magic_projectile.vert.bin", "res/shaders/entity/magic_projectile.frag.bin"),
		SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_NONE, false);

#ifdef _DEBUG
	AddHotReloadedShader("shaders/mesh.vert", "shaders/mesh.frag", game->renderer.defaultShader, game->renderer.geometryPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/shadow.frag", game->renderer.shadowShader, game->renderer.shadowPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/blurh.frag", game->renderer.blurHShader, game->renderer.blurHPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/blurv.frag", game->renderer.blurVShader, game->renderer.blurVPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/directional_light.frag", game->renderer.directionalLightShader, game->renderer.directionalLightPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/environment_light.frag", game->renderer.environmentLightShader, game->renderer.environmentLightPipeline);
	AddHotReloadedShader("shaders/lighting/point_light.vert", "shaders/lighting/point_light.frag", game->renderer.pointLightShader, game->renderer.pointLightPipeline);
	AddHotReloadedShader("shaders/lighting/reflection_probe.vert", "shaders/lighting/reflection_probe.frag", game->renderer.reflectionProbeShader, game->renderer.reflectionProbePipeline);
	AddHotReloadedShader("shaders/sky/sky.vert", "shaders/sky/sky.frag", game->renderer.skyShader, game->renderer.skyPipeline);
	AddHotReloadedShader("shaders/sky/sky_upsample.vert", "shaders/sky/sky_upsample.frag", game->renderer.skyUpsampleShader, game->renderer.skyUpsamplePipeline);
	AddHotReloadedShader("shaders/sky/sky_cube.vert", "shaders/sky/sky_cube.frag", game->renderer.skyCubeShader, game->renderer.skyCubePipeline);
	AddHotReloadedComputeShader("shaders/sky/transmittance_lut.comp", game->renderer.skyTransmittaceLUTShader);
	AddHotReloadedComputeShader("shaders/sky/multiscatter_lut.comp", game->renderer.skyMultiScatterLUTShader);
	AddHotReloadedComputeShader("shaders/sky/skyview_lut.comp", game->renderer.skyViewLUTShader);
	AddHotReloadedComputeShader("shaders/sky/sun_color.comp", game->renderer.sunColorShader);
	AddHotReloadedComputeShader("shaders/sky/cloud_noise.comp", game->renderer.cloudNoiseShader);
	AddHotReloadedComputeShader("shaders/sky/cloud_noise_detail.comp", game->renderer.cloudNoiseDetailShader);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/tonemapping.frag", game->renderer.tonemappingShader, game->renderer.tonemappingPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/deferred_diffuse.frag", game->renderer.deferredDiffuseShader, game->renderer.deferredDiffusePipeline);
	AddHotReloadedShader("shaders/entity/magic_projectile.vert", "shaders/entity/magic_projectile.frag", game->magicProjectileShader->pipelineInfo.shader, game->magicProjectileShader);
#endif

	InitItemDatabase(&game->items, cmdBuffer);

	InitReflectionProbe(&game->reflectionProbe, vec3(0, 29, -58), vec3(9, 13, 9));

	ResetGame(false);
}

void GameDestroy()
{
	DestroyRenderer(&game->renderer);
}

void GameResize(int newWidth, int newHeight)
{
	ResizeRenderer(&game->renderer, newWidth, newHeight);
	ResizeRenderer2D(&game->guiRenderer, newWidth, newHeight);
}

static bool cameraZoom = false;
void GameUpdate()
{
	UpdateHotReloadedResources();

	if (app->keys[SDL_SCANCODE_ESCAPE] && !app->lastKeys[SDL_SCANCODE_ESCAPE])
		game->mouseLocked = !game->mouseLocked;

	SDL_SetWindowRelativeMouseMode(window, game->mouseLocked);

	UpdatePlayer(&game->player);

	for (int i = 0; i < game->entities.capacity; i++)
	{
		if (game->entities.occupied[i])
		{
			Entity* entity = &game->entities.data[i];
			UpdateEntity(entity);
			if (entity->removed)
			{
				DestroyEntity(entity);
				PoolRelease(&game->entities, entity);
			}
		}
	}

	if (GetKeyDown(SDL_SCANCODE_C))
		cameraZoom = !cameraZoom;
	game->cameraFov = cameraZoom ? 30.0f : 90.0f;
	game->projection = mat4::Perspective(game->cameraFov * Deg2Rad, app->width / (float)app->height, game->cameraNear);
	game->view = mat4::Rotate(game->cameraRotation.conjugated()) * mat4::Translate(-game->cameraPosition);
	game->pv = game->projection * game->view;
	GetFrustumPlanes(game->pv, game->frustumPlanes);

	// discard fragments in front of reflection probe volume

	if (GetKeyDown(SDL_SCANCODE_R))
	{
		TeleportPlayer(&game->player, game->playerSpawn.translation());
		game->player.velocity = vec3::Zero;
		game->player.rotation = game->playerSpawn.rotation().getAngle();
	}
}

void GameRender()
{
	BeginRenderer2D(&game->guiRenderer);

	RenderPlayer(&game->player);

	for (int i = 0; i < game->entities.capacity; i++)
	{
		if (game->entities.occupied[i])
		{
			RenderEntity(&game->entities.data[i]);
		}
	}

	RenderModel(&game->renderer, &game->mapModel, nullptr, mat4::Identity);

	RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Up, gameTime * 0.5f * PI) * vec3(2, 2, 0), vec3(1, 0.5f, 1) * 50);
	RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Right, gameTime * 0.5f * PI * 0.7f) * vec3(2, 2, 0), vec3(0.5f, 1, 0.5f) * 50);

	UpdateReflectionProbe(&game->renderer, &game->reflectionProbe);
	RenderReflectionProbe(&game->renderer, &game->reflectionProbe);

	/*
	// round counter
	{
		int numGroups = (game->round + 4) / 5;
		for (int i = 0; i < numGroups; i++)
		{
			int numStrikes = i < numGroups - 1 ? 5 : (game->round - 1) % 5 + 1;
			float alpha = game->roundStartTimer != -1 ? SDL_sinf(gameTime * 4) * 0.5f + 0.5f : 1;
			GUIPanel(i * game->roundCounter->info.height, app->height - game->roundCounter->info.height, game->roundCounter, ivec4((numStrikes - 1) * game->roundCounter->info.height, 0, game->roundCounter->info.height, game->roundCounter->info.height), vec4(0.5f, 0, 0, alpha));
		}
	}

	// point counter
	{
		int w = 120;
		int h = 32;
		int x = app->width - 20 - w;
		int y = app->height - 20 - h;
		int padding = 4;

		GUIPanel(x, y, w, h, vec4(0.1f, 0.1f, 0.1f, 1));

		char str[10];
		int len = SDL_snprintf(str, 10, "%d", game->points);
		for (int i = 0; i < len; i++)
		{
			int digit = str[i] - '0';
			int u = digit * 16;
			GUIPanel(x + w - padding - len * 16 + i * 16, y, game->digits, ivec4(u, 0, 16, 32), vec4(0.5f, 0, 0, 1));
		}
	}
	*/

	//DebugText(0, app->height / 16 - 1, COLOR_WHITE, COLOR_BLACK, "%d, %.2f, %.2f", game->round, game->roundStartTimer, game->gameOverTimer);
	DebugText(0, app->height / 16 - 1, COLOR_WHITE, COLOR_BLACK, "(%d,%d,%d) %d/%d hp",
		(int)(game->player.position.x * 100),
		(int)(game->player.position.y * 100),
		(int)(game->player.position.z * 100),
		game->player.health, game->player.maxHealth);
	//DebugText(0, app->height / 16 - 3, COLOR_WHITE, COLOR_BLACK, "%d entities in memory, %d skeletons remaining", game->entities.size, game->numSkeletonsRemaining);
}

void GameShowFrame(SDL_GPUCommandBuffer* cmdBuffer)
{
	vec3 sunDirection = quat::FromAxisAngle(vec3(0, 1, 2).normalized(), -2.5f * 0.1f) * vec3(1, 0, 0);
	//sunDirection.y = -fabsf(sunDirection.y - 0.2f) + 0.2f;
	//sunDirection = vec3(-1, -0.025f, 0).normalized();
	//sunDirection = vec3(0.5f, -1, -1).normalized();

	RendererShow(&game->renderer, game->cameraPosition, game->cameraRotation, game->cameraNear, game->cameraFov, app->width / (float)app->height, game->projection, game->view, game->pv, game->frustumPlanes, sunDirection, swapchain, cmdBuffer);

	mat4 guiProjectionView = mat4::Orthographic(0, (float)app->width, 0, (float)app->height, -1, 1);
	SetRenderer2DCamera(&game->guiRenderer, 0, guiProjectionView);
	EndRenderer2D(&game->guiRenderer, cmdBuffer);
}
