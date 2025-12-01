#pragma once

#include "Vector.h"


struct Quaternion
{
	float x, y, z, w;


	Quaternion();
	Quaternion(float x, float y, float z, float w);
	Quaternion(const vec3& xyz, float w);

	void normalize();

	float length() const;
	Quaternion normalized() const;
	Quaternion conjugated() const;
	vec4 toAxisAngle() const;
	vec3 eulers() const;
	float getAngle() const;
	vec3 getAxis() const;

	vec3 forward() const;
	vec3 back() const;
	vec3 left() const;
	vec3 right() const;
	vec3 down() const;
	vec3 up() const;


	static Quaternion FromAxisAngle(vec3 axis, float angle);
	static Quaternion LookAt(const vec3& eye, const vec3& at, const vec3& up);
	static Quaternion FromEulers(vec3 eulers);

	static const Quaternion Identity;
};


Quaternion operator*(const Quaternion& a, const Quaternion& b);
Quaternion operator*(const Quaternion& a, const float& b);
Quaternion operator*(const float& a, const Quaternion& b);

Quaternion operator+(const Quaternion& a, const Quaternion& b);

vec3 operator*(const Quaternion& a, const vec3& b);

bool operator==(const Quaternion& a, const Quaternion& b);

Quaternion slerp(const Quaternion& left, Quaternion right, float t);
