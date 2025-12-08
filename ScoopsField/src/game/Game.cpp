#include "graphics/VertexBuffer.h"

#include "model/Model.h"

#include "math/Vector.h"


extern SDL_GPUDevice* device;


void GameInit(SDL_GPUCommandBuffer* cmdBuffer)
{
	InitRenderer(&game->renderer, width, height, cmdBuffer);

	InitAnimationCache(&resource->animationCache);

	//game->mesh = LoadMesh("res/models/monkey.glb.bin", cmdBuffer);
	LoadModel(&game->model, "res/models/Fox.glb.bin", cmdBuffer);
	InitAnimationState(&game->modelAnim, &game->model);

	game->cameraPosition = vec3(0, 1, 3);
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

	Quaternion cameraRotation = Quaternion::FromAxisAngle(vec3::Up, game->cameraYaw) * Quaternion::FromAxisAngle(vec3::Right, game->cameraPitch);

	vec3 delta = vec3::Zero;
	if (app->keys[SDL_SCANCODE_A]) delta += cameraRotation.left();
	if (app->keys[SDL_SCANCODE_D]) delta += cameraRotation.right();
	if (app->keys[SDL_SCANCODE_S]) delta += cameraRotation.back();
	if (app->keys[SDL_SCANCODE_W]) delta += cameraRotation.forward();
	if (app->keys[SDL_SCANCODE_SPACE]) delta += vec3::Up;
	if (app->keys[SDL_SCANCODE_LCTRL]) delta += vec3::Down;

	if (delta.lengthSquared() > 0)
	{
		float speed = app->keys[SDL_SCANCODE_LSHIFT] ? 20.0f : app->keys[SDL_SCANCODE_LALT] ? 3.0f : 10.0f;
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

	AnimateModel(&game->model, &game->modelAnim, &game->model.animations[2], gameTime);
}

void GameRender(SDL_GPUCommandBuffer* cmdBuffer)
{
	RenderModel(&game->renderer, &game->model, &game->modelAnim, mat4::Scale(0.01f));
	RenderLight(&game->renderer, Quaternion::FromAxisAngle(vec3::Up, gameTime * PI) * vec3(2, 2, 0), vec3(1, 0.5f, 1));
	RenderLight(&game->renderer, Quaternion::FromAxisAngle(vec3::Right, gameTime * PI * 0.7f) * vec3(2, 2, 0), vec3(0.5f, 1, 0.5f));

	mat4 projection = mat4::Perspective(90 * Deg2Rad, width / (float)height, game->cameraNear, game->cameraFar);
	Quaternion invCameraRotation = Quaternion::FromAxisAngle(vec3::Right, -game->cameraPitch) * Quaternion::FromAxisAngle(vec3::Up, -game->cameraYaw);
	mat4 view = mat4::Rotate(invCameraRotation) * mat4::Translate(-game->cameraPosition);

	RendererShow(&game->renderer, projection, view, game->cameraNear, game->cameraFar, cmdBuffer);
}
