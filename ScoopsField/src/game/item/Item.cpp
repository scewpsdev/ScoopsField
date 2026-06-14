#include "Item.h"


static void InitWeapon(ItemDatabase* items, Item* item, const char* name, bool twoHanded, int damage, vec2 damageRange, DamageType damageType)
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
	item->weapon.damageType = damageType;

	item->weapon.runningAttack = -1;
}

static void InitStaff(ItemDatabase* items, Item* item, const char* name, bool twoHanded, vec3 castOffset, DamageType damageType)
{
	item->twoHanded = twoHanded;

	char modelPath[256];
	SDL_snprintf(modelPath, 256, "res/items/%s/%s.glb.bin", name, name);
	LoadModel(&item->model, modelPath, false, cmdBuffer);

	char movesetPath[256];
	SDL_snprintf(movesetPath, 256, "res/items/%s/%s_moveset.glb.bin", name, name);
	LoadModel(&item->moveset, movesetPath, false, cmdBuffer);

	item->equipSound = &items->equipLightSound;

	item->weapon.castOffset = castOffset;
	item->weapon.damageType = damageType;

	item->weapon.runningAttack = -1;
	item->weapon.riposteAttack = -1;
}

static void AddAttackSound(Attack* attack, Sound* sound, float time, float volume, float speed, float pan)
{
	SDL_assert(attack->numSounds < MAX_ATTACK_SOUNDS);
	AttackSound* attackSound = &attack->sounds[attack->numSounds++];
	attackSound->sound = sound;
	attackSound->time = time;
	attackSound->volume = volume;
	attackSound->speed = speed;
	attackSound->pan = pan;
}

static void AddAttackEffect(Attack* attack, const char* path, float time, vec3 localPosition)
{
	SDL_assert(attack->numEffects < MAX_ATTACK_EFFECTS);
	AttackEffect* attackEffect = &attack->effects[attack->numEffects++];
	attackEffect->path = path;
	attackEffect->time = time;
	attackEffect->localPosition = localPosition;
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
	attack->damageType = item->weapon.damageType;
	attack->staminaCost = 0.1f;
	attack->followUp = followUp;
	attack->twoHanded = item->twoHanded;

	AddAttackSound(attack, &game->swingSound, attack->damageWindow.x, 1, 1, (attackID % 2 * -2 + 1) * 0.2f);

	return attackID;
}

static int AddBowDraw(Item* item, const char* name, const char* animation, float animationSpeed)
{
	int attackID = item->weapon.numAttacks++;
	Attack* attack = &item->weapon.attacks[attackID];
	attack->name = name;
	attack->animation = animation;
	attack->animationSpeed = animationSpeed;
	attack->itemAnimation = "bow_draw";
	attack->damageWindow = vec2(0);
	attack->followUpCancelTime = 0;
	attack->damageMultiplier = 1;
	attack->damageType = item->weapon.damageType;
	attack->staminaCost = 0.1f;
	attack->followUp = nullptr;
	attack->followUpCancelTime = GetAnimationByName(&item->moveset, animation)->duration;
	attack->stance = true;
	attack->bowDraw = true;
	attack->twoHanded = true;
	attack->canCancel = true;

	return attackID;
}

static int AddCast(Item* item, const char* name, const char* animation, float animationSpeed, float castTime)
{
	int attackID = item->weapon.numAttacks++;
	Attack* attack = &item->weapon.attacks[attackID];
	attack->name = name;
	attack->animation = animation;
	attack->animationSpeed = animationSpeed;
	attack->itemAnimation = "cast";
	attack->damageWindow = vec2(0);
	attack->followUpCancelTime = 0;
	attack->damageMultiplier = 1;
	attack->damageType = item->weapon.damageType;
	attack->staminaCost = 0.1f;
	attack->followUp = nullptr;
	attack->followUpCancelTime = GetAnimationByName(&item->moveset, animation)->duration;
	attack->projectileCast = true;
	attack->projectileCastTime = castTime;
	attack->twoHanded = item->twoHanded;

	return attackID;
}

static int AddBlock(Item* item, const char* name, const char* animation, float animationSpeed, int parryEndFrame)
{
	int attackID = item->weapon.numAttacks++;
	Attack* attack = &item->weapon.attacks[attackID];
	attack->name = name;
	attack->secondary = true;
	attack->stance = true;
	attack->animation = animation;
	attack->animationSpeed = animationSpeed;
	attack->parryWindow = vec2(0, (float)parryEndFrame) / 24.0f / animationSpeed;
	attack->blockWindow = vec2(0, 1000) / 24.0f / animationSpeed;
	attack->followUpCancelTime = parryEndFrame / 24.0f / animationSpeed + 0.4f;
	attack->twoHanded = item->twoHanded;

	return attackID;
}

static void InitWeapons(ItemDatabase* items)
{
	// kings sword
	{
		Item* item = &items->items[ITEM_KINGS_SWORD];
		InitWeapon(items, item, "kings_sword", false, 50, vec2(0.1f, 0.85f), DAMAGE_TYPE_SLASH);

		item->equipSound = &items->equipSwordSound;

		AddAttack(item, "attack1", "attack1", 1.0f, 10, 18, 24, 1, "attack2");
		AddAttack(item, "attack2", "attack2", 1.0f, 10, 18, 24, 1, "attack1");
		item->weapon.riposteAttack = AddAttack(item, "riposte", "attack_riposte", 1.0f, 13, 17, 20, 1, "attack1");
		AddBlock(item, "block", "block", 1, 6);
		item->weapon.runningAttack = AddAttack(item, "attack_running", "attack_running", 1.0f, 15, 22, 28, 1, "attack1");
	}
	// longsword
	{
		Item* item = &items->items[ITEM_LONGSWORD];
		InitWeapon(items, item, "longsword", true, 70, vec2(0.1f, 1.0f), DAMAGE_TYPE_SLASH);

		item->equipSound = &items->equipHeavySound;

		AddAttack(item, "attack1", "attack1", 1, 15, 24, 32, 1.0f, "attack2");
		AddAttack(item, "attack2", "attack2", 1, 15, 24, 32, 1.0f, "attack1");
		AddBlock(item, "block", "block", 1, 6);
	}
	// shortbow
	{
		Item* item = &items->items[ITEM_SHORTBOW];
		InitWeapon(items, item, "shortbow", true, 50, vec2(), DAMAGE_TYPE_BLUNT);

		item->equipSound = &items->equipLightSound;

		AddBowDraw(item, "draw", "attack1", 1);
		Attack* draw = &item->weapon.attacks[item->weapon.numAttacks - 1];
		AddAttackSound(draw, &items->bowDrawSound, 0.3f, 1, 1.1f, 0.1f);
		AddAttackSound(draw, &items->bowSetArrowSound, 6 / 24.0f, 1, 1, 0.1f);
	}
	// darkwood staff
	{
		Item* item = &items->items[ITEM_DARKWOOD_STAFF];
		InitStaff(items, item, "darkwood_staff", false, vec3(0, 0.275f, 0), DAMAGE_TYPE_BLUNT);

		item->equipSound = &items->equipLightSound;

		AddCast(item, "cast", "cast", 1, 24 / 24.0f);
		Attack* cast = &item->weapon.attacks[item->weapon.numAttacks - 1];
		AddAttackSound(cast, &items->spellCastSound, 24 / 24.0f, 1, 1, 0.1f);
		AddAttackEffect(cast, "effects/action/cast_charge.rfs", 0.0f, item->weapon.castOffset);
	}

	// arrow
	{
		Item* item = &items->items[ITEM_ARROW];
		InitWeapon(items, item, "arrow", false, 50, vec2(), DAMAGE_TYPE_NONE);
	}
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

static void InitShields(ItemDatabase* items)
{
	// wooden shield
	{
		Item* item = &items->items[ITEM_WOODEN_SHIELD];
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

	LoadSounds(&items->bowDrawSound, "sounds/item/bow_draw", 6);
	LoadSounds(&items->bowDrawQuickSound, "sounds/item/bow_draw_quick", 2);
	LoadSounds(&items->bowShootSound, "sounds/item/bow_shoot", 9);
	LoadSounds(&items->bowSetArrowSound, "sounds/item/bow_set_arrow", 8);

	LoadSound(&items->spellCastSound, "res/sounds/item/spell_cast.ogg.bin");

	InitWeapons(items);
	InitShields(items);
}

Item* GetItem(ItemType type)
{
	SDL_assert(type < ITEM_LAST);
	return &game->items.items[type];
}

Attack* GetAttackByName(Item* item, const char* name)
{
	for (int i = 0; i < item->weapon.numAttacks; i++)
	{
		if (SDL_strcmp(item->weapon.attacks[i].name, name) == 0)
			return &item->weapon.attacks[i];
	}
	return nullptr;
}

Attack* GetFirstAttack(Item* item, bool secondary)
{
	for (int i = 0; i < item->weapon.numAttacks; i++)
	{
		if (item->weapon.attacks[i].secondary == secondary)
			return &item->weapon.attacks[i];
	}
	return nullptr;
}
