#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"

#include <SDL3/SDL.h>


struct RenderTarget;

struct ReflectionProbe
{
	RenderTarget* renderTarget;
	vec3 position;
	vec3 size;
};


void InitReflectionProbe(ReflectionProbe* probe, vec3 position, vec3 size);
