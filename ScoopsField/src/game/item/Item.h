#pragma once

#include "model/Model.h"

#include "audio/Audio.h"


enum ItemType
{
	ITEM_TYPE_NONE,

	ITEM_TYPE_KINGS_SWORD,
	ITEM_TYPE_LONGSWORD,
	ITEM_TYPE_WOODEN_SHIELD,

	ITEM_TYPE_LAST
};

struct Attack
{
	const char* name;
	const char* animation;
	float animationSpeed;
	vec2 damageWindow;
	float followUpCancelTime;
	float damageMultiplier;

	const char* followUp;
};

struct Weapon
{
	int damage;
	vec2 damageRange;

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
	Item items[ITEM_TYPE_LAST];

	Sound equipLight;
	Sound equipHeavy;
	Sound equipSword;
	Sound equipArmor;
};

void InitItemDatabase(ItemDatabase* items, SDL_GPUCommandBuffer* cmdBuffer);

Item* GetItem(ItemType type);

Attack* GetNextAttack(Attack* attack, Item* item);
