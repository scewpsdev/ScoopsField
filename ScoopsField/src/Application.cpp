#include "Application.h"

#include "utils/StringUtils.h"
#include "math/Math.h"


GameMemory* memory;
AppState* app;
GraphicsState* graphics;
AudioState* audio;
PhysicsState* physics;
ResourceState* resource;
GameState* game;

SDL_Window* window;
SDL_GPUDevice* device;

// these get either set at the beginning of the frame or on resize
float deltaTime;
float gameTime;

// these get set during the frame and can be null on reload
SDL_GPUTexture* swapchain = nullptr;
SDL_GPUCommandBuffer* cmdBuffer = nullptr;


bool GetKey(SDL_Scancode key)
{
	return app->keys[key];
}

bool GetKeyDown(SDL_Scancode key)
{
	return app->keys[key] && !app->lastKeys[key];
}

bool GetKeyUp(SDL_Scancode key)
{
	return !app->keys[key] && app->lastKeys[key];
}

bool GetMouseButton(uint32_t button)
{
	return app->mouseButtons & SDL_BUTTON_MASK(button);
}

bool GetMouseButtonDown(uint32_t button)
{
	return (app->mouseButtons & SDL_BUTTON_MASK(button)) && !(app->lastMouseButtons & SDL_BUTTON_MASK(button));
}

bool GetMouseButtonUp(uint32_t button)
{
	return !(app->mouseButtons & SDL_BUTTON_MASK(button)) && (app->lastMouseButtons & SDL_BUTTON_MASK(button));
}

void DebugTextEx(int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor)
{
#ifdef _DEBUG
	DebugTextRendererSubmit(&app->debugTextRenderer, x, y, txt, len, color, bgcolor);
#endif
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

void GUIPanel(int x, int y, int w, int h, vec4 color)
{
	SpriteDrawData drawData = {};
	drawData.position = vec3((float)(x + w / 2), (float)(app->height - y - h / 2), 0);
	drawData.size = vec2((float)w, (float)h);
	drawData.rotation = 0;
	drawData.color = color;
	DrawSprite(&game->guiRenderer, 0, &drawData);
}

void GUIPanel(int x, int y, int w, int h, Texture* texture, vec4 color)
{
	SpriteDrawData drawData = {};
	drawData.position = vec3((float)(x + w / 2), (float)(app->height - y - h / 2), 0);
	drawData.size = vec2((float)w, (float)h);
	drawData.rotation = 0;
	drawData.color = color;
	drawData.texture = texture;
	drawData.rect = vec4(0, 0, 1, 1);
	DrawSprite(&game->guiRenderer, 0, &drawData);
}

void GUIPanel(int x, int y, Texture* texture, const ivec4& textureRect, vec4 color)
{
	int w = textureRect.z;
	int h = textureRect.w;
	int u = textureRect.x;
	int v = textureRect.y;

	SpriteDrawData drawData = {};
	drawData.position = vec3((float)(x + w / 2), (float)(app->height - y - h / 2), 0);
	drawData.size = vec2((float)w, (float)h);
	drawData.rotation = 0;
	drawData.color = color;
	drawData.texture = texture;
	drawData.rect = vec4(
		u / (float)texture->info.width,
		v / (float)texture->info.height,
		w / (float)texture->info.width,
		h / (float)texture->info.height
	);
	DrawSprite(&game->guiRenderer, 0, &drawData);
}

void GUIPanel(int x, int y, Texture* texture)
{
	GUIPanel(x, y, texture, ivec4(0, 0, texture->info.width, texture->info.height), vec4(1));
}


#include "graphics/GPUTiming.cpp"
#include "renderer/Renderer.cpp"
#include "game/Game.cpp"


static void CompileResources()
{
#ifdef _DEBUG
	int result = system("D:\\Dev\\Rainfall\\RainfallResourceCompiler\\bin\\x64\\Release\\RainfallResourceCompiler.exe " PROJECT_PATH "\\res res png hdr ogg vsh fsh csh glsl vert frag comp ttf rfs gltf glb");
	//SDL_assert(result == 0);
#endif
}

static void InitPlatformCallbacks(PlatformCallbacks* callbacks)
{
	callbacks->compileResources = CompileResources;
}

void* SDLmalloc(size_t size)
{
	memory->platformMemoryUsage += size;
	memory->platformAllocationCount++;
	memory->platformAllocationsPerFrame++;
	void* mem = memory->defaultMalloc(size);
	if (HashMapHasSlot(&memory->platformAllocations))
		HashMapAdd(&memory->platformAllocations, mem, size);
	return mem;
}

void* SDLcalloc(size_t nmemb, size_t size)
{
	memory->platformMemoryUsage += nmemb * size;
	memory->platformAllocationCount++;
	void* mem = memory->defaultCalloc(nmemb, size);
	if (HashMapHasSlot(&memory->platformAllocations))
		HashMapAdd(&memory->platformAllocations, mem, nmemb * size);
	return mem;
}

void* SDLrealloc(void* mem, size_t size)
{
	if (!mem)
		return SDL_malloc(size);
	else if (!size)
	{
		SDL_free(mem);
		return nullptr;
	}

	void* newMem = memory->defaultRealloc(mem, size);
	if (newMem == mem)
	{
		if (uint64_t* memsize = HashMapGet(&memory->platformAllocations, mem))
		{
			memory->platformMemoryUsage += size - *memsize;
			*memsize = size;
		}
	}
	else
	{
		if (uint64_t* memsize = HashMapRemove(&memory->platformAllocations, mem))
		{
			memory->platformMemoryUsage += size - *memsize;
			HashMapAdd(&memory->platformAllocations, newMem, size);
		}
	}
	return newMem;
}

void SDLfree(void* mem)
{
	memory->defaultFree(mem);
	if (uint64_t* memsize = HashMapRemove(&memory->platformAllocations, mem))
	{
		memory->platformMemoryUsage -= *memsize;
		memory->platformAllocationCount--;
	}
}

static AppState* InitAppState()
{
	AppState* appState = (AppState*)(BumpAllocatorMalloc(&memory->constantAllocator, sizeof(AppState)));
	memory->appState = appState;
	InitPlatformCallbacks(&appState->platformCallbacks);

#if _DEBUG
	SDL_GetOriginalMemoryFunctions(&memory->defaultMalloc, &memory->defaultCalloc, &memory->defaultRealloc, &memory->defaultFree);
	SDL_SetMemoryFunctions(SDLmalloc, SDLcalloc, SDLrealloc, SDLfree);
#endif

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
		return nullptr;
	}

	SDL_Log("SDL %s", SDL_GetRevision());

	const char* title = "ScoopsField";
	int width = 1280;
	int height = 720;
	SDL_Window* window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN /*| SDL_WINDOW_MAXIMIZED*/);
	if (!window)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s", SDL_GetError());
		return nullptr;
	}
	SDL_Log("Display %dx%d", width, height);

	bool gpuDebug = false;
#if _DEBUG
	gpuDebug = true;
#endif

	SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, gpuDebug, nullptr);
	if (!device)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_GPU, "Failed to create graphics device: %s", SDL_GetError());
		return nullptr;
	}
	if (!SDL_ClaimWindowForGPUDevice(device, window))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_GPU, "Failed to create swapchain: %s", SDL_GetError());
		return nullptr;
	}

	SDL_Log("Video driver %s-%s", SDL_GetCurrentVideoDriver(), SDL_GetGPUDeviceDriver(device));

	SDL_GPUSwapchainComposition swapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
	SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
	if (!SDL_SetGPUSwapchainParameters(device, window, swapchainComposition, presentMode))
	{
		SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to set swapchain parameters: %s", SDL_GetError());
	}

	appState->window = window;
	appState->device = device;

	InitGPUTimer(&appState->gpuTiming, device);

#if _DEBUG
	appState->debugStats = true;
#endif

	return appState;
}

extern "C" __declspec(dllexport) SDL_AppResult AppInit(GameMemory* memory, int argc, char** argv)
{
	::memory = memory;

	InitBumpAllocator(&memory->constantAllocator, memory->constantMemory, memory->constantMemorySize);
	InitBumpAllocator(&memory->transientAllocator, memory->transientMemory, memory->transientMemorySize);

	InitHashMap(&memory->platformAllocations);
	InitHashMap(&memory->physicsAllocations);

	memory->appState = InitAppState();
	if (!memory->appState)
		return SDL_APP_FAILURE;

	app = memory->appState;
	graphics = &app->graphics;
	audio = &app->audio;
	physics = &app->physics;
	resource = &app->resourceState;
	game = &app->game;
	window = app->window;
	device = app->device;

	SDL_GetWindowSizeInPixels(window, &app->width, &app->height);

	if (app->platformCallbacks.compileResources)
		app->platformCallbacks.compileResources();

	app->lastFrame = SDL_GetTicksNS();
	app->lastSecond = SDL_GetTicksNS();

	SDL_GetKeyboardState(&app->numKeys);
	app->lastKeys = (bool*)BumpAllocatorCalloc(&memory->constantAllocator, app->numKeys, sizeof(bool));

	SDL_GetMouseState(&app->lastMousePosition.x, &app->lastMousePosition.y);

	app->soloud = new(BumpAllocatorMalloc(&memory->constantAllocator, sizeof(SoLoud::Soloud)))SoLoud::Soloud();
	if (SoLoud::result result = app->soloud->init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to initialize audio backend: %s", app->soloud->getErrorString(result));
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "%s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	InitAudio(&app->audio, app->soloud);

	SDL_Log("Audio backend: %s", app->soloud->getBackendString());

	InitPhysics(&app->physics);

	InitResourceState(&app->resourceState);

	cmdBuffer = SDL_AcquireGPUCommandBuffer(device);

	InitDebugTextRenderer(&app->debugTextRenderer, 1000, cmdBuffer);

	GameInit(cmdBuffer);

	SDL_SubmitGPUCommandBuffer(cmdBuffer); cmdBuffer = nullptr;

	if (!SDL_ShowWindow(window))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_Log("Initialization complete");


	return SDL_APP_CONTINUE;
}

void AppResize(int newWidth, int newHeight)
{
	app->width = newWidth;
	app->height = newHeight;

	GameResize(newWidth, newHeight);
}

extern "C" __declspec(dllexport) void AppDestroy(GameMemory* memory, AppState* appState, SDL_AppResult result)
{
	SDL_Log("Shutting down");

	SDL_HideWindow(window);

	GameDestroy();

	DestroyPhysics(&app->physics);

	app->soloud->stopAll();
	app->soloud->deinit();

	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

extern "C" __declspec(dllexport) void AppReload(GameMemory* memory)
{
	::memory = memory;
	app = memory->appState;
	graphics = &app->graphics;
	audio = &app->audio;
	physics = &app->physics;
	resource = &app->resourceState;
	game = &app->game;
	window = app->window;
	device = app->device;

	gameTime = app->gameTime;
}

static SDL_AppResult OnEvent(SDL_Event* event)
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

	char physicsMemoryUsageStr[16];
	MemoryString(physicsMemoryUsageStr, 16, memory->physicsMemoryUsage);

	DebugText(0, 0, COLOR_WHITE, COLOR_BLACK, "%d fps, %.3f +- %.3f ms", app->fps, app->avgMs, app->avgMsVariance);
	DebugText(0, 1, COLOR_WHITE, COLOR_BLACK, "platform %s, %d allocations, %d per frame", platformMemoryUsageStr, memory->platformAllocationCount, app->platformAllocationsPerFrame);
	DebugText(0, 2, COLOR_WHITE, COLOR_BLACK, "physics %s, %d allocations, %d per frame", physicsMemoryUsageStr, memory->physicsAllocationCount, app->physicsAllocationsPerFrame);
	DebugText(0, 3, COLOR_WHITE, COLOR_BLACK, "constant %s, %d allocations", memoryUsageStr, memory->constantAllocator.count);
	DebugText(0, 4, COLOR_WHITE, COLOR_BLACK, "transient %s, %d allocations", transientMemoryUsageStr, memory->transientAllocator.count);

	PrintGPUTimers(&app->gpuTiming, 0, 5);
}

extern "C" __declspec(dllexport) SDL_AppResult AppIterate()
{
	app->now = SDL_GetTicksNS();

	uint64_t delta = app->now - app->lastFrame;
	app->frameTime += delta;

	int framesSinceSecond = app->frameIdx - app->lastSecondFrame;
	if (framesSinceSecond > 0)
		app->frameTimeVariance += llabs(delta - (app->frameTime / framesSinceSecond));

	int fpsCap = 0;
	if (fpsCap)
	{
		uint64_t remaining = 1000000000 / fpsCap - (app->now - app->lastFrame);
		SDL_DelayPrecise(remaining);
		deltaTime = 1.0f / fpsCap;
		app->now += remaining;
	}
	else
	{
		deltaTime = (app->now - app->lastFrame) / 1e9f;

		const float maxDelta = 0.05f;
		deltaTime = min(deltaTime, maxDelta);
	}

	app->gameTime += deltaTime;
	gameTime = app->gameTime;

	app->platformAllocationsPerFrame = max(app->platformAllocationsPerFrame, memory->platformAllocationsPerFrame);
	memory->platformAllocationsPerFrame = 0;

	app->physicsAllocationsPerFrame = max(app->physicsAllocationsPerFrame, memory->physicsAllocationsPerFrame);
	memory->physicsAllocationsPerFrame = 0;

	if (app->now - app->lastSecond >= 1e9)
	{
		app->fps = framesSinceSecond;

		app->avgMs = app->frameTime / 1e6f / framesSinceSecond;
		app->avgMsVariance = app->frameTimeVariance / 1e6f / framesSinceSecond;

		app->frameTime = 0;
		app->frameTimeVariance = 0;
		app->lastSecondFrame = app->frameIdx;
		app->lastSecond = app->now;

		app->platformAllocationsPerFrame = 0;
		app->physicsAllocationsPerFrame = 0;
	}

	SDL_Event event = {};
	while (SDL_PollEvent(&event))
	{
		SDL_AppResult result = OnEvent(&event);
		if (result != SDL_APP_CONTINUE)
			return result;
	}

	app->keys = SDL_GetKeyboardState(&app->numKeys);
	app->mouseButtons = SDL_GetMouseState(&app->mousePosition.x, &app->mousePosition.y);
	SDL_GetRelativeMouseState(&app->mouseDelta.x, &app->mouseDelta.y);

	UpdateAudio(&app->audio);

	SDL_assert(!cmdBuffer);
	SDL_assert(!swapchain);

	cmdBuffer = SDL_AcquireGPUCommandBuffer(device);

	BeginGPUTimerFrame(&app->gpuTiming, device, cmdBuffer);

	BeginGPUTimer(cmdBuffer, "frame");

	EndPhysicsFrame(&app->physics);

	GameUpdate();

	DebugTextRendererBegin(&app->debugTextRenderer);

	if (GetKeyDown(SDL_SCANCODE_F10))
		app->debugStats = !app->debugStats;
	if (app->debugStats)
		RenderDebugStats();
	else
		DebugText(0, 0, COLOR_WHITE, COLOR_BLACK, "%d fps", app->fps);

	GameRender();

	StartPhysicsFrame(&app->physics);

	Uint32 swapchainWidth, swapchainHeight;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuffer, app->window, &swapchain, &swapchainWidth, &swapchainHeight);

	GameShowFrame(cmdBuffer);
	DebugTextRendererEnd(&app->debugTextRenderer, app->width, app->height, cmdBuffer);

	EndGPUTimer(cmdBuffer); // frame

	if (app->acquireFence)
	{
		*app->fenceTarget = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdBuffer);
		app->acquireFence = false;
		app->fenceTarget = nullptr;
		cmdBuffer = nullptr;
	}
	else
	{
		SDL_SubmitGPUCommandBuffer(cmdBuffer);
		cmdBuffer = nullptr;
	}
	swapchain = nullptr;

	ResolveGPUTimers(&app->gpuTiming, device);

	ResetBumpAllocator(&memory->transientAllocator);

	app->frameIdx++;

	app->lastFrame = app->now;

	SDL_memcpy(app->lastKeys, app->keys, app->numKeys * sizeof(bool));
	app->lastMousePosition = app->mousePosition;
	app->lastMouseButtons = app->mouseButtons;

	return SDL_APP_CONTINUE;
}
