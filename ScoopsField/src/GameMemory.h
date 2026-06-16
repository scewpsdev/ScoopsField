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

	AppState* appState;
};
