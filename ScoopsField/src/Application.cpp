#include "Application.h"

#include "utils/StringUtils.h"
#include "math/Math.h"


GameMemory* memory;
AppState* app;
GraphicsState* graphics;
ResourceState* resource;
GameState* game;

SDL_Window* window;
SDL_GPUDevice* device;

int width, height;
float deltaTime;
int fps;
float avgMs;

SDL_GPUTexture* swapchain = nullptr;


void DebugTextEx(int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor)
{
	DebugTextRendererSubmit(&app->debugTextRenderer, x, y, txt, len, color, bgcolor);
}

void DebugText(int x, int y, uint32_t color, uint32_t bgcolor, const char* fmt, ...)
{
	static char buffer[256];

	va_list args;
	va_start(args, fmt);
	int len = SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	DebugTextEx(x, y, buffer, len, color, bgcolor);
}

void DebugText(int x, int y, const char* fmt, ...)
{
	static char buffer[256];

	va_list args;
	va_start(args, fmt);
	int len = SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	DebugTextEx(x, y, buffer, len, 0xFFFFFFFF, 0);
}


#include "game/Game.cpp"


extern "C" __declspec(dllexport) SDL_AppResult AppInit(GameMemory* memory, AppState* appState, int argc, char** argv)
{
	::memory = memory;
	app = appState;
	graphics = &app->graphics;
	resource = &app->resourceState;
	game = &app->game;
	window = app->window;
	device = app->device;

	SDL_GetWindowSizeInPixels(window, &width, &height);

	if (app->platformCallbacks.compileResources)
		app->platformCallbacks.compileResources();

	app->lastFrame = SDL_GetTicksNS();
	app->lastSecond = SDL_GetTicksNS();

	SDL_GetKeyboardState(&app->numKeys);
	app->lastKeys = (bool*)BumpAllocatorCalloc(&memory->constantAllocator, app->numKeys, sizeof(bool));

	SDL_GetMouseState(&app->lastMousePosition.x, &app->lastMousePosition.y);

	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(device);

	InitDebugTextRenderer(&app->debugTextRenderer, 1000, cmdBuffer);

	GameInit(cmdBuffer);

	SDL_SubmitGPUCommandBuffer(cmdBuffer);

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
	app = appState;
	graphics = &app->graphics;
	resource = &app->resourceState;
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

static void RenderDebugStats()
{
	uint64_t memoryUsage = memory->constantAllocator.offset;
	uint64_t transientMemoryUsage = memory->transientAllocator.offset;

	char memoryUsageStr[16];
	MemoryString(memoryUsageStr, 16, memoryUsage);

	char transientMemoryUsageStr[16];
	MemoryString(transientMemoryUsageStr, 16, transientMemoryUsage);

	char platformMemoryUsageStr[16];
	MemoryString(platformMemoryUsageStr, 16, memory->platformMemoryUsage);

	DebugText(0, 0, COLOR_WHITE, COLOR_BLACK, "%d fps, %.3f ms", fps, avgMs);
	DebugText(0, 1, COLOR_WHITE, COLOR_BLACK, "platform %s, %d allocations", platformMemoryUsageStr, memory->platformAllocationCount);
	DebugText(0, 2, COLOR_WHITE, COLOR_BLACK, "constant %s, %d allocations", memoryUsageStr, memory->constantAllocator.count);
	DebugText(0, 3, COLOR_WHITE, COLOR_BLACK, "transient %s, %d allocations", transientMemoryUsageStr, memory->transientAllocator.count);
}

extern "C" __declspec(dllexport) void AppIterate(GameMemory* memory, AppState* appState)
{
	::memory = memory;
	app = appState;
	graphics = &app->graphics;
	resource = &app->resourceState;
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
	}

	app->keys = SDL_GetKeyboardState(&app->numKeys);
	app->mouseButtons = SDL_GetMouseState(&app->mousePosition.x, &app->mousePosition.y);
	SDL_GetRelativeMouseState(&app->mouseDelta.x, &app->mouseDelta.y);

	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(device);

	Uint32 swapchainWidth, swapchainHeight;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuffer, app->window, &swapchain, &swapchainWidth, &swapchainHeight);

	DebugTextRendererBegin(&app->debugTextRenderer);
	RenderDebugStats();

	GameUpdate();
	GameRender(cmdBuffer);

	DebugTextRendererEnd(&app->debugTextRenderer, width, height, cmdBuffer);

	SDL_SubmitGPUCommandBuffer(cmdBuffer);
	swapchain = nullptr;

	app->frameCounter++;

	app->lastFrame = app->now;

	SDL_memcpy(app->lastKeys, app->keys, app->numKeys * sizeof(bool));
	app->lastMousePosition = app->mousePosition;
	app->lastMouseButtons = app->mouseButtons;
}
