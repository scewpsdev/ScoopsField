#pragma once

#include <SDL3/SDL.h>

#include "utils/HashMap.h"
#include "utils/BumpAllocator.h"


struct AppState;

struct GameMemory
{
	uint64_t constantMemorySize;
	uint8_t* constantMemory;

	uint64_t transientMemorySize;
	uint8_t* transientMemory;

	SDL_malloc_func defaultMalloc;
	SDL_calloc_func defaultCalloc;
	SDL_realloc_func defaultRealloc;
	SDL_free_func defaultFree;

	BumpAllocator constantAllocator;
	BumpAllocator transientAllocator;

	HashMap<void*, uint64_t, 4000> platformAllocations;
	uint64_t platformMemoryUsage;
	int platformAllocationCount;
	int platformAllocationsPerFrame;

	HashMap<void*, uint64_t, 4000> physicsAllocations;
	uint64_t physicsMemoryUsage;
	int physicsAllocationCount;
	int physicsAllocationsPerFrame;

	AppState* appState;
};
