#include "Item.h"


static void InitWeapon(Item* item, const char* name, bool twoHanded, int damage, vec2 damageRange)
{
	item->twoHanded = twoHanded;

	item->weapon.damage = damage;
	item->weapon.damageRange = damageRange;

	char modelPath[256];
	SDL_snprintf(modelPath, 256, "res/items/%s/%s.glb.bin", name, name);
	LoadModel(&item->model, modelPath, false, cmdBuffer);

	char movesetPath[256];
	SDL_snprintf(movesetPath, 256, "res/items/%s/%s_moveset.glb.bin", name, name);
	LoadModel(&item->moveset, movesetPath, false, cmdBuffer);
}

static void AddAttack(Item* item, const char* animation, float animationSpeed, int damageStartFrame, int damageEndFrame, int cancelFrame, float damageMultiplier)
{
	Attack* attack = &item->weapon.attacks[item->weapon.numAttacks++];
	attack->animation = animation;
	attack->animationSpeed = animationSpeed;
	attack->damageWindow = vec2((float)damageStartFrame, (float)damageEndFrame) / 24.0f / animationSpeed;
	attack->followUpCancelTime = cancelFrame / 24.0f / animationSpeed;
	attack->damageMultiplier = damageMultiplier;
}

static void InitShield(Item* item, const char* name, bool twoHanded)
{
	item->twoHanded = twoHanded;

	item->weapon.damage = 0;
	item->weapon.damageRange = vec2(0);

	char modelPath[256];
	SDL_snprintf(modelPath, 256, "res/items/%s/%s.glb.bin", name, name);
	LoadModel(&item->model, modelPath, false, cmdBuffer);

	char movesetPath[256];
	SDL_snprintf(movesetPath, 256, "res/items/%s/%s_moveset.glb.bin", name, name);
	LoadModel(&item->moveset, movesetPath, false, cmdBuffer);
}

static void InitWeapons(ItemDatabase* items)
{
	// kings sword
	{
		Item* item = &items->items[ITEM_TYPE_KINGS_SWORD];
		InitWeapon(item, "kings_sword", false, 50, vec2(0.1f, 0.85f));
		AddAttack(item, "attack1", 1.5f, 10, 18, 24, 1.0f);
		AddAttack(item, "attack2", 1.5f, 10, 18, 24, 1.0f);
	}
	// longsword
	{
		Item* item = &items->items[ITEM_TYPE_LONGSWORD];
		InitWeapon(item, "longsword", true, 70, vec2(0.1f, 1.0f));
		AddAttack(item, "attack1", 1, 15, 24, 40, 1.0f);
		AddAttack(item, "attack2", 1, 15, 24, 40, 1.0f);
	}
}

static void InitShields(ItemDatabase* items)
{
	// wooden shield
	{
		Item* item = &items->items[ITEM_TYPE_WOODEN_SHIELD];
		InitShield(item, "wooden_shield", false);
	}
}

void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer)
{
	InitWeapons(items);
	InitShields(items);
}
