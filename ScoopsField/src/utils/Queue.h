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
void InitQueue(Queue<T, CAPACITY>* queue)
{
	SDL_memset(queue->data, 0, sizeof(queue->data));
	queue->capacity = CAPACITY;
	queue->size = 0;
	queue->head = 0;
	queue->tail = 0;
}

template<typename T, int CAPACITY>
bool QueuePush(Queue<T, CAPACITY>* queue, const T& value)
{
	if (queue->size == queue->capacity)
		return false;

	queue->data[queue->tail] = value;
	queue->tail = (queue->tail + 1) % queue->capacity;
	queue->size++;
	return true;
}

template<typename T, int CAPACITY>
bool QueuePop(Queue<T, CAPACITY>* queue, T* value)
{
	if (queue->size == 0)
		return false;

	*value = queue->data[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	queue->size--;
	return true;
}
