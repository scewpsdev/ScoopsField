#include "Item.h"


void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer)
{
	// kings sword
	{
		Item* item = &items->items[ITEM_TYPE_KINGS_SWORD];
		LoadModel(&item->model, "res/items/kings_sword/kings_sword.glb.bin", cmdBuffer);
		LoadModel(&item->moveset, "res/items/kings_sword/kings_sword_moveset.glb.bin", cmdBuffer);
	}
}
