#pragma once

#include <SDL3/SDL.h>

#include <stdlib.h>

#include "Core.h"

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
