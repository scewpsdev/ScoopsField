#include "Application.h"

#include "utils/StringUtils.h"
#include "math/Math.h"


GameMemory* memory;
AppState* app;
GraphicsState* graphics;
GameState* game;

SDL_Window* window;
SDL_GPUDevice* device;

int width, height;
float deltaTime;
int fps;
float avgMs;
uint64_t memoryUsage;
uint64_t transientMemoryUsage;

SDL_GPUTexture* swapchain = nullptr;


#include "game/Game.cpp"


extern "C" __declspec(dllexport) SDL_AppResult AppInit(GameMemory* memory, AppState* appState, int argc, char** argv)
{
	::memory = memory;
	app = (AppState*)memory->constantMemory;
	graphics = &app->graphics;
	game = &app->game;
	window = app->window;
	device = app->device;

	SDL_GetWindowSizeInPixels(window, &width, &height);

	if (appState->platformCallbacks.compileResources)
		appState->platformCallbacks.compileResources();

	app->lastFrame = SDL_GetTicksNS();
	app->lastSecond = SDL_GetTicksNS();

	SDL_GetKeyboardState(&app->numKeys);
	app->lastKeys = (bool*)BumpAllocatorCalloc(&memory->constantAllocator, app->numKeys, sizeof(bool));

	SDL_GetMouseState(&app->lastMousePosition.x, &app->lastMousePosition.y);

	GameInit();

	return SDL_APP_CONTINUE;
}

void AppResize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;

	GameResize(newWidth, newHeight);
}

extern "C" __declspec(dllexport) void AppDestroy(GameMemory* memory, AppState* appState, SDL_AppResult result)
{
	SDL_Log("Shutting down...");

	::memory = memory;
	app = (AppState*)memory->constantMemory;
	graphics = &app->graphics;
	game = &app->game;
	window = app->window;
	device = app->device;

	GameDestroy();
}

extern "C" __declspec(dllexport) SDL_AppResult AppOnEvent(GameMemory* memory, AppState* appState, SDL_Event* event)
{
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		return SDL_APP_SUCCESS;


	if (event->type == SDL_EVENT_WINDOW_RESIZED)
		AppResize(event->window.data1, event->window.data2);

	return SDL_APP_CONTINUE;
}

extern "C" __declspec(dllexport) void AppIterate(GameMemory* memory, AppState* appState)
{
	::memory = memory;
	app = (AppState*)memory->constantMemory;
	graphics = &app->graphics;
	game = &app->game;
	window = app->window;
	device = app->device;

	app->now = SDL_GetTicksNS();
	deltaTime = (app->now - app->lastFrame) / 1e9f;

	if (app->now - app->lastSecond >= 1e9)
	{
		fps = app->frameCounter;

		avgMs = (app->now - app->lastSecond) / 1e6f / app->frameCounter;

		app->frameCounter = 0;
		app->lastSecond = app->now;

		char memoryUsageStr[16];
		MemoryString(memoryUsageStr, 16, memoryUsage);

		char transientMemoryUsageStr[16];
		MemoryString(transientMemoryUsageStr, 16, transientMemoryUsage);

		char systemMemoryUsageStr[16];
		MemoryString(systemMemoryUsageStr, 16, (uint64_t)SDL_GetSystemRAM() * 1024 * 1024);

		SDL_Log("%d fps, %.3f ms | %s, %s, %s", fps, avgMs, memoryUsageStr, transientMemoryUsageStr, systemMemoryUsageStr);

		memoryUsage = 0;
		transientMemoryUsage = 0;
	}

	app->keys = SDL_GetKeyboardState(&app->numKeys);
	app->mouseButtons = SDL_GetMouseState(&app->mousePosition.x, &app->mousePosition.y);
	SDL_GetRelativeMouseState(&app->mouseDelta.x, &app->mouseDelta.y);

	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(device);

	Uint32 swapchainWidth, swapchainHeight;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuffer, app->window, &swapchain, &swapchainWidth, &swapchainHeight);

	GameUpdate();
	GameRender(cmdBuffer);

	SDL_SubmitGPUCommandBuffer(cmdBuffer);
	swapchain = nullptr;

	app->frameCounter++;

	memoryUsage = max(memoryUsage, memory->constantAllocator.offset);
	transientMemoryUsage = max(transientMemoryUsage, memory->transientAllocator.offset);

	app->lastFrame = app->now;

	SDL_memcpy(app->lastKeys, app->keys, app->numKeys * sizeof(bool));
	app->lastMousePosition = app->mousePosition;
	app->lastMouseButtons = app->mouseButtons;
}
