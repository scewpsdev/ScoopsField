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


	void clear()
	{
		size = 0;
		SDL_memset(occupied, 0, sizeof(occupied));

		for (int i = 0; i < CAPACITY - 1; i++)
			freeList[i] = i + 1;

		freeList[CAPACITY - 1] = -1;
		freeHead = 0;
	}

	T* add()
	{
		if (freeHead == -1)
			return nullptr;

		int idx = freeHead;
		freeHead = freeList[idx];
		size++;
		occupied[idx] = true;
		T* element = &data[idx];
		SDL_memset(element, 0, sizeof(T));
		return element;
	}

	void add(T value)
	{
		T* element = add();
		SDL_assert(element);
		*element = value;
	}

	void remove(T* element)
	{
		int idx = (int)(element - &data[0]);
		occupied[idx] = false;
		freeList[idx] = freeHead;
		freeHead = idx;
		size--;
	}
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
	T* data = &pool->data[idx];
	SDL_memset(data, 0, sizeof(T));
	return data;
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
