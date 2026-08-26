#pragma once

#include "Hash.h"

#include <SDL3/SDL.h>


enum HashSetSlotState : uint8_t
{
	HASHSET_SLOT_EMPTY = 0,
	HASHSET_SLOT_USED = 1,
	HASHSET_SLOT_TOMBSTONE = 2,
};

template<typename VALUE_TYPE, int CAPACITY>
struct HashSetSlot
{
	VALUE_TYPE value;
	HashSetSlotState state;
};

template<typename VALUE_TYPE, int CAPACITY>
struct HashSet
{
	HashSetSlot<VALUE_TYPE, CAPACITY> slots[CAPACITY];
	int capacity;
	int numUsedSlots;
	int numTombstones;
};


template<typename VALUE_TYPE, int CAPACITY>
inline void InitHashSet(HashSet<VALUE_TYPE, CAPACITY>* set)
{
	set->numUsedSlots = 0;
	set->numTombstones = 0;
	set->capacity = CAPACITY;

	SDL_memset(set->slots, 0, sizeof(set->slots));
}

template<typename VALUE_TYPE, int CAPACITY>
inline bool HashSetHasSlot(HashSet<VALUE_TYPE, CAPACITY>* set)
{
	return set->numUsedSlots + set->numTombstones < CAPACITY;
}

template<typename VALUE_TYPE, int CAPACITY>
inline VALUE_TYPE* HashSetAdd(HashSet<VALUE_TYPE, CAPACITY>* set, const VALUE_TYPE& value)
{
	if (set->numUsedSlots + set->numTombstones == CAPACITY)
		return nullptr;

	int idx = hash(value) % CAPACITY;
	int firstTombstone = INT32_MAX;

	while (true)
	{
		HashSetSlotState state = set->slots[idx].state;
		if (state == HASHSET_SLOT_EMPTY)
		{
			int target = (firstTombstone != INT32_MAX ? firstTombstone : idx);
			HashSetSlot<VALUE_TYPE, CAPACITY>* slot = &set->slots[target];
			slot->value = value;

			if (slot->state == HASHSET_SLOT_TOMBSTONE)
				set->numTombstones--;

			slot->state = HASHSET_SLOT_USED;
			set->numUsedSlots++;
			return &slot->value;
		}
		else if (state == HASHSET_SLOT_TOMBSTONE)
		{
			if (firstTombstone == INT32_MAX)
				firstTombstone = idx;
		}
		else if (state == HASHSET_SLOT_USED && set->slots[idx].value == value)
		{
			return &set->slots[idx].value;
		}

		idx = (idx + 1) % CAPACITY;
	}
}

template<typename VALUE_TYPE, int CAPACITY>
inline VALUE_TYPE* HashSetRemove(HashSet<VALUE_TYPE, CAPACITY>* set, const VALUE_TYPE& value)
{
	int idx = hash(value) % CAPACITY;
	int firstIdx = idx;
	while (true)
	{
		HashSetSlotState state = set->slots[idx].state;
		if (state == HASHSET_SLOT_EMPTY)
		{
			return nullptr;
		}
		else if (state == HASHSET_SLOT_USED && set->slots[idx].value == value)
		{
			set->slots[idx].state = HASHSET_SLOT_TOMBSTONE;
			set->numUsedSlots--;
			set->numTombstones++;
			return &set->slots[idx].value;
		}

		idx = (idx + 1) % CAPACITY;

		if (idx == firstIdx)
		{
			return nullptr;
		}
	}
}

template<typename VALUE_TYPE, int CAPACITY>
inline VALUE_TYPE* HashSetContains(HashSet<VALUE_TYPE, CAPACITY>* set, const VALUE_TYPE& value)
{
	int idx = hash(value) % CAPACITY;
	int firstIdx = idx;
	while (true)
	{
		HashSetSlotState state = set->slots[idx].state;
		if (state == HASHSET_SLOT_EMPTY)
			return nullptr;
		else if (state == HASHSET_SLOT_USED && set->slots[idx].value == value)
			return &set->slots[idx].value;

		idx = (idx + 1) % CAPACITY;
		if (idx == firstIdx)
			return nullptr;
	}
}

template<typename VALUE_TYPE, int CAPACITY>
inline void ClearHashSet(HashSet<VALUE_TYPE, CAPACITY>* set)
{
	SDL_memset(set->slots, 0, sizeof(set->slots));

	set->numUsedSlots = 0;
	set->numTombstones = 0;
}
