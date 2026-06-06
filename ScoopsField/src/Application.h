#pragma once

#if _DEBUG
#define GPU_TIMING
#endif

#include <SDL3/SDL.h>

#include <stdlib.h>

#include "GameMemory.h"

#include "Resource.h"


#include "game/Game.h"

#include "graphics/Graphics.h"
#include "graphics/GPUTiming.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "math/Math.h"
#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"

#include "utils/BumpAllocator.h"
#include "utils/Queue.h"
#include "utils/Pool.h"
#include "utils/HashMap.h"


#define PROJECT_PATH "D:\\Dev\\ScoopsField\\ScoopsField"


struct PlatformCallbacks
{
	void (*compileResources)();
};

struct AppState
{
	PlatformCallbacks platformCallbacks;

	SDL_Window* window;
	SDL_GPUDevice* device;

	bool acquireFence;
	SDL_GPUFence** fenceTarget;

	GpuTimerContext gpuTiming;

	int width, height;
	bool debugStats;
	int frameIdx;
	int lastSecondFrame;

	uint64_t now;
	uint64_t lastFrame;
	uint64_t lastSecond;
	uint64_t frameTime;
	uint64_t frameTimeVariance;
	uint64_t updateTime;
	uint64_t cpuFrame;
	uint64_t swapchainWait;
	uint64_t gpuSubmit;
	int fps;
	float avgMs;
	float avgMsVariance;
	float updateTimeMs;
	float cpuFrameMs;
	float swapchainWaitMs;
	float gpuSubmitMs;

	int platformAllocationsPerFrame;
	int physicsAllocationsPerFrame;

	float deltaTime;
	float gameTime;

	int numKeys;
	const bool* keys;
	bool* lastKeys;

	vec2 mousePosition;
	vec2 lastMousePosition;
	vec2 mouseDelta;
	SDL_MouseButtonFlags mouseButtons;
	SDL_MouseButtonFlags lastMouseButtons;

	SoLoud::Soloud* soloud;

	GraphicsState graphics;
	AudioState audio;
	PhysicsState physics;
	ResourceState resourceState;
	GameState game;

	DebugTextRenderer debugTextRenderer;
};



bool EveryInterval(float seconds, uint32_t h);
bool GetKey(SDL_Scancode key);
bool GetKeyDown(SDL_Scancode key);
bool GetKeyUp(SDL_Scancode key);
bool GetMouseButton(uint32_t button);
bool GetMouseButtonDown(uint32_t button);
bool GetMouseButtonUp(uint32_t button);

void DebugTextEx(int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor);
void DebugText(int x, int y, uint32_t color, uint32_t bgcolor, const char* fmt, ...);
void DebugText(int x, int y, const char* fmt, ...);
