#pragma once

#include "game/entity/EntityBase.h"


struct Elevator : EntityBase
{
	Model* platform;
	Model* button;
	RigidBody body;
	RigidBody buttonBody;

	float shaftHeight;
	float platformHeight;
	float velocity;
	bool moving;

	float buttonHeight;
	float buttonActivatedTime;
	float buttonDeactivatedTime;

	bool toggled;
	bool buttonActive;
};


void InitElevator(Elevator* elevator, vec3 position, quat rotation, float shaftHeight);
void DestroyElevator(Elevator* elevator);

void UpdateElevator(Elevator* elevator);
void RenderElevator(Elevator* elevator);
