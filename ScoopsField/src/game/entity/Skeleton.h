#pragma once

#include "Entity.h"

#include "action/EntityAction.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "physics/RigidBody.h"

#include <SDL3/SDL.h>


struct SkeletonEntity : Entity
{
	vec3 position;
	float rotation;

	bool moving;

	Model* model;
	AnimationState anim;
	RigidBody body;

	AnimationPlayback idleAnim;
	AnimationPlayback runAnim;
	AnimationPlayback attackAnim;

	EntityActionManager actions;

	int health;
	int maxHealth;
};
