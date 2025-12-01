#include "Math.h"

#include "utils/Random.h"

#include <math.h>
#include <float.h>


int fsign(float f)
{
	return f < 0.0f ? -1 : f > 0.0f ? 1 : 0;
}

float radians(float degrees)
{
	return degrees / 180.0f * PI;
}

float degrees(float radians)
{
	return radians / PI * 180.0f;
}

static float nextGaussian = FLT_MAX;
static float RandomGaussian(Random& random)
{
	if (nextGaussian == FLT_MAX)
	{
		float u1 = random.nextFloat();
		float u2 = random.nextFloat();
		float r = sqrtf(-2 * logf(u1));
		float t = 2 * PI * u2;
		float x = r * cosf(t);
		float y = r * sinf(t);
		nextGaussian = y;
		return x;
	}
	else
	{
		float r = nextGaussian;
		nextGaussian = FLT_MAX;
		return r;
	}
}

vec3 RandomPointOnSphere(Random& random)
{
	float x = RandomGaussian(random);
	float y = RandomGaussian(random);
	float z = RandomGaussian(random);
	vec3 p = vec3(x, y, z);
	return p.normalized();
}

AABB TransformBoundingBox(const AABB& localBox, const Matrix& transform)
{
	vec3 size = localBox.max - localBox.min;
	vec3 corners[] =
	{
		localBox.min,
		localBox.min + vec3(size.x, 0, 0),
		localBox.min + vec3(0, size.y, 0),
		localBox.min + vec3(0, 0, size.z),
		localBox.min + vec3(size.xy, 0),
		localBox.min + vec3(0, size.yz),
		localBox.min + vec3(size.x, 0, size.z),
		localBox.min + size
	};
	vec3 aabbMin = vec3(FLT_MAX), aabbMax = vec3(FLT_MIN);
	for (int i = 0; i < 8; i++)
	{
		vec3 corner = transform * corners[i];

		aabbMin.x = fminf(aabbMin.x, corner.x);
		aabbMax.x = fmaxf(aabbMax.x, corner.x);

		aabbMin.y = fminf(aabbMin.y, corner.y);
		aabbMax.y = fmaxf(aabbMax.y, corner.y);

		aabbMin.z = fminf(aabbMin.z, corner.z);
		aabbMax.z = fmaxf(aabbMax.z, corner.z);
	}

	AABB worldSpaceBox;
	worldSpaceBox.min = aabbMin;
	worldSpaceBox.max = aabbMax;
	return worldSpaceBox;
}

ivec2 WorldToScreenSpace(const vec3& p, const Matrix& vp, int displayWidth, int displayHeight)
{
	vec4 clipSpacePosition = vp * vec4(p, 1.0f);
	vec3 ndcSpacePosition = clipSpacePosition.xyz / clipSpacePosition.w;
	if (ndcSpacePosition.z >= -1.0f && ndcSpacePosition.z <= 1.0f)
	{
		vec2 windowSpacePosition = ndcSpacePosition.xy * 0.5f + 0.5f;
		ivec2 pixelPosition = ivec2(
			(int)(windowSpacePosition.x * displayWidth + 0.5f),
			displayHeight - (int)(windowSpacePosition.y * displayHeight + 0.5f)
		);
		return pixelPosition;
	}
	else
	{
		return ivec2(-1, -1);
	}
}

vec4 ARGBToVector(uint32_t argb)
{
	uint8_t a = (argb & 0xFF000000) >> 24;
	uint8_t r = (argb & 0x00FF0000) >> 16;
	uint8_t g = (argb & 0x0000FF00) >> 8;
	uint8_t b = (argb & 0x000000FF) >> 0;
	return vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
