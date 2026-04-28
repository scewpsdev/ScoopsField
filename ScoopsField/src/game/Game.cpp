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


static void StartRound(int round)
{
	game->round = round;
	game->roundStartTimer = 0;
}

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

	game->round = 0;
	game->points = 0;
	game->roundStartTimer = -1;
	game->numSkeletonsRemaining = 0;
	game->gameOverTimer = -1;

	//game->ambientSource = PlaySound(&game->ambientSound, 0.5f);

	game->cameraPosition = vec3(0, 0, 3);
	//game->cameraPitch = -0.4f * PI;
	//game->cameraYaw = 0.25f * PI;
	game->cameraNear = 0.1f;
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

	StartRound(1);
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

	InitItemDatabase(&game->items, cmdBuffer);

	LoadModel(&game->cube, "res/models/cube.glb.bin", false, cmdBuffer);

	//LoadModel(&game->mapModel, "res/maps/testmap/testmap.gltf.bin", true, cmdBuffer);
	LoadModel(&game->mapModel, "res/maps/painted_world/painted_world.glb.bin", true, cmdBuffer);

	InitRigidBody(&game->mapCollider, RIGID_BODY_STATIC, vec3::Zero, quat::Identity);
	AddModelCollider(&game->mapCollider, &game->mapModel, vec3::Zero, quat::Identity, vec3::One, 1, 1, false);

	Model* navmeshModel = (Model*)BumpAllocatorMalloc(&memory->transientAllocator, sizeof(Model));
	LoadModel(navmeshModel, "res/maps/testmap/testmap_navmesh.glb.bin", true, cmdBuffer);
	InitNavmesh(&game->mapNavmesh, navmeshModel);

	LoadSound(&game->testSound, "res/sounds/test.ogg.bin");
	LoadSound(&game->ambientSound, "res/sounds/ambience.ogg.bin");
	for (int i = 0; i < 6; i++)
	{
		char path[64];
		SDL_snprintf(path, 64, "res/sounds/step%d.ogg.bin", i + 1);
		LoadSound(&game->stepSounds[i], path);
	}
	LoadSound(&game->landSound, "res/sounds/land.ogg.bin");
	for (int i = 0; i < 3; i++)
	{
		char path[64];
		SDL_snprintf(path, 64, "res/sounds/swing%d.ogg.bin", i + 1);
		LoadSound(&game->swingSounds[i], path);
	}
	for (int i = 0; i < 2; i++)
	{
		char path[64];
		SDL_snprintf(path, 64, "res/sounds/hit_slash%d.ogg.bin", i + 1);
		LoadSound(&game->slashHitSounds[i], path);
	}
	for (int i = 0; i < 5; i++)
	{
		char path[64];
		SDL_snprintf(path, 64, "res/sounds/hit_rock%d.ogg.bin", i + 1);
		LoadSound(&game->skeletonHitSounds[i], path);
	}
	for (int i = 0; i < 2; i++)
	{
		char path[64];
		SDL_snprintf(path, 64, "res/sounds/exhausted%d.ogg.bin", i + 1);
		LoadSound(&game->exhaustedSounds[i], path);
	}

	game->crosshair = LoadTexture("res/textures/ui/crosshair.png.bin", cmdBuffer);
	game->crosshairInteract = LoadTexture("res/textures/ui/crosshair_hand.png.bin", cmdBuffer);
	game->vignette = LoadTexture("res/textures/vignette.png.bin", cmdBuffer);
	game->roundCounter = LoadTexture("res/textures/counter.png.bin", cmdBuffer);
	game->digits = LoadTexture("res/textures/digits.png.bin", cmdBuffer);

	AddFileWatcher(PROJECT_PATH "/res/shaders/mesh.vert");
	AddFileWatcher(PROJECT_PATH "/res/shaders/mesh.frag");
	AddFileWatcher(PROJECT_PATH "/res/shaders/lighting/directional_light.vert");
	AddFileWatcher(PROJECT_PATH "/res/shaders/lighting/directional_light.frag");
	AddFileWatcher(PROJECT_PATH "/res/shaders/lighting/point_light.vert");
	AddFileWatcher(PROJECT_PATH "/res/shaders/lighting/point_light.frag");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky.glsl");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky.vert");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky.frag");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky_upsample.vert");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky_upsample.frag");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky_cube.vert");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sky_cube.frag");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/transmittance_lut.comp");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/multiscatter_lut.comp");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/skyview_lut.comp");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/sun_color.comp");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/clouds.glsl");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/cloud_noise.comp");
	AddFileWatcher(PROJECT_PATH "/res/shaders/sky/cloud_noise_detail.comp");

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

void GameUpdate()
{
	if (FileHasChanged(PROJECT_PATH "/res/shaders/mesh.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/mesh.frag"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->renderer.defaultShader, "res/shaders/mesh.vert.bin", "res/shaders/mesh.frag.bin");
		ReloadGraphicsPipeline(game->renderer.geometryPipeline);
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/lighting/directional_light.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/lighting/directional_light.frag"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->renderer.directionalLightShader, "res/shaders/lighting/directional_light.vert.bin", "res/shaders/lighting/directional_light.frag.bin");
		ReloadGraphicsPipeline(game->renderer.directionalLightPipeline);
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/lighting/point_light.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/lighting/point_light.frag"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->renderer.pointLightShader, "res/shaders/lighting/point_light.vert.bin", "res/shaders/lighting/point_light.frag.bin");
		ReloadGraphicsPipeline(game->renderer.pointLightPipeline);
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky.frag") || FileHasChanged(PROJECT_PATH "/res/shaders/sky/clouds.glsl"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->renderer.skyShader, "res/shaders/sky/sky.vert.bin", "res/shaders/sky/sky.frag.bin");
		ReloadGraphicsPipeline(game->renderer.skyPipeline);
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky_upsample.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky_upsample.frag"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->renderer.skyUpsampleShader, "res/shaders/sky/sky_upsample.vert.bin", "res/shaders/sky/sky_upsample.frag.bin");
		ReloadGraphicsPipeline(game->renderer.skyUpsamplePipeline);
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky_cube.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky_cube.frag"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->renderer.skyCubeShader, "res/shaders/sky/sky_cube.vert.bin", "res/shaders/sky/sky_cube.frag.bin");
		ReloadGraphicsPipeline(game->renderer.skyCubePipeline);
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/transmittance_lut.comp"))
	{
		app->platformCallbacks.compileResources();
		ReloadComputeShader(game->renderer.skyTransmittaceLUTShader, "res/shaders/sky/transmittance_lut.comp.bin");
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/multiscatter_lut.comp"))
	{
		app->platformCallbacks.compileResources();
		ReloadComputeShader(game->renderer.skyMultiScatterLUTShader, "res/shaders/sky/multiscatter_lut.comp.bin");
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/skyview_lut.comp") || FileHasChanged(PROJECT_PATH "/res/shaders/sky/sky.glsl"))
	{
		app->platformCallbacks.compileResources();
		ReloadComputeShader(game->renderer.skyViewLUTShader, "res/shaders/sky/skyview_lut.comp.bin");
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/sun_color.comp"))
	{
		app->platformCallbacks.compileResources();
		ReloadComputeShader(game->renderer.sunColorShader, "res/shaders/sky/sun_color.comp.bin");
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/cloud_noise.comp"))
	{
		app->platformCallbacks.compileResources();
		ReloadComputeShader(game->renderer.cloudNoiseShader, "res/shaders/sky/cloud_noise.comp.bin");
	}
	if (FileHasChanged(PROJECT_PATH "/res/shaders/sky/cloud_noise_detail.comp"))
	{
		app->platformCallbacks.compileResources();
		ReloadComputeShader(game->renderer.cloudNoiseDetailShader, "res/shaders/sky/cloud_noise_detail.comp.bin");
	}

	if (app->keys[SDL_SCANCODE_ESCAPE] && !app->lastKeys[SDL_SCANCODE_ESCAPE])
		game->mouseLocked = !game->mouseLocked;

	SDL_SetWindowRelativeMouseMode(window, game->mouseLocked);

	if (game->roundStartTimer != -1)
	{
		game->roundStartTimer += deltaTime;
		if (game->roundStartTimer >= ROUND_START_DELAY)
		{
			game->roundStartTimer = -1;

			// init round
			int numSkeletons = 3 + game->round * 2;
			int skeletonHealth = 60 + game->round * 10;
			for (int i = 0; i < numSkeletons; i++)
			{
				vec2 position = game->random.nextVector2(-9, 9);
				Entity* skeleton = PoolAlloc(&game->entities);
				SDL_assert(skeleton);
				InitSkeleton(skeleton, vec3(position.x, 0, position.y), 0, skeletonHealth);
			}

			game->numSkeletonsRemaining = numSkeletons;
		}
	}
	else if (game->numSkeletonsRemaining == 0)
	{
		StartRound(game->round + 1);
	}

	if (game->gameOverTimer != -1)
	{
		game->gameOverTimer += deltaTime;
		if (game->gameOverTimer >= GAME_OVER_DELAY)
		{
			game->gameOverTimer = -1;
			ResetGame(true);
			return;
		}
	}

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

	float fov = GetKey(SDL_SCANCODE_C) ? 30.0f : 90.0f;
	game->projection = mat4::Perspective(fov * Deg2Rad, app->width / (float)app->height, game->cameraNear);
	game->view = mat4::Rotate(game->cameraRotation.conjugated()) * mat4::Translate(-game->cameraPosition);
	game->pv = game->projection * game->view;
	GetFrustumPlanes(game->pv, game->frustumPlanes);

	vec3 test = vec3(0, 0, -5);
	vec4 a = game->projection * vec4(test, 1);

	if (GetKeyDown(SDL_SCANCODE_P))
	{
		PlaySound(&game->testSound);

		Entity* kingsSword = PoolAlloc(&game->entities);
		InitItemEntity(kingsSword, GetItem(ITEM_TYPE_KINGS_SWORD), vec3(0, 2, 0), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));
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

	DebugText(0, app->height / 16 - 1, COLOR_WHITE, COLOR_BLACK, "%d, %.2f, %.2f", game->round, game->roundStartTimer, game->gameOverTimer);
	DebugText(0, app->height / 16 - 2, COLOR_WHITE, COLOR_BLACK, "%d/%d hp", game->player.health, game->player.maxHealth);
	DebugText(0, app->height / 16 - 3, COLOR_WHITE, COLOR_BLACK, "%d entities in memory, %d skeletons remaining", game->entities.size, game->numSkeletonsRemaining);
}

void GameShowFrame(SDL_GPUCommandBuffer* cmdBuffer)
{
	vec3 sunDirection = quat::FromAxisAngle(vec3(0, 1, 2).normalized(), -gameTime * 0.01f) * vec3(1, 0, 0);
	//sunDirection.y = -fabsf(sunDirection.y - 0.2f) + 0.2f;
	//sunDirection = vec3(-1, -0.025f, 0).normalized();

	RendererShow(&game->renderer, game->cameraPosition, game->projection, game->view, game->pv, game->frustumPlanes, game->cameraNear, sunDirection, swapchain, cmdBuffer);

	mat4 guiProjectionView = mat4::Orthographic(0, (float)app->width, 0, (float)app->height, -1, 1);
	SetRenderer2DCamera(&game->guiRenderer, 0, guiProjectionView);
	EndRenderer2D(&game->guiRenderer, cmdBuffer);
}
