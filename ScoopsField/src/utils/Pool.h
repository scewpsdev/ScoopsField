#pragma once

#include <SDL3/SDL.h>


template<typename T, int CAPACITY>
struct Pool
{
	T data[CAPACITY];
	bool occupied[CAPACITY];
	int capacity;
	int size;

	int freeList[CAPACITY];
	int freeHead;
};


template<typename T, int CAPACITY>
void InitPool(Pool<T, CAPACITY>* pool)
{
	pool->capacity = CAPACITY;
	pool->size = 0;
	SDL_memset(pool->occupied, 0, sizeof(pool->occupied));

	for (int i = 0; i < CAPACITY - 1; i++)
		pool->freeList[i] = i + 1;

	pool->freeList[CAPACITY - 1] = -1;
	pool->freeHead = 0;
}

template<typename T, int CAPACITY>
void ClearPool(Pool<T, CAPACITY>* pool)
{
	pool->size = 0;
	SDL_memset(pool->occupied, 0, sizeof(pool->occupied));

	for (int i = 0; i < CAPACITY - 1; i++)
		pool->freeList[i] = i + 1;

	pool->freeList[CAPACITY - 1] = -1;
	pool->freeHead = 0;
}

template<typename T, int CAPACITY>
T* PoolAlloc(Pool<T, CAPACITY>* pool)
{
	if (pool->freeHead == -1)
		return nullptr;

	int idx = pool->freeHead;
	pool->freeHead = pool->freeList[idx];
	pool->size++;
	pool->occupied[idx] = true;
	return &pool->data[idx];
}

template<typename T, int CAPACITY>
void PoolRelease(Pool<T, CAPACITY>* pool, T* element)
{
	int idx = (int)(element - &pool->data[0]);
	pool->occupied[idx] = false;
	pool->freeList[idx] = pool->freeHead;
	pool->freeHead = idx;
	pool->size--;
}
