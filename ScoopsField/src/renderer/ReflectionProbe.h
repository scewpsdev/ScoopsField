#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"

#include <SDL3/SDL.h>


struct RenderTarget;

struct ReflectionProbe
{
	vec3 position;
	vec3 size;

	RenderTarget* cubemap;
	RenderTarget* irradiance;
};


void InitReflectionProbe(ReflectionProbe* probe, vec3 position, vec3 size);
