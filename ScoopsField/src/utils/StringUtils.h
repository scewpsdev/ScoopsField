#pragma once

#include <SDL3/SDL.h>


inline void MemoryString(char* str, size_t maxLen, uint64_t mem)
{
	if (mem >= 1 << 30)
	{
		SDL_snprintf(str, maxLen, "%.2f GB", mem / (float)(1 << 30));
	}
	else if (mem >= 1 << 20)
	{
		SDL_snprintf(str, maxLen, "%.2f MB", mem / (float)(1 << 20));
	}
	else if (mem >= 1 << 10)
	{
		SDL_snprintf(str, maxLen, "%.2f KB", mem / (float)(1 << 10));
	}
	else
	{
		SDL_snprintf(str, maxLen, "%d B", mem);
	}
}
