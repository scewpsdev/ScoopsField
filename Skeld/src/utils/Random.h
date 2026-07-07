#pragma once

#include "Hash.h"
#include "math/Math.h"

#include <stdint.h>
#include <string.h>


struct Random
{
	uint32_t v;


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

	vec2 nextVector2(float min, float max)
	{
		return vec2(
			nextFloat(min, max),
			nextFloat(min, max)
		);
	}

	vec3 nextVector3(float min, float max)
	{
		return vec3(
			nextFloat(min, max),
			nextFloat(min, max),
			nextFloat(min, max)
		);
	}

	vec3 randomDirection(vec3 direction, float randomness, bool uniform)
	{
		float minCosTheta = 1 - 2 * randomness;

		float cosTheta = uniform ? mix(minCosTheta, 1.0f, nextFloat()) : SDL_cosf(mix(0.0f, SDL_acosf(minCosTheta), nextFloat()));
		float sinTheta = SDL_sqrtf(1 - cosTheta * cosTheta);
		float phi = 2 * PI * nextFloat();

		vec3 localDirection = vec3(SDL_cosf(phi) * sinTheta, SDL_sinf(phi) * sinTheta, cosTheta);
		vec3 right = SDL_fabsf(direction.z) < 0.999f ? vec3(0, 0, 1) : vec3(1, 0, 0);
		vec3 tangent = cross(right, direction).normalized();
		vec3 bitangent = cross(direction, tangent);

		return tangent * localDirection.x + bitangent * localDirection.y + direction * localDirection.z;
	}

	void nextBytes(uint8_t* bytes, int size)
	{
		int numInts = (size + 3) / 4;
		for (int i = 0; i < numInts; i++)
		{
			uint32_t i32 = next();
			memcpy(&bytes[i * 4], &i32, min(4, size - i * 4));
		}
	}
};


inline void InitRandom(Random* random, uint32_t seed)
{
	random->v = hash(seed);
}
