#pragma once

#include "Hash.h"

#include <SDL3/SDL.h>


enum SlotState : uint8_t
{
	SLOT_EMPTY = 0,
	SLOT_USED = 1,
	SLOT_TOMBSTONE = 2,
};

template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
struct HashSlot
{
	KEY_TYPE key;
	VALUE_TYPE value;
	SlotState state;
};

template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
struct HashMap
{
	HashSlot<KEY_TYPE, VALUE_TYPE, CAPACITY> slots[CAPACITY];
	int capacity;
	int numUsedSlots;
	int numTombstones;

	inline VALUE_TYPE& operator[](const KEY_TYPE& key)
	{
		VALUE_TYPE* value = HashMapGet(this, key);
		if (!value)
			value = HashMapAdd(this, key, VALUE_TYPE{});
		return *value;
	}
};


template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
inline void InitHashMap(HashMap<KEY_TYPE, VALUE_TYPE, CAPACITY>* map)
{
	map->numUsedSlots = 0;
	map->numTombstones = 0;
	map->capacity = CAPACITY;

	SDL_memset(map->slots, 0, sizeof(map->slots));
}

template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
inline VALUE_TYPE* HashMapAdd(HashMap<KEY_TYPE, VALUE_TYPE, CAPACITY>* map, const KEY_TYPE& key, const VALUE_TYPE& value)
{
	if (map->numUsedSlots + map->numTombstones == CAPACITY)
		return nullptr;

	int idx = hash(key) % CAPACITY;
	int firstTombstone = INT32_MAX;

	while (true)
	{
		SlotState state = map->slots[idx].state;
		if (state == SLOT_EMPTY)
		{
			int target = (firstTombstone != INT32_MAX ? firstTombstone : idx);
			HashSlot<KEY_TYPE, VALUE_TYPE, CAPACITY>* slot = &map->slots[target];
			slot->key = key;
			slot->value = value;

			if (slot->state == SLOT_TOMBSTONE)
				map->numTombstones--;

			slot->state = SLOT_USED;
			map->numUsedSlots++;
			return &slot->value;
		}
		else if (state == SLOT_TOMBSTONE)
		{
			if (firstTombstone == INT32_MAX)
				firstTombstone = idx;
		}
		else if (state == SLOT_USED && map->slots[idx].key == key)
		{
			map->slots[idx].value = value;
			return &map->slots[idx].value;
		}

		idx = (idx + 1) % CAPACITY;
	}
}

template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
inline bool HashMapRemove(HashMap<KEY_TYPE, VALUE_TYPE, CAPACITY>* map, const KEY_TYPE& key)
{
	int idx = hash(key) % CAPACITY;
	int firstIdx = idx;
	while (true)
	{
		SlotState state = map->slots[idx].state;
		if (state == SLOT_EMPTY)
			return false;
		else if (state == SLOT_USED && map->slots[idx].key == key)
		{
			map->slots[idx].state = SLOT_TOMBSTONE;
			map->numUsedSlots--;
			map->numTombstones++;
			return true;
		}

		idx = (idx + 1) % CAPACITY;
		if (idx == firstIdx)
			return false;
	}
}

template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
inline VALUE_TYPE* HashMapGet(HashMap<KEY_TYPE, VALUE_TYPE, CAPACITY>* map, const KEY_TYPE& key)
{
	int idx = hash(key) % CAPACITY;
	int firstIdx = idx;
	while (true)
	{
		SlotState state = map->slots[idx].state;
		if (state == SLOT_EMPTY)
			return nullptr;
		else if (state == SLOT_USED && map->slots[idx].key == key)
			return &map->slots[idx].value;

		idx = (idx + 1) % CAPACITY;
		if (idx == firstIdx)
			return nullptr;
	}
}

template<typename KEY_TYPE, typename VALUE_TYPE, int CAPACITY>
inline void HashMapClear(HashMap<KEY_TYPE, VALUE_TYPE, CAPACITY>* map)
{
	SDL_memset(map->slots, 0, sizeof(map->slots));

	map->numUsedSlots = 0;
	map->numTombstones = 0;
}
