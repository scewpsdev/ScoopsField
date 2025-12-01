#include "Hash.h"


uint32_t hash(uint32_t i)
{
	i = (uint32_t)(i ^ (uint32_t)(61)) ^ (uint32_t)(i >> (uint32_t)(16));
	i *= (uint32_t)(9);
	i = i ^ (i >> 4);
	i *= (uint32_t)(0x27d4eb2d);
	i = i ^ (i >> 15);
	return i;
}

uint32_t hash(int i)
{
	return hash((uint32_t)i);
}

uint32_t hash(float f)
{
	return hash(*(uint32_t*)&f);
}

uint32_t hash(const vec3& v)
{
	uint32_t u = hash(v.x);
	u = u * 19 + hash(v.y);
	u = u * 19 + hash(v.z);
	return u;
}

uint32_t hash(const Quaternion& q)
{
	uint32_t u = hash(q.x);
	u = u * 19 + hash(q.y);
	u = u * 19 + hash(q.z);
	u = u * 19 + hash(q.w);
	return u;
}

// https://stackoverflow.com/questions/2624192/good-hash-function-for-strings
uint32_t hash(const char* str)
{
	uint32_t hash = 7;
	int i = 0;
	while (char c = str[i++])
	{
		hash = hash * 31 + c;
	}
	return hash;
}

uint32_t hash(const void* ptr)
{
	return (uint32_t)(uint64_t)ptr;
}

uint32_t hashCombine(uint32_t h0, uint32_t h1)
{
	return h0 * 19 + h1;
}
