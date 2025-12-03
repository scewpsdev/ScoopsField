#pragma once

#include "Vector.h"


struct AABB
{
	union
	{
		vec3 min = vec3::Zero;
		struct {
			float x0, y0, z0;
		};
	};
	union
	{
		vec3 max = vec3::Zero;
		struct {
			float x1, y1, z1;
		};
	};
};

struct Sphere
{
	union
	{
		vec3 center = vec3::Zero;
		struct {
			float xcenter, ycenter, zcenter;
		};
	};

	float radius = 0;

	Sphere()
	{
	}

	Sphere(vec3 center, float radius)
		: center(center), radius(radius)
	{
	}
};
