#pragma once

#include "Vector.h"


struct AABB
{
	union
	{
		vec3 min;
		struct {
			float x0, y0, z0;
		};
	};
	union
	{
		vec3 max;
		struct {
			float x1, y1, z1;
		};
	};
};

struct Sphere
{
	union
	{
		vec3 center;
		struct {
			float xcenter, ycenter, zcenter;
		};
	};

	float radius;
};
