#pragma once

#include "game/entity/EntityBase.h"


struct Portal : EntityBase
{
	Portal* destination;
};


void InitPortal(Portal* portal, vec3 position, quat rotation);
void DestroyPortal(Portal* portal);

void UpdatePortal(Portal* portal);
void RenderPortal(Portal* portal);
