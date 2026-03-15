#include "Item.h"


static void InitWeapon(ItemDatabase* items, Item* item, const char* name, bool twoHanded, int damage, vec2 damageRange)
{
	item->twoHanded = twoHanded;

	char modelPath[256];
	SDL_snprintf(modelPath, 256, "res/items/%s/%s.glb.bin", name, name);
	LoadModel(&item->model, modelPath, false, cmdBuffer);

	char movesetPath[256];
	SDL_snprintf(movesetPath, 256, "res/items/%s/%s_moveset.glb.bin", name, name);
	LoadModel(&item->moveset, movesetPath, false, cmdBuffer);

	item->equipSound = &items->equipLightSound;

	item->weapon.damage = damage;
	item->weapon.damageRange = damageRange;

	item->weapon.runningAttack = -1;
}

static int AddAttack(Item* item, const char* name, const char* animation, float animationSpeed, int damageStartFrame, int damageEndFrame, int cancelFrame, float damageMultiplier, const char* followUp = nullptr)
{
	int attackID = item->weapon.numAttacks++;
	Attack* attack = &item->weapon.attacks[attackID];
	attack->name = name;
	attack->animation = animation;
	attack->animationSpeed = animationSpeed;
	attack->damageWindow = vec2((float)damageStartFrame, (float)damageEndFrame) / 24.0f / animationSpeed;
	attack->followUpCancelTime = cancelFrame / 24.0f / animationSpeed;
	attack->damageMultiplier = damageMultiplier;
	attack->followUp = followUp;

	return attackID;
}

static void InitShield(ItemDatabase* items, Item* item, const char* name, bool twoHanded)
{
	item->twoHanded = twoHanded;

	char modelPath[256];
	SDL_snprintf(modelPath, 256, "res/items/%s/%s.glb.bin", name, name);
	LoadModel(&item->model, modelPath, false, cmdBuffer);

	char movesetPath[256];
	SDL_snprintf(movesetPath, 256, "res/items/%s/%s_moveset.glb.bin", name, name);
	LoadModel(&item->moveset, movesetPath, false, cmdBuffer);

	item->equipSound = &items->equipLightSound;

	item->weapon.damage = 0;
	item->weapon.damageRange = vec2(0);

	item->weapon.runningAttack = -1;
}

static void InitWeapons(ItemDatabase* items)
{
	// kings sword
	{
		Item* item = &items->items[ITEM_TYPE_KINGS_SWORD];
		InitWeapon(items, item, "kings_sword", false, 50, vec2(0.1f, 0.85f));

		item->equipSound = &items->equipSwordSound;

		AddAttack(item, "attack1", "attack1", 1.0f, 10, 18, 24, 1.0f, "attack2");
		AddAttack(item, "attack2", "attack2", 1.0f, 10, 18, 24, 1.0f, "attack1");
		item->weapon.runningAttack = AddAttack(item, "attack_running", "attack_running", 1.0f, 15, 22, 28, 1.0f, "attack1");
	}
	// longsword
	{
		Item* item = &items->items[ITEM_TYPE_LONGSWORD];
		InitWeapon(items, item, "longsword", true, 70, vec2(0.1f, 1.0f));

		item->equipSound = &items->equipHeavySound;

		AddAttack(item, "attack1", "attack1", 1, 15, 24, 32, 1.0f, "attack2");
		AddAttack(item, "attack2", "attack2", 1, 15, 24, 32, 1.0f, "attack1");
	}
}

static void InitShields(ItemDatabase* items)
{
	// wooden shield
	{
		Item* item = &items->items[ITEM_TYPE_WOODEN_SHIELD];
		InitShield(items, item, "wooden_shield", false);
	}
}

void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer)
{
	LoadSound(&items->equipLightSound, "res/sounds/item/equip_light.ogg.bin");
	LoadSound(&items->equipHeavySound, "res/sounds/item/equip_heavy.ogg.bin");
	LoadSound(&items->equipSwordSound, "res/sounds/item/equip_sword.ogg.bin");
	LoadSound(&items->equipArmorSound, "res/sounds/item/equip_armor.ogg.bin");
	LoadSound(&items->clothSound, "res/sounds/item/cloth.ogg.bin");

	InitWeapons(items);
	InitShields(items);
}

Item* GetItem(ItemType type)
{
	SDL_assert(type < ITEM_TYPE_LAST);
	return &game->items.items[type];
}

Attack* GetAttackByName(Item* item, const char* name)
{
	for (int i = 0; i < item->weapon.numAttacks; i++)
	{
		if (SDL_strcmp(item->weapon.attacks[i].animation, name) == 0)
			return &item->weapon.attacks[i];
	}
	return nullptr;
}

Attack* GetNextAttack(Attack* attack, Item* item)
{
	if (attack->followUp)
	{
		return GetAttackByName(item, attack->followUp);
	}
	else
	{
		return &item->weapon.attacks[0];
	}
}
