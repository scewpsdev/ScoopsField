#pragma once

#include "model/Model.h"


enum ItemType
{
	ITEM_TYPE_NONE,

	ITEM_TYPE_KINGS_SWORD,

	ITEM_TYPE_LAST
};

struct Item
{
	Model model;
	Model moveset;
};

struct ItemDatabase
{
	Item items[ITEM_TYPE_LAST];
};

void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer);
