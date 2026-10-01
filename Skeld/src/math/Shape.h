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


	inline bool contains(vec3 position) const
	{
		return position.x >= x0 && position.x <= x1 &&
			position.y >= y0 && position.y <= y1 &&
			position.z >= z0 && position.z <= z1;
	}
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
