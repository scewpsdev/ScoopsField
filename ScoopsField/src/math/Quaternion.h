#pragma once

#include "Vector.h"


struct quat
{
	float x, y, z, w;


	quat();
	quat(float x, float y, float z, float w);
	quat(const vec3& xyz, float w);

	void normalize();

	float length() const;
	quat normalized() const;
	quat conjugated() const;
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


	static quat FromAxisAngle(vec3 axis, float angle);
	static quat LookAt(const vec3& eye, const vec3& at, const vec3& up);
	static quat FromEulers(vec3 eulers);

	static const quat Identity;
};


quat operator*(const quat& a, const quat& b);
quat operator*(const quat& a, const float& b);
quat operator*(const float& a, const quat& b);

quat operator+(const quat& a, const quat& b);

vec3 operator*(const quat& a, const vec3& b);

bool operator==(const quat& a, const quat& b);

quat slerp(const quat& left, quat right, float t);
