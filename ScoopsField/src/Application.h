#pragma once

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

	uint64_t transientMemoryUsage;
	int transientMemoryCount;

	uint64_t platformMemoryUsage;
	int platformAllocationCount;
	int platformAllocationCounter;
	int platformAllocationsPerFrame;

	uint64_t physicsMemoryUsage;
	int physicsAllocationCount;
	int physicsAllocationCounter;
	int physicsAllocationsPerFrame;

	uint64_t meshMemoryUsage;
	int meshAllocationCount;

	uint64_t particleMemoryUsage;
	int particleAllocationCount;

	SDL_Window* window;
	SDL_GPUDevice* device;

	bool acquireFence;
	SDL_GPUFence** fenceTarget;

	GpuTimerContext gpuTiming;

	int width, height;
	int debugStats;
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

	float deltaTime;

	int numKeys;
	const bool* keys;
	bool* lastKeys;

	vec2 mousePosition;
	vec2 lastMousePosition;
	vec2 mouseDelta;
	ivec2 mouseWheel;
	ivec2 lastMouseWheel;
	ivec2 mouseWheelDelta;
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


extern AppState* app;
extern SDL_GPUDevice* device;


void* PhysicsMalloc(size_t size);
void PhysicsFree(void* mem);
void* MeshMalloc(size_t size);
void MeshFree(void* mem);
void* ParticleMalloc(size_t size);
void ParticleFree(void* mem);

bool EveryInterval(float seconds, uint32_t h);
bool GetKey(SDL_Scancode key);
bool GetKeyDown(SDL_Scancode key);
bool GetKeyUp(SDL_Scancode key);
bool GetMouseButton(uint32_t button);
bool GetMouseButtonDown(uint32_t button);
bool GetMouseButtonUp(uint32_t button);
int GetMouseScroll();

void DebugTextEx(int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor);
void DebugText(int x, int y, uint32_t color, uint32_t bgcolor, const char* fmt, ...);
void DebugText(int x, int y, const char* fmt, ...);
