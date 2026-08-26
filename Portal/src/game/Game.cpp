#include "graphics/VertexBuffer.h"

#include "model/Model.h"

#include "math/Vector.h"


#define ROUND_START_DELAY 500.0f
#define GAME_OVER_DELAY 5.0f


extern SDL_GPUDevice* device;


Entity* CreateEntity()
{
	Entity* entity = PoolAlloc(&game->entities);
	SDL_memset(entity, 0, sizeof(Entity));
	return entity;
}


#include "item/Item.cpp"
#include "particle/ParticleSystem.cpp"
#include "player/Player.cpp"
#include "entity/component/Creature.cpp"
#include "entity/component/ItemEntity.cpp"
#include "entity/component/RestingSpot.cpp"
#include "entity/component/Projectile.cpp"
#include "entity/component/Trail.cpp"
#include "entity/component/Ragdoll.cpp"
#include "entity/component/Sconce.cpp"
#include "entity/component/Elevator.cpp"
#include "entity/component/Portal.cpp"


static void ResetGame(bool destroy, bool init)
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

		for (int i = 0; i < game->numReflectionProbes; i++)
		{
			DestroyReflectionProbe(&game->reflectionProbes[i]);
		}
		game->numReflectionProbes = 0;

		DestroyModel(&game->mapModel);
		DestroyRigidBody(&game->mapCollider);
	}


	if (init)
	{
		game->gameTime = 0.0f;

		//game->ambientSource = PlaySound(&game->ambientSound, 0.5f);

		game->cameraPosition = vec3(0, 0, 3);
		//game->cameraPitch = -0.4f * PI;
		//game->cameraYaw = 0.25f * PI;
		game->cameraNear = 0.01f;
		//game->cameraFar = 1000;

		game->mouseLocked = true;

		game->playerSpawn = mat4::Identity;

		InitPool(&game->entities);

		//LoadModel(&game->mapModel, "res/maps/painted_world/painted_world.glb.bin", true, cmdBuffer);
		//LoadModel(&game->mapModel, "res/maps/testmap/testmap.glb.bin", false, cmdBuffer);
		LoadModel(&game->mapModel, "res/maps/testmap/testmap.glb.bin", false, cmdBuffer);

		Model* mapCollider = (Model*)BumpAllocatorMalloc(&memory->transientAllocator, sizeof(Model));
		LoadModel(mapCollider, "res/maps/testmap/testmap_collider.glb.bin", true, cmdBuffer);

		InitRigidBody(&game->mapCollider, RIGID_BODY_STATIC, vec3::Zero, quat::Identity, nullptr);
		AddModelCollider(&game->mapCollider, mapCollider, vec3::Zero, quat::Identity, vec3::One, 1, 1, false);

		DestroyModel(mapCollider);

		//Model* navmeshModel = (Model*)BumpAllocatorMalloc(&memory->transientAllocator, sizeof(Model));
		//LoadModel(navmeshModel, "res/maps/testmap/testmap_navmesh.glb.bin", true, cmdBuffer);
		//InitNavmesh(&game->mapNavmesh, navmeshModel);

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
			else if (SDL_strncmp(node->name, "Reflection", 10) == 0)
			{
				vec3 position = node->transform.translation();
				vec3 size = node->transform.scale();
				InitReflectionProbe(&game->reflectionProbes[game->numReflectionProbes++], position, size);
			}
		}

		Portal* portal1, * portal2;
		InitPortal(portal1 = (Portal*)CreateEntity(), vec3(5, 0, -5), quat::Identity);
		InitPortal(portal2 = (Portal*)CreateEntity(), vec3(8, 0, -11), quat::FromAxisAngle(vec3::Up, -0.5f * PI));
		portal1->destination = portal2;
		portal2->destination = portal1;


		InitPlayer(&game->player, cmdBuffer, game->playerSpawn.translation(), game->playerSpawn.rotation().getAngle());

		//InitKnight((Creature*)CreateEntity(), vec3(0, 0, -5), 0);
		//InitKnight((Creature*)CreateEntity(), vec3(-4, 0, -5), 0);
		//InitKnight((Creature*)CreateEntity(), vec3(4, 0, -5), 0);

		//InitSconce((Sconce*)CreateEntity(), vec3(-7, -37, 40));
		//InitSconce((Sconce*)CreateEntity(), vec3(7, -37, 40));
		//InitSconce((Sconce*)CreateEntity(), vec3(-7, -37, 54));
		//InitSconce((Sconce*)CreateEntity(), vec3(7, -37, 54));

		//InitElevator((Elevator*)CreateEntity(), vec3(0, -37, 47), quat::Identity, 37);

		//InitItemEntity((ItemEntity*)CreateEntity(), GetItem(ITEM_KINGS_SWORD), vec3(-2, 2, -2), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));
		//InitItemEntity((ItemEntity*)CreateEntity(), GetItem(ITEM_LONGSWORD), vec3(0, 2, -2), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));
		//InitItemEntity((ItemEntity*)CreateEntity(), GetItem(ITEM_DARKWOOD_STAFF), vec3(2, 2, -2), quat::FromAxisAngle(vec3(1, 1, 1).normalized(), 13242));

		//InitReflectionProbe(&game->reflectionProbes[game->numReflectionProbes++], vec3(0, 29, -58), vec3(9, 13, 9));
		//InitReflectionProbe(&game->reflectionProbes[game->numReflectionProbes++], vec3(0, -15.5f, 47), vec3(4, 14.5f, 4));
		//InitReflectionProbe(&game->reflectionProbes[game->numReflectionProbes++], vec3(0, -33.5f, 47), vec3(9, 3.5f, 9));
		//InitReflectionProbe(&game->reflectionProbes[game->numReflectionProbes++], vec3(0, -35, 73), vec3(3, 2, 16));

		//{
		//	ParticleEffect* effect = (ParticleEffect*)CreateEntity();
		//	LoadParticleEffect(effect, "effects/testeffect/testeffect.rfs", vec3(0, 2, 0), quat::Identity);
		//}
	}
}

void GameInit(SDL_GPUCommandBuffer* cmdBuffer)
{
	InitRenderer(&game->renderer, app->width, app->height, cmdBuffer);

	game->guiShader = LoadGraphicsShader("res/shaders/sprite.vert.bin", "res/shaders/sprite.frag.bin");

	{
		VertexBufferLayout spriteVertexLayout = {};
		spriteVertexLayout.numAttributes = 1;
		spriteVertexLayout.attributes[0].location = 0;
		spriteVertexLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

		GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(
			SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			SDL_GPU_CULLMODE_BACK,
			game->guiShader,
			nullptr,
			1, &spriteVertexLayout
		);

		CreateBlendStateAlphaPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

		game->guiPipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	InitSpriteRenderer(&game->guiRenderer, 1000, game->guiPipeline, cmdBuffer);

	game->textShader = LoadGraphicsShader("res/shaders/sprite.vert.bin", "res/shaders/text.frag.bin");

	{
		VertexBufferLayout spriteVertexLayout = {};
		spriteVertexLayout.numAttributes = 1;
		spriteVertexLayout.attributes[0].location = 0;
		spriteVertexLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

		GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(
			SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			SDL_GPU_CULLMODE_BACK,
			game->textShader,
			nullptr,
			1, &spriteVertexLayout
		);

		CreateBlendStateAlphaPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

		game->textPipeline = CreateGraphicsPipeline(&pipelineInfo);
	}

	InitSpriteRenderer(&game->textRenderer, 1000, game->textPipeline, cmdBuffer);

	InitRandom(&game->random, (uint32_t)SDL_GetTicks());

	InitParticleSystem(&game->particles);

	LoadModel(&game->cube, "res/models/cube.glb.bin", false, cmdBuffer);

	LoadSound(&game->ambientSound, "res/sounds/ambience.ogg.bin");
	LoadSounds(&game->stepSound, "sounds/step", 6);
	LoadSound(&game->landSound, "res/sounds/land.ogg.bin");
	LoadSounds(&game->exhaustedSound, "sounds/exhausted", 2);
	LoadSounds(&game->swingSound, "sounds/swing", 3);
	LoadSounds(&game->armorSound, "sounds/armor", 10);
	LoadSounds(&game->hitSlashSound, "sounds/hit/hit_slash", 2);
	LoadSounds(&game->hitSkeletonSound, "sounds/hit/hit_rock", 5);
	LoadSounds(&game->hitArmorSound, "sounds/hit/hit_armor", 10);
	LoadSound(&game->hitArrowSound, "res/sounds/hit/hit_arrow.ogg.bin");
	LoadSounds(&game->hitBlockSound, "sounds/hit/hit_block", 2);
	LoadSound(&game->hitParrySound, "res/sounds/hit/hit_parry.ogg.bin");
	LoadSound(&game->hitShieldSound, "res/sounds/hit/hit_shield.ogg.bin");
	LoadSound(&game->hitShieldParrySound, "res/sounds/hit/hit_shield_parry.ogg.bin");
	LoadSounds(&game->stepBareSound, "sounds/step/step_bare", 3);
	LoadSound(&game->jumpBareSound, "res/sounds/step/jump_bare.ogg.bin");
	LoadSounds(&game->landBareSound, "sounds/step/land_bare", 3);
	LoadSound(&game->fireSound, "res/sounds/fire.ogg.bin");

	game->crosshair = LoadTexture("res/textures/ui/crosshair.png.bin", cmdBuffer);
	game->crosshairInteract = LoadTexture("res/textures/ui/crosshair_hand.png.bin", cmdBuffer);
	game->hitmarker = LoadTexture("res/textures/ui/hitmarker.png.bin", cmdBuffer);
	game->blockmarker = LoadTexture("res/textures/ui/blockmarker.png.bin", cmdBuffer);
	game->vignette = LoadTexture("res/textures/vignette.png.bin", cmdBuffer);
	game->roundCounter = LoadTexture("res/textures/counter.png.bin", cmdBuffer);
	game->digits = LoadTexture("res/textures/digits.png.bin", cmdBuffer);

	game->magicProjectileShader = CreateForwardGraphicsPipeline(
		LoadGraphicsShader("res/shaders/entity/magic_projectile.vert.bin", "res/shaders/entity/magic_projectile.frag.bin"),
		game->renderer.meshLayout, NUM_MESH_BUFFER_LAYOUTS, SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_NONE, false);

	VertexBufferLayout trailLayout = {};
	InitTrailVertexLayout(&trailLayout);
	game->trailShader = CreateForwardGraphicsPipeline(
		LoadGraphicsShader("res/shaders/entity/trail.vert.bin", "res/shaders/entity/trail.frag.bin"),
		&trailLayout, 1, SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, SDL_GPU_CULLMODE_NONE, false);
	game->trailAdditiveShader = CreateForwardGraphicsPipeline(
		LoadGraphicsShader("res/shaders/entity/trail.vert.bin", "res/shaders/entity/trail.frag.bin"),
		&trailLayout, 1, SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, SDL_GPU_CULLMODE_NONE, true);

	VertexBufferLayout particleLayouts[6];
	particleLayouts[0] = game->particles.quad->layout;
	InitParticleInstanceBufferLayouts(&particleLayouts[1]);
	Shader* particleShader = LoadGraphicsShader("res/shaders/entity/particle.vert.bin", "res/shaders/entity/particle.frag.bin");
	game->particleShader = CreateForwardGraphicsPipeline(particleShader, particleLayouts, 6, SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, SDL_GPU_CULLMODE_BACK, false);
	game->particleAdditiveShader = CreateForwardGraphicsPipeline(particleShader, particleLayouts, 6, SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, SDL_GPU_CULLMODE_BACK, true);

	LoadFont("default", "fonts/libre-baskerville.regular.ttf");
	game->font = GetFont("default", 20);

#ifdef _DEBUG
	AddHotReloadedShader("shaders/mesh.vert", "shaders/mesh.frag", game->renderer.defaultShader, game->renderer.geometryPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/shadow.frag", game->renderer.shadowShader, game->renderer.shadowPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/blurh.frag", game->renderer.blurHShader, game->renderer.blurHPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/blurv.frag", game->renderer.blurVShader, game->renderer.blurVPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/directional_light.frag", game->renderer.directionalLightShader, game->renderer.directionalLightPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/environment_light.frag", game->renderer.environmentLightShader, game->renderer.environmentLightPipeline);
	AddHotReloadedShader("shaders/lighting/point_light.vert", "shaders/lighting/point_light.frag", game->renderer.pointLightShader, game->renderer.pointLightPipeline);
	AddHotReloadedShader("shaders/lighting/reflection_probe.vert", "shaders/lighting/reflection_probe.frag", game->renderer.reflectionProbeShader, game->renderer.reflectionProbePipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/sky/sky.frag", "shaders/sky/clouds.glsl", game->renderer.skyShader, game->renderer.skyPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/sky/sky_upsample.frag", game->renderer.skyUpsampleShader, game->renderer.skyUpsamplePipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/sky/sky_cube.frag", game->renderer.skyCubeShader, game->renderer.skyCubePipeline);
	AddHotReloadedComputeShader("shaders/sky/transmittance_lut.comp", nullptr, game->renderer.skyTransmittanceLUTShader);
	AddHotReloadedComputeShader("shaders/sky/multiscatter_lut.comp", nullptr, game->renderer.skyMultiScatterLUTShader);
	AddHotReloadedComputeShader("shaders/sky/skyview_lut.comp", "shaders/sky/sky.glsl", game->renderer.skyViewLUTShader);
	AddHotReloadedComputeShader("shaders/sky/sun_color.comp", nullptr, game->renderer.sunColorShader);
	AddHotReloadedComputeShader("shaders/sky/weather_map.comp", nullptr, game->renderer.weatherMapShader);
	AddHotReloadedComputeShader("shaders/sky/cloud_noise.comp", nullptr, game->renderer.cloudNoiseShader);
	AddHotReloadedComputeShader("shaders/sky/cloud_noise_detail.comp", nullptr, game->renderer.cloudNoiseDetailShader);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/tonemapping.frag", game->renderer.tonemappingShader, game->renderer.tonemappingPipeline);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/lighting/deferred_diffuse.frag", game->renderer.deferredDiffuseShader, game->renderer.deferredDiffusePipeline);
	AddHotReloadedShader("shaders/entity/magic_projectile.vert", "shaders/entity/magic_projectile.frag", game->magicProjectileShader->pipelineInfo.shader, game->magicProjectileShader);
	AddHotReloadedShader("shaders/entity/particle.vert", "shaders/entity/particle.frag", game->particleShader->pipelineInfo.shader, game->particleShader);
	AddHotReloadedComputeShader("shaders/postprocessing/ssao.comp", nullptr, game->renderer.ssaoShader);
	AddHotReloadedComputeShader("shaders/postprocessing/ssao_blur.comp", nullptr, game->renderer.ssaoBlurShader);
	AddHotReloadedComputeShader("shaders/postprocessing/bloom_downsample.comp", nullptr, game->renderer.bloomDownsampleShader);
	AddHotReloadedComputeShader("shaders/postprocessing/bloom_upsample.comp", nullptr, game->renderer.bloomUpsampleShader);
	AddHotReloadedComputeShader("shaders/lighting/specular_convolute.comp", nullptr, game->renderer.specularConvoluteShader);
	AddHotReloadedShader("shaders/screenquad.vert", "shaders/depth_downsample.frag", game->renderer.depthDownsampleShader, game->renderer.depthDownsamplePipeline);
#endif

	InitItemDatabase(&game->items, cmdBuffer);

	ResetGame(false, true);
}

void GameDestroy()
{
	ResetGame(true, false);

	DestroyRenderer(&game->renderer);
	DestroySpriteRenderer(&game->guiRenderer);
	DestroyShader(game->guiShader);
	DestroyGraphicsPipeline(game->guiPipeline);
	DestroySpriteRenderer(&game->textRenderer);
	DestroyShader(game->textShader);
	DestroyGraphicsPipeline(game->textPipeline);

	DestroyParticleSystem(&game->particles);

	DestroyModel(&game->cube);

	DestroySound(&game->ambientSound);
	DestroySound(&game->stepSound);
	DestroySound(&game->landSound);
	DestroySound(&game->exhaustedSound);
	DestroySound(&game->swingSound);
	DestroySound(&game->armorSound);
	DestroySound(&game->hitSlashSound);
	DestroySound(&game->hitSkeletonSound);
	DestroySound(&game->hitArmorSound);
	DestroySound(&game->hitArrowSound);
	DestroySound(&game->hitBlockSound);
	DestroySound(&game->hitParrySound);
	DestroySound(&game->stepBareSound);
	DestroySound(&game->jumpBareSound);
	DestroySound(&game->landBareSound);
	DestroySound(&game->fireSound);

	DestroyTexture(game->crosshair);
	DestroyTexture(game->crosshairInteract);
	DestroyTexture(game->hitmarker);
	DestroyTexture(game->blockmarker);
	DestroyTexture(game->vignette);
	DestroyTexture(game->roundCounter);
	DestroyTexture(game->digits);

	DestroyGraphicsPipeline(game->magicProjectileShader);
	DestroyGraphicsPipeline(game->trailShader);
	DestroyGraphicsPipeline(game->trailAdditiveShader);
	DestroyGraphicsPipeline(game->particleShader);
	DestroyGraphicsPipeline(game->particleAdditiveShader);

	DestroyItemDatabase(&game->items);
}

void GameResize(int newWidth, int newHeight)
{
	ResizeRenderer(&game->renderer, newWidth, newHeight);
}

static bool cameraZoom = false;
void GameUpdate()
{
	game->gameTime += deltaTime;
	gameTime = game->gameTime;

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
			if (entity->removed || entity->position.y < -200)
			{
				DestroyEntity(entity);
				PoolRelease(&game->entities, entity);
			}
		}
	}

	UpdateParticleSystem(&game->particles);

	if (GetKeyDown(SDL_SCANCODE_C))
		cameraZoom = !cameraZoom;
	game->cameraFov = cameraZoom ? 30.0f : 90.0f;
	game->projection = mat4::Perspective(game->cameraFov * Deg2Rad, app->width / (float)app->height, game->cameraNear);
	game->view = mat4::Rotate(game->cameraRotation.conjugated()) * mat4::Translate(-game->cameraPosition);
	game->pv = game->projection * game->view;
	GetFrustumPlanes(game->pv, game->frustumPlanes);

	// TODO discard fragments in front of reflection probe volume

	if (GetKeyDown(SDL_SCANCODE_R) || game->player.position.y < -200)
	{
		ResetGame(true, false);
		CompileResources();
		ResetGame(false, true);
	}

	if (GetKeyDown(SDL_SCANCODE_F9))
	{
		__debugbreak();
	}
}

void GameRender()
{
	mat4 guiPV = mat4::Orthographic(0, (float)app->width, 0, (float)app->height, -1, 1);
	BeginSpriteRenderer(&game->guiRenderer, guiPV);
	BeginSpriteRenderer(&game->textRenderer, guiPV);

	RenderPlayer(&game->player);

	for (int i = 0; i < game->entities.capacity; i++)
	{
		if (game->entities.occupied[i])
		{
			RenderEntity(&game->entities.data[i]);
		}
	}

	RenderParticleSystem(&game->particles);

	RenderModel(&game->renderer, &game->mapModel, nullptr, mat4::Identity, true);

	//RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Up, 1 * 0.5f * PI) * vec3(2, 2, 0), vec3(1, 0.5f, 1) * 1);
	//RenderLight(&game->renderer, vec3(5, 2, -25) + quat::FromAxisAngle(vec3::Up, gameTime * 0.5f * PI) * vec3(-8, 0, 0), vec3(0.5f, 1, 0.5f));
	//RenderModel(&game->renderer, &game->cube, mat4::Translate(vec3(5, 2, -25) + quat::FromAxisAngle(vec3::Up, gameTime * 0.5f * PI) * vec3(-8, 0, 0)));

	for (int i = 0; i < game->numReflectionProbes; i++)
	{
		UpdateReflectionProbe(&game->renderer, &game->reflectionProbes[i]);
		RenderReflectionProbe(&game->renderer, &game->reflectionProbes[i]);
	}

	for (int i = 0; i < game->mapModel.numLights; i++)
	{
		Light* light = &game->mapModel.lights[i];
		Node* node = GetNodeByName(&game->mapModel, light->name);
		vec3 position = node->transform * light->position;
		RenderLight(&game->renderer, position, light->color);
	}

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
	DebugText(0, app->height / 16 - 2, COLOR_WHITE, COLOR_BLACK, "%d/%d entities", game->entities.size, game->entities.capacity);
}

void GameShowFrame(SDL_GPUCommandBuffer* cmdBuffer)
{
	vec3 sunDirection = quat::FromAxisAngle(vec3(0, 1, 2).normalized(), -20 * 0.03f) * vec3(1, 0, 0);
	//sunDirection.y = -fabsf(sunDirection.y - 0.2f) + 0.2f;
	//sunDirection = vec3(-1, -0.025f, 0).normalized();
	//sunDirection = vec3(0.5f, -1, -1).normalized();

	RendererShow(&game->renderer, game->cameraPosition, game->cameraRotation, game->cameraNear, game->cameraFov, app->width / (float)app->height, game->projection, game->view, game->pv, game->frustumPlanes, sunDirection, swapchain, cmdBuffer);

	//DrawText(&game->textRenderer, 100, 100, "abcdefABCDEF", 12, game->font, 0xFFFF77FF);

	EndSpriteRenderer(&game->guiRenderer, cmdBuffer);
	EndSpriteRenderer(&game->textRenderer, cmdBuffer);
}
