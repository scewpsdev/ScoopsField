#pragma once

#include <stdint.h>
#include <SDL3/SDL.h>


struct BumpAllocator
{
	uint8_t* buffer;
	uint64_t capacity;
	uint64_t offset;
};


inline void InitBumpAllocator(BumpAllocator* allocator, uint8_t* buffer, uint64_t capacity)
{
	allocator->buffer = buffer;
	allocator->capacity = capacity;
}

inline void ResetBumpAllocator(BumpAllocator* allocator)
{
//#if _DEBUG
//	SDL_memset4(allocator->buffer, 0, (allocator->offset + 3) / 4);
//#endif
	allocator->offset = 0;
}

inline uint8_t* BumpAllocatorMalloc(BumpAllocator* allocator, size_t size)
{
	if (allocator->offset + size <= allocator->capacity)
	{
		uint8_t* ptr = &allocator->buffer[allocator->offset];
		allocator->offset += (size * 4 + 3) / 4;
		return ptr;
	}
	return nullptr;
}

inline uint8_t* BumpAllocatorCalloc(BumpAllocator* allocator, size_t num, size_t size)
{
	if (allocator->offset + num * size <= allocator->capacity)
	{
		uint8_t* ptr = &allocator->buffer[allocator->offset];
		SDL_memset(ptr, 0, num * size);
		allocator->offset += (num * size * 4 + 3) / 4;
		return ptr;
	}
	return nullptr;
}

inline uint8_t* BumpAllocatorRealloc(BumpAllocator* allocator, uint8_t* ptr, size_t size, size_t newSize)
{
	SDL_assert(ptr && newSize);

	if (newSize > size && ptr == &allocator->buffer[allocator->offset] - size)
	{
		allocator->offset += newSize - size;
		return ptr;
	}

	uint8_t* newBuffer = BumpAllocatorMalloc(allocator, newSize);
	SDL_memcpy(newBuffer, ptr, size);
	return newBuffer;
}
