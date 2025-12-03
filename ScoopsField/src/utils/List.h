#pragma once

#include <SDL3/SDL.h>


template<typename T, int CAPACITY>
struct List
{
	T buffer[CAPACITY];
	int capacity;
	int size;


	T& operator[](int idx)
	{
		SDL_assert(idx < size);
		return buffer[idx];
	}

	const T& operator[](int idx) const
	{
		SDL_assert(idx < size);
		return buffer[idx];
	}

	void resize(int newSize)
	{
		SDL_assert(newSize <= CAPACITY);
		size = newSize;
	}

	void add(const T& t)
	{
		SDL_assert(size < CAPACITY);
		buffer[size++] = t;
	}

	void removeAt(int idx)
	{
		SDL_assert(idx < size);
		for (int i = idx; i < size - 1; i++)
			buffer[i] = buffer[i + 1];
		size--;
	}

	void insert(int idx, const T& element)
	{
		SDL_assert(size < CAPACITY);
		SDL_assert(idx < size);
		resize(size + 1);
		for (int i = size - 1; i >= idx + 1; i--)
			buffer[i] = buffer[i - 1];
		buffer[idx] = element;
	}

	int indexOf(const T& t)
	{
		for (int i = 0; i < size; i++)
		{
			if (buffer[i] == t)
				return i;
		}
		return -1;
	}

	bool contains(const T& t)
	{
		return indexOf(t) != -1;
	}

	void remove(const T& t)
	{
		int idx = indexOf(t);
		if (idx != -1)
			removeAt(idx);
	}

	void clear()
	{
		size = 0;
	}
};


template<typename T, int CAPACITY>
void InitList(List<T, CAPACITY>* list)
{
	list->capacity = CAPACITY;
	list->size = 0;
}
