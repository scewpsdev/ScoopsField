#include "Math.h"

#include "utils/Random.h"

#include <math.h>
#include <float.h>


float radians(float degrees)
{
	return degrees / 180.0f * PI;
}

float degrees(float radians)
{
	return radians / PI * 180.0f;
}

float lerpAngle(float a, float b, float t)
{
	a = SDL_fmodf(a + PI * 2.0f, PI * 2.0f);
	b = SDL_fmodf(b + PI * 2.0f, PI * 2.0f);

	if (a - b > PI)
		a -= PI * 2.0f;
	else if (b - a > PI)
		b -= PI * 2.0f;

	return a + (b - a) * t;
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

AABB TransformBoundingBox(const AABB& localBox, const mat4& transform)
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

ivec2 WorldToScreenSpace(const vec3& p, const mat4& vp, int displayWidth, int displayHeight)
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

vec3 SRGBToLinear(vec3 color)
{
	const float gamma = 2.2f;
	return pow(color, gamma);
}

vec4 SRGBToLinear(vec4 color)
{
	return vec4(SRGBToLinear(color.xyz), color.a);
}

vec3 DecodeRG11B10(uint32_t bits)
{
	uint32_t rb = (bits & 0b11111111111);
	uint32_t gb = (bits & 0b1111111111100000000000) >> 11;
	uint32_t bb = (bits & 0b11111111110000000000000000000000) >> 22;
	//uint32_t rb = buffer[0] << 3 + (buffer[1] & 0b11100000) >> 5;
	//uint32_t gb = buffer[1] & 0b11111 << 6 + buffer[2] & 0b11111100 >> 2;
	//uint32_t bb = buffer[2] & 0b11 << 8 + buffer[3];
	uint32_t expr = (rb & 0b11111000000) >> 6;
	uint32_t expg = (gb & 0b11111000000) >> 6;
	uint32_t expb = (bb & 0b1111100000) >> 5;
	uint32_t manr = rb & 0b111111;
	uint32_t mang = gb & 0b111111;
	uint32_t manb = bb & 0b11111;
	float r = powf(2, (float)expr - 15) * (1.0f + manr / (float)0b1000000);
	float g = powf(2, (float)expg - 15) * (1.0f + mang / (float)0b1000000);
	float b = powf(2, (float)expb - 15) * (1.0f + manb / (float)0b100000);
	return vec3(r, g, b);
}

bool FrustumCulling(const Sphere& boundingSphere, vec4 planes[6])
{
	vec3 boundingSpherePos = boundingSphere.center;
	float boundingSphereRadius = boundingSphere.radius;

	for (int i = 0; i < 6; i++)
	{
		float distance = boundingSpherePos.x * planes[i].x + boundingSpherePos.y * planes[i].y + boundingSpherePos.z * planes[i].z;
		float l = sqrtf(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);
		distance += planes[i].w / l;
		if (distance + boundingSphereRadius < 0.0f)
			return false;
	}
	return true;
}

bool FrustumCulling(const Sphere& boundingSphere, mat4 transform, vec4 planes[6])
{
	vec4 boundingSpherePos = (transform * vec4(boundingSphere.center, 1.0f));
	vec3 scale = transform.scale();
	float boundingSphereRadius = max(max(scale.x, scale.y), scale.z) * boundingSphere.radius;

	for (int i = 0; i < 6; i++)
	{
		float distance = boundingSpherePos.x * planes[i].x + boundingSpherePos.y * planes[i].y + boundingSpherePos.z * planes[i].z;
		float l = sqrtf(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);
		distance += planes[i].w / l;
		if (distance + boundingSphereRadius < 0.0f)
			return false;
	}
	return true;
}
