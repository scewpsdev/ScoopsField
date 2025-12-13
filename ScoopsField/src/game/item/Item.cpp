#include "Item.h"


static void InitWeapon(Item* item, int damage, vec2 damageRange)
{
	item->weapon.damage = damage;
	item->weapon.damageRange = damageRange;
}

static void AddAttack(Item* item, const char* animation, int damageStartFrame, int damageEndFrame, int cancelFrame, float damageMultiplier)
{
	Attack* attack = &item->weapon.attacks[item->weapon.numAttacks++];
	attack->animation = animation;
	attack->damageWindow = vec2((float)damageStartFrame, (float)damageEndFrame) / 24.0f;
	attack->followUpCancelTime = cancelFrame / 24.0f;
	attack->damageMultiplier = damageMultiplier;
}

void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer)
{
	// kings sword
	{
		Item* item = &items->items[ITEM_TYPE_KINGS_SWORD];
		LoadModel(&item->model, "res/items/kings_sword/kings_sword.glb.bin", cmdBuffer);
		LoadModel(&item->moveset, "res/items/kings_sword/kings_sword_moveset.glb.bin", cmdBuffer);
		InitWeapon(item, 50, vec2(0.1f, 0.85f));
		AddAttack(item, "attack1", 10, 18, 24, 1.0f);
		AddAttack(item, "attack2", 10, 18, 24, 1.0f);
	}
}
