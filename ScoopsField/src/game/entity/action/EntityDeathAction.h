#pragma once


struct EntityDeathAction
{
	float duration;
};


void InitEntityDeathAction(struct EntityAction* action);
void StartEntityDeathAction(struct EntityAction* action, struct Entity* entity);
void StopEntityDeathAction(struct EntityAction* action, struct Entity* entity);
void UpdateEntityDeathAction(struct EntityAction* action, struct Entity* entity);
