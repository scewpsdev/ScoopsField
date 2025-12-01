#pragma once

#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"

#include <SDL3/SDL.h>


#define PI 3.14159265359f
#define Deg2Rad (PI / 180.0f)
#define Rad2Deg (180.0f / PI)


struct AABB
{
	vec3 min;
	vec3 max;
};


inline int ipow(int base, int exp)
{
	if (!exp)
		return 1;
	int result = 1;
	for (;;)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		if (!exp)
			break;
		base *= base;
	}

	return result;
}

inline int idivfloor(int a, int b)
{
	int q = a / b;
	int r = a % b;
	// If remainder != 0 and signs differ, round down
	if ((r != 0) && ((a ^ b) < 0))
		q -= 1;
	return q;
}

inline float clamp(float f, float min, float max)
{
	return SDL_min(SDL_max(f, min), max);
}

inline float remap(float x, float min, float max, float newMin, float newMax)
{
	return (x - min) / (max - min) * (newMax - newMin) + newMin;
}

inline float smoothstep(float edge0, float edge1, float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp(remap(x, edge0, edge1, 0, 1), 0, 1);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

int fsign(float f);

float radians(float degrees);
float degrees(float radians);

inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }
inline float min(float a, float b) { return a < b ? a : b; }
inline float max(float a, float b) { return a > b ? a : b; }
template<typename T>
inline T min(T a, T b) { return a < b ? a : b; }
template<typename T>
inline T max(T a, T b) { return a > b ? a : b; }

vec3 RandomPointOnSphere(struct Random& random);
AABB TransformBoundingBox(const AABB& localBox, const Matrix& transform);
ivec2 WorldToScreenSpace(const vec3& p, const Matrix& vp, int displayWidth, int displayHeight);
vec4 ARGBToVector(uint32_t argb);
