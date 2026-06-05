#pragma once

#include "model/Model.h"

#include "audio/Audio.h"


enum ItemType
{
	ITEM_NONE,

	ITEM_KINGS_SWORD,
	ITEM_LONGSWORD,

	ITEM_SHORTBOW,

	ITEM_DARKWOOD_STAFF,

	ITEM_WOODEN_SHIELD,

	ITEM_ARROW,

	ITEM_LAST
};

struct AttackSound
{
	Sound* sound;
	float time;
	float volume;
	float speed;
	float pan;
};

struct AttackEffect
{
	const char* path;
	float time;
	vec3 localPosition;
};

struct Attack
{
	const char* name;
	bool secondary;
	bool twoHanded;
	bool stance;
	bool projectileShoot;

	bool projectileCast;
	float projectileCastTime;

	const char* animation;
	float animationSpeed;
	const char* itemAnimation;

	vec2 damageWindow;
	vec2 blockWindow;
	vec2 parryWindow;
	float followUpCancelTime;
	float damageMultiplier;
	float staminaCost;

	const char* followUp;

#define MAX_ATTACK_SOUNDS 8
	AttackSound sounds[MAX_ATTACK_SOUNDS];
	int numSounds;

#define MAX_ATTACK_EFFECTS 8
	AttackEffect effects[MAX_ATTACK_EFFECTS];
	int numEffects;
};

struct Weapon
{
	int damage;
	vec2 damageRange;

	vec3 castOffset;

#define MAX_WEAPON_ATTACKS 16
	Attack attacks[MAX_WEAPON_ATTACKS];
	int numAttacks;
	int runningAttack;
};

struct Item
{
	Model model;
	Model moveset;
	bool twoHanded;
	Sound* equipSound;

	union {
		Weapon weapon;
	};
};

struct ItemDatabase
{
	Item items[ITEM_LAST];

	Sound equipLightSound;
	Sound equipHeavySound;
	Sound equipSwordSound;
	Sound equipArmorSound;
	Sound clothSound;
	Sound bowDrawSound;
	Sound bowDrawQuickSound;
	Sound bowShootSound;
	Sound bowSetArrowSound;
	Sound spellCastSound;
};

void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer);

Item* GetItem(ItemType type);

Attack* GetFirstAttack(Item* item, bool secondary);
Attack* GetNextAttack(Attack* attack, Item* item);
