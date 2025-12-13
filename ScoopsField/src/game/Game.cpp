#include "graphics/VertexBuffer.h"

#include "model/Model.h"

#include "math/Vector.h"


extern SDL_GPUDevice* device;


static bool EveryInterval(float seconds)
{
	int iteration = (int)(gameTime / seconds);
	int lastIteration = (int)((gameTime - deltaTime) / seconds);
	return iteration != lastIteration || gameTime - deltaTime < 0;
}


#include "item/Item.cpp"
#include "player/Player.cpp"
#include "entity/Skeleton.cpp"


static void StartRound(int round)
{
	game->round = round;
	game->roundStartTimer = 0;
}

void GameInit(SDL_GPUCommandBuffer* cmdBuffer)
{
	InitRenderer(&game->renderer, width, height, cmdBuffer);

	Renderer2DLayerInfo layerInfo = {};
	layerInfo.width = width;
	layerInfo.height = height;
	layerInfo.maxSprites = 1000;
	layerInfo.textureFormat = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	InitRenderer2D(&game->guiRenderer, 1, &layerInfo, cmdBuffer);

	InitItemDatabase(&game->items, cmdBuffer);

	//game->mesh = LoadMesh("res/models/monkey.glb.bin", cmdBuffer);

	InitPlayer(&game->player, cmdBuffer);

	InitPool(&game->skeletons);

	game->round = 0;
	game->roundStartTimer = -1;

	StartRound(1);

	LoadModel(&game->cube, "res/models/cube.glb.bin", cmdBuffer);

	InitRigidBody(&game->platform, RIGID_BODY_STATIC, vec3::Zero, quat::Identity);
	AddBoxCollider(&game->platform, vec3(20, 2, 20), vec3(0, -1, 0), quat::Identity, 1, 1, false);

	LoadSound(&game->testSound, "res/sounds/test.ogg.bin");

	game->crosshair = LoadTexture("res/textures/crosshair.png.bin", cmdBuffer);

	game->cameraPosition = vec3(0, 0, 3);
	//game->cameraPitch = -0.4f * PI;
	//game->cameraYaw = 0.25f * PI;
	game->cameraNear = 0.1f;
	game->cameraFar = 1000;

	game->mouseLocked = true;
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
	/*
	if (FileHasChanged(PROJECT_PATH "/res/shaders/chunk.vert") || FileHasChanged(PROJECT_PATH "/res/shaders/chunk.frag"))
	{
		app->platformCallbacks.compileResources();
		ReloadGraphicsShader(game->chunkShader, "res/shaders/chunk.vert.bin", "res/shaders/chunk.frag.bin");
		ReloadGraphicsPipeline(game->chunkPipeline);
	}
	*/

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
			for (int i = 0; i < numSkeletons; i++)
			{
				vec2 position = game->random.nextVector2(-9, 9);
				SkeletonEntity* skeleton = PoolAlloc(&game->skeletons);
				SDL_assert(skeleton);
				InitSkeleton(skeleton, vec3(position.x, 0, position.y), 0);
			}

			game->numSkeletonsRemaining = numSkeletons;
		}
	}
	else if (game->numSkeletonsRemaining == 0)
	{
		StartRound(game->round + 1);
	}

	UpdatePlayer(&game->player);

	game->projection = mat4::Perspective(90 * Deg2Rad, width / (float)height, game->cameraNear, game->cameraFar);
	quat invCameraRotation = quat::FromAxisAngle(vec3::Right, -game->cameraPitch) * quat::FromAxisAngle(vec3::Up, -game->cameraYaw);
	game->view = mat4::Rotate(invCameraRotation) * mat4::Translate(-game->cameraPosition);
	game->pv = game->projection * game->view;
	GetFrustumPlanes(game->pv, game->frustumPlanes);

	for (int i = 0; i < game->skeletons.capacity; i++)
	{
		if (game->skeletons.occupied[i])
		{
			SkeletonEntity* skeleton = &game->skeletons.data[i];
			UpdateSkeleton(skeleton);
			if (skeleton->removed)
			{
				DestroySkeleton(skeleton);
				PoolRelease(&game->skeletons, skeleton);
			}
		}
	}

	if (GetKeyDown(SDL_SCANCODE_P))
		PlaySound(&game->testSound);
}

void GameRender()
{
	BeginRenderer2D(&game->guiRenderer);

	RenderPlayer(&game->player);

	for (int i = 0; i < game->skeletons.capacity; i++)
	{
		if (game->skeletons.occupied[i])
		{
			RenderSkeleton(&game->skeletons.data[i]);
		}
	}

	mat4 platformTransform = mat4::Transform(vec3(0, -1, 0), quat::Identity, vec3(20, 2, 20));
	RenderModel(&game->renderer, &game->cube, nullptr, platformTransform);

	RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Up, gameTime * PI) * vec3(2, 2, 0), vec3(1, 0.5f, 1) * 5);
	RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Right, gameTime * PI * 0.7f) * vec3(2, 2, 0), vec3(0.5f, 1, 0.5f) * 5);

	GUIPanel(width / 2 - game->crosshair->info.width / 2, height / 2 - game->crosshair->info.height / 2, game->crosshair);

	DebugText(0, height / 16 - 1, "%d, %.2f", game->round, game->roundStartTimer);
	DebugText(0, height / 16 - 2, "%d skeletons, %d in memory", game->numSkeletonsRemaining, game->skeletons.size);
}

void GameShowFrame(SDL_GPUCommandBuffer* cmdBuffer)
{
	RendererShow(&game->renderer, game->pv, game->frustumPlanes, game->cameraNear, game->cameraFar, cmdBuffer);

	mat4 guiProjectionView = mat4::Orthographic(0, (float)width, 0, (float)height, -1, 1);
	SetRenderer2DCamera(&game->guiRenderer, 0, guiProjectionView);
	EndRenderer2D(&game->guiRenderer, cmdBuffer);
}
