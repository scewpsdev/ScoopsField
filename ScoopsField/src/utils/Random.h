#pragma once

#include "Hash.h"
#include "math/Math.h"

#include <stdint.h>
#include <string.h>


struct Random
{
	uint32_t v;


	Random()
		: v(0)
	{
	}

	Random(uint32_t seed)
		: v(hash(seed))
	{
	}

	uint32_t next()
	{
		uint32_t value = v;
		v = hash(v);
		return value;
	}

	float nextFloat()
	{
		uint32_t value = next();
		return value / (float)UINT32_MAX;
	}

	float nextFloat(float min, float max)
	{
		return min + (max - min) * nextFloat();
	}

	vec3 nextVector3(float min, float max)
	{
		return vec3(
			nextFloat(min, max),
			nextFloat(min, max),
			nextFloat(min, max)
		);
	}

	void nextBytes(uint8_t* bytes, int size)
	{
		int numInts = (size + 3) / 4;
		for (int i = 0; i < numInts; i++)
		{
			uint32_t i32 = next();
			memcpy(&bytes[i * 4], &i32, min(4, size - numInts * 4));
		}
	}
};
