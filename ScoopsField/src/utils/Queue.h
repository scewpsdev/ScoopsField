#pragma once

#include <SDL3/SDL.h>


template<typename T, int CAPACITY>
struct Queue
{
	T data[CAPACITY];
	int capacity;
	int size;
	int head;
	int tail;
};


template<typename T, int CAPACITY>
void InitQueue(Queue<T, CAPACITY>& queue)
{
	SDL_memset(queue.data, 0, sizeof(queue.data));
	queue.capacity = CAPACITY;
	queue.size = 0;
	queue.head = 0;
	queue.tail = 0;
}

template<typename T, int CAPACITY>
T* QueuePush(Queue<T, CAPACITY>& queue, const T& value)
{
	if (queue.size == queue.capacity)
		return nullptr;

	T* slot = &queue.data[queue.tail];
	*slot = value;
	queue.tail = (queue.tail + 1) % queue.capacity;
	queue.size++;
	return slot;
}

template<typename T, int CAPACITY>
T* QueuePop(Queue<T, CAPACITY>& queue)
{
	if (queue.size == 0)
		return nullptr;

	T* value = &queue.data[queue.head];
	queue.head = (queue.head + 1) % queue.capacity;
	queue.size--;
	return value;
}

template<typename T, int CAPACITY>
T* QueuePeek(Queue<T, CAPACITY>& queue)
{
	if (queue.size == 0)
		return nullptr;

	return &queue.data[queue.head];
}

template<typename T, int CAPACITY>
T* QueuePeekAt(Queue<T, CAPACITY>& queue, int offset)
{
	if (queue.size == 0 || offset >= queue.size)
		return nullptr;

	return &queue.data[(queue.head + offset) % CAPACITY];
}
