#include "graphics/VertexBuffer.h"

#include "math/Vector.h"


void GameInit()
{
	InitRenderer(&game->renderer, width, height);

	game->cameraPosition = vec3(0, 16, 32);
	//game->cameraPitch = -0.4f * PI;
	//game->cameraYaw = 0.25f * PI;

	game->mouseLocked = true;
}

void GameDestroy()
{
	DestroyRenderer(&game->renderer);
}

void GameResize(int newWidth, int newHeight)
{
	ResizeRenderer(&game->renderer, newWidth, newHeight);
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

	vec3 delta = vec3::Zero;
	if (app->keys[SDL_SCANCODE_A]) delta += game->cameraRotation.left();
	if (app->keys[SDL_SCANCODE_D]) delta += game->cameraRotation.right();
	if (app->keys[SDL_SCANCODE_S]) delta += game->cameraRotation.back();
	if (app->keys[SDL_SCANCODE_W]) delta += game->cameraRotation.forward();
	if (app->keys[SDL_SCANCODE_SPACE]) delta += vec3::Up;
	if (app->keys[SDL_SCANCODE_LCTRL]) delta += vec3::Down;

	if (delta.lengthSquared() > 0)
	{
		float speed = app->keys[SDL_SCANCODE_LSHIFT] ? 100.0f : app->keys[SDL_SCANCODE_LALT] ? 10.0f : 40.0f;
		vec3 velocity = delta.normalized() * speed;
		vec3 displacement = velocity * deltaTime;
		game->cameraPosition += displacement;
	}

	if (app->keys[SDL_SCANCODE_ESCAPE] && !app->lastKeys[SDL_SCANCODE_ESCAPE])
		game->mouseLocked = !game->mouseLocked;

	SDL_SetWindowRelativeMouseMode(window, game->mouseLocked);

	if (game->mouseLocked)
	{
		game->cameraYaw -= app->mouseDelta.x * 0.001f;
		game->cameraPitch -= app->mouseDelta.y * 0.001f;
	}

	game->cameraRotation = Quaternion::FromAxisAngle(vec3::Up, game->cameraYaw) * Quaternion::FromAxisAngle(vec3::Right, game->cameraPitch);
}

void GameRender(SDL_GPUCommandBuffer* cmdBuffer)
{
	RenderScene(&game->renderer, game, cmdBuffer);
}
