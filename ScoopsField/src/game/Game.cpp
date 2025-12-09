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

void GameInit(SDL_GPUCommandBuffer* cmdBuffer)
{
	InitRenderer(&game->renderer, width, height, cmdBuffer);

	InitAnimationCache(&resource->animationCache);

	//game->mesh = LoadMesh("res/models/monkey.glb.bin", cmdBuffer);
	LoadModel(&game->model, "res/models/Fox.glb.bin", cmdBuffer);
	InitAnimationState(&game->modelAnim, &game->model);

	LoadModel(&game->cube, "res/models/cube.glb.bin", cmdBuffer);

	InitRigidBody(&game->platform, RIGID_BODY_STATIC, vec3::Zero, Quaternion::Identity);
	AddBoxCollider(&game->platform, vec3(20, 2, 20), vec3(0, -1, 0), Quaternion::Identity, 1, 1, false);

	InitCharacterController(&game->controller, 0.2f, 2, 0.2f, vec3(0, 3, 3));

	LoadSound(&game->testSound, "res/sounds/test.ogg.bin");

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
		MoveCharacterController(&game->controller, displacement, 1);
		//game->cameraPosition += displacement;
	}

	game->cameraPosition = GetCharacterControllerPosition(&game->controller) + vec3(0, 1.5f, 0);

	if (app->keys[SDL_SCANCODE_ESCAPE] && !app->lastKeys[SDL_SCANCODE_ESCAPE])
		game->mouseLocked = !game->mouseLocked;

	SDL_SetWindowRelativeMouseMode(window, game->mouseLocked);

	if (game->mouseLocked)
	{
		game->cameraYaw -= app->mouseDelta.x * 0.001f;
		game->cameraPitch -= app->mouseDelta.y * 0.001f;
	}

	AnimateModel(&game->model, &game->modelAnim, &game->model.animations[2], gameTime);

	if (EveryInterval(1))
	{
		RigidBody* body = &game->cubeBodies[game->numCubeBodies++];
		InitRigidBody(body, RIGID_BODY_DYNAMIC, vec3(0, 10, 0), Quaternion::FromEulers(vec3(342, 5, 1)));
		AddBoxCollider(body, vec3(1), vec3::Zero, Quaternion::Identity, 1, 1, false);
	}

	if (app->keys[SDL_SCANCODE_P] && !app->lastKeys[SDL_SCANCODE_P])
		PlaySound(&game->testSound);
}

void GameRender()
{
	RenderModel(&game->renderer, &game->model, &game->modelAnim, mat4::Scale(0.01f));

	mat4 platformTransform = mat4::Transform(vec3(0, -1, 0), Quaternion::Identity, vec3(20, 2, 20));
	RenderModel(&game->renderer, &game->cube, nullptr, platformTransform);

	for (int i = 0; i < game->numCubeBodies; i++)
	{
		vec3 cubePosition;
		Quaternion cubeRotation;
		GetRigidBodyTransform(&game->cubeBodies[i], &cubePosition, &cubeRotation);
		mat4 cubeTransform = mat4::Transform(cubePosition, cubeRotation, vec3::One);
		RenderModel(&game->renderer, &game->cube, nullptr, cubeTransform);
	}

	RenderLight(&game->renderer, Quaternion::FromAxisAngle(vec3::Up, gameTime * PI) * vec3(2, 2, 0), vec3(1, 0.5f, 1));
	RenderLight(&game->renderer, Quaternion::FromAxisAngle(vec3::Right, gameTime * PI * 0.7f) * vec3(2, 2, 0), vec3(0.5f, 1, 0.5f));

	DebugText(0, height / 16 - 1, "%.2f, %.2f, %.2f", game->cameraPosition.x, game->cameraPosition.y, game->cameraPosition.z);
}
