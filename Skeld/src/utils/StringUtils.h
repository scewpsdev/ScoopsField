#pragma once

#include <SDL3/SDL.h>


inline bool StartsWith(const char* str, const char* prefix)
{
	int len = (int)SDL_strlen(str);
	int slen = (int)SDL_strlen(prefix);
	if (slen > len)
		return false;
	for (int i = 0; i < slen; i++)
	{
		if (str[i] != prefix[i])
			return false;
	}
	return true;
}

inline bool EndsWith(const char* str, const char* suffix)
{
	int len = (int)SDL_strlen(str);
	int slen = (int)SDL_strlen(suffix);
	if (slen > len)
		return false;
	for (int i = 0; i < slen; i++)
	{
		if (str[len - 1 - i] != suffix[slen - 1 - i])
			return false;
	}
	return true;
}

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

template<typename T>
inline void ReverseArray(T* buffer, uint64_t size, int length)
{
	for (int i = 0; i < length / 2; i++)
	{
		T tmp = buffer[i];
		buffer[i] = buffer[length - 1 - i];
		buffer[length - 1 - i] = tmp;
	}
}
