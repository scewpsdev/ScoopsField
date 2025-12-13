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
#include "entity/Player.cpp"


void GameInit(SDL_GPUCommandBuffer* cmdBuffer)
{
	InitRenderer(&game->renderer, width, height, cmdBuffer);

	Renderer2DLayerInfo layerInfo = {};
	layerInfo.width = width;
	layerInfo.height = height;
	layerInfo.maxSprites = 1000;
	layerInfo.textureFormat = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	InitRenderer2D(&game->guiRenderer, 1, &layerInfo, cmdBuffer);

	InitAnimationCache(&resource->animationCache);

	InitItemDatabase(&game->items, cmdBuffer);

	//game->mesh = LoadMesh("res/models/monkey.glb.bin", cmdBuffer);

	InitPlayer(&game->player, cmdBuffer);

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

	UpdatePlayer(&game->player);

	if (EveryInterval(1))
	{
		RigidBody* body = &game->cubeBodies[game->numCubeBodies++];
		InitRigidBody(body, RIGID_BODY_DYNAMIC, vec3(0, 10, 0), quat::FromEulers(vec3(342, 5, 1)));
		AddBoxCollider(body, vec3(1), vec3::Zero, quat::Identity, 1, 1, false);
	}

	if (app->keys[SDL_SCANCODE_P] && !app->lastKeys[SDL_SCANCODE_P])
		PlaySound(&game->testSound);
}

void GameRender()
{
	BeginRenderer2D(&game->guiRenderer);

	RenderPlayer(&game->player);

	mat4 platformTransform = mat4::Transform(vec3(0, -1, 0), quat::Identity, vec3(20, 2, 20));
	RenderModel(&game->renderer, &game->cube, nullptr, platformTransform);

	for (int i = 0; i < game->numCubeBodies; i++)
	{
		vec3 cubePosition;
		quat cubeRotation;
		GetRigidBodyTransform(&game->cubeBodies[i], &cubePosition, &cubeRotation);
		mat4 cubeTransform = mat4::Transform(cubePosition, cubeRotation, vec3::One);
		RenderModel(&game->renderer, &game->cube, nullptr, cubeTransform);
	}

	RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Up, gameTime * PI) * vec3(2, 2, 0), vec3(1, 0.5f, 1) * 5);
	RenderLight(&game->renderer, quat::FromAxisAngle(vec3::Right, gameTime * PI * 0.7f) * vec3(2, 2, 0), vec3(0.5f, 1, 0.5f) * 5);

	GUIPanel(width / 2 - game->crosshair->info.width / 2, height / 2 - game->crosshair->info.height / 2, game->crosshair);

	DebugText(0, height / 16 - 1, "%.2f, %.2f, %.2f", game->cameraPosition.x, game->cameraPosition.y, game->cameraPosition.z);
}

void GameShowFrame(SDL_GPUCommandBuffer* cmdBuffer)
{
	mat4 projection = mat4::Perspective(90 * Deg2Rad, width / (float)height, game->cameraNear, game->cameraFar);
	quat invCameraRotation = quat::FromAxisAngle(vec3::Right, -game->cameraPitch) * quat::FromAxisAngle(vec3::Up, -game->cameraYaw);
	mat4 view = mat4::Rotate(invCameraRotation) * mat4::Translate(-game->cameraPosition);

	RendererShow(&game->renderer, projection, view, game->cameraNear, game->cameraFar, cmdBuffer);

	mat4 guiProjectionView = mat4::Orthographic(0, (float)width, 0, (float)height, -1, 1);
	SetRenderer2DCamera(&game->guiRenderer, 0, guiProjectionView);
	EndRenderer2D(&game->guiRenderer, cmdBuffer);
}
