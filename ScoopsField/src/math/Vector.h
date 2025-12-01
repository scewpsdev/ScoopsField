#pragma once

#include <stdint.h>


struct vec2
{
	float x, y;


	vec2();
	vec2(float f);
	vec2(float x, float y);

	float lengthSquared() const;
	float length() const;
	vec2 normalized() const;

	float angle() const;
	vec2 rotate(float angle) const;

	vec2 operator-() const;

	static const vec2 Zero;
	static const vec2 One;
	static const vec2 Left;
	static const vec2 Right;
	static const vec2 Down;
	static const vec2 Up;
	static const vec2 AxisX;
	static const vec2 AxisY;
};

struct ivec3;
struct vec3
{
	union {
		struct {
			float x, y, z;
		};
		struct {
			vec2 xy;
			float z;
		};
		struct {
			float x;
			vec2 yz;
		};
	};


	vec3();
	vec3(float xyz);
	vec3(float x, float y, float z);
	vec3(const vec2& xy, float z);
	vec3(float x, const vec2& yz);

	float lengthSquared() const;
	float length() const;
	vec3 normalized() const;
	float max() const;

	vec3 operator-() const;

	vec3& operator+=(const vec3& v);
	vec3& operator-=(const vec3& v);
	vec3& operator*=(const vec3& v);
	vec3& operator/=(const vec3& v);

	operator ivec3() const;

	static const vec3 Zero;
	static const vec3 One;
	static const vec3 Left;
	static const vec3 Right;
	static const vec3 Down;
	static const vec3 Up;
	static const vec3 Forward;
	static const vec3 Back;
	static const vec3 AxisX;
	static const vec3 AxisY;
	static const vec3 AxisZ;

	static void OrthoNormalize(const vec3& normal, vec3& tangent);
};

struct vec4
{
	union
	{
		struct {
			float x, y, z, w;
		};
		struct {
			vec3 xyz;
			float w;
		};
		struct {
			vec2 xy;
			vec2 zw;
		};
		struct {
			float x;
			vec3 yzw;
		};
		struct {
			float x;
			vec2 yz;
			float w;
		};
		struct {
			float r, g, b, a;
		};
		struct {
			vec3 rgb;
			float a;
		};
		struct {
			vec2 rg;
			vec2 ba;
		};
		struct {
			float r;
			vec3 gba;
		};
		struct {
			float r;
			vec2 gb;
			float a;
		};
		float elements[4];
	};


	vec4();
	vec4(float f);
	vec4(float x, float y, float z, float w);
	vec4(const vec3& xyz, float w);
	vec4(float x, const vec3& yzw);

	float& operator[](int index);
	const float& operator[](int index) const;

	vec4& operator+=(const vec4& v);
	vec4& operator-=(const vec4& v);
	vec4& operator*=(const vec4& v);
	vec4& operator/=(const vec4& v);

	vec4& operator+=(float f);
	vec4& operator-=(float f);
	vec4& operator*=(float f);
	vec4& operator/=(float f);

	static const vec4 Zero;
	static const vec4 One;
};

struct ivec2
{
	int x, y;


	ivec2();
	ivec2(int i);
	ivec2(int x, int y);
};

struct ivec3
{
	union {
		struct {
			int x, y, z;
		};
		struct {
			ivec2 xy;
			int z;
		};
		struct {
			int x;
			ivec2 yz;
		};
		int elements[3];
	};


	ivec3();
	ivec3(int i);
	ivec3(int x, int y, int z);

	int& operator[](int idx);

	ivec3 operator-() const;

	operator vec3() const;

	static const ivec3 Zero;
	static const ivec3 One;
	static const ivec3 Left;
	static const ivec3 Right;
	static const ivec3 Down;
	static const ivec3 Up;
	static const ivec3 Forward;
	static const ivec3 Back;
	static const ivec3 AxisX;
	static const ivec3 AxisY;
	static const ivec3 AxisZ;
};

struct ivec4
{
	union {
		struct {
			int x, y, z, w;
		};
		struct {
			ivec3 xyz;
			int w;
		};
		struct {
			ivec2 xy;
			vec2 zw;
		};
		struct {
			int x;
			ivec3 yzw;
		};
		struct {
			int x;
			ivec2 yz;
			int w;
		};
	};


	ivec4();
	ivec4(int x, int y, int z, int w);
	ivec4(const ivec3& xyz, int w);
};


vec2 operator+(vec2 a, vec2 b);
vec2 operator-(vec2 a, vec2 b);
vec2 operator*(vec2 a, vec2 b);
vec2 operator/(vec2 a, vec2 b);

vec2 operator+(vec2 a, float b);
vec2 operator-(vec2 a, float b);
vec2 operator*(vec2 a, float b);
vec2 operator/(vec2 a, float b);

vec2 operator+(float a, vec2 b);
vec2 operator-(float a, vec2 b);
vec2 operator*(float a, vec2 b);
vec2 operator/(float a, vec2 b);

vec2 operator+(ivec2 a, vec2 b);
vec2 operator-(ivec2 a, vec2 b);
vec2 operator*(ivec2 a, vec2 b);
vec2 operator/(ivec2 a, vec2 b);


vec2& operator+=(vec2& a, const vec2& b);
vec2& operator-=(vec2& a, const vec2& b);

bool operator==(const vec2& a, const vec2& b);


vec3 operator+(vec3 a, vec3 b);
vec3 operator-(vec3 a, vec3 b);
vec3 operator*(vec3 a, vec3 b);
vec3 operator/(vec3 a, vec3 b);

vec3 operator+(vec3 a, float b);
vec3 operator-(vec3 a, float b);
vec3 operator*(vec3 a, float b);
vec3 operator/(vec3 a, float b);

vec3 operator+(vec3 a, int b);
vec3 operator-(vec3 a, int b);
vec3 operator*(vec3 a, int b);
vec3 operator/(vec3 a, int b);

vec3 operator+(float a, vec3 b);
vec3 operator-(float a, vec3 b);
vec3 operator*(float a, vec3 b);
vec3 operator/(float a, vec3 b);

bool operator==(const vec3& a, const vec3& b);

vec4 operator+(vec4 a, vec4 b);
vec4 operator-(vec4 a, vec4 b);
vec4 operator*(vec4 a, vec4 b);
vec4 operator/(vec4 a, vec4 b);

vec4 operator+(vec4 a, float b);
vec4 operator-(vec4 a, float b);
vec4 operator*(vec4 a, float b);
vec4 operator/(vec4 a, float b);

vec4 operator+(float a, vec4 b);
vec4 operator-(float a, vec4 b);
vec4 operator*(float a, vec4 b);
vec4 operator/(float a, vec4 b);

bool operator==(const vec4& a, const vec4& b);


ivec3 operator+(ivec3 a, ivec3 b);
ivec3 operator-(ivec3 a, ivec3 b);
ivec3 operator*(ivec3 a, ivec3 b);
ivec3 operator/(ivec3 a, ivec3 b);

ivec3 operator+(ivec3 a, int b);
ivec3 operator-(ivec3 a, int b);
ivec3 operator*(ivec3 a, int b);
ivec3 operator/(ivec3 a, int b);

vec3 operator+(ivec3 a, float b);
vec3 operator-(ivec3 a, float b);
vec3 operator*(ivec3 a, float b);
vec3 operator/(ivec3 a, float b);

bool operator==(const ivec3& a, const ivec3& b);
bool operator!=(const ivec3& a, const ivec3& b);

float dot(const vec3& a, const vec3& b);
float dot(const vec4& a, const vec4& b);
vec3 cross(const vec3& a, const vec3& b);

ivec3 abs(const ivec3& v);
vec3 abs(const vec3& v);
vec2 abs(const vec2& v);

vec2 min(const vec2& a, const vec2& b);
vec2 max(const vec2& a, const vec2& b);
vec3 min(const vec3& a, const vec3& b);
vec3 max(const vec3& a, const vec3& b);

ivec2 sign(const vec2& v);
ivec3 sign(const ivec3& v);

vec2 mix(const vec2& a, const vec2& b, float t);
vec3 mix(const vec3& a, const vec3& b, float t);
template<typename T>
T mix(const T& a, const T& b, float t)
{
	return t * b + (1 - t) * a;
}

vec3 floor(const vec3& v);
