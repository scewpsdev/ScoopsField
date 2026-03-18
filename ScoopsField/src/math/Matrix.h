#pragma once

#include "Vector.h"
#include "Quaternion.h"


struct mat4
{
	union
	{
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};
		float matrix[4][4];
		float elements[16];
		vec4 columns[4];
	};

	mat4();
	mat4(float diagonal);
	mat4(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3);
	mat4(const float elements[16]);

	vec3 translation() const;
	vec3 scale() const;
	quat rotation() const;

	void decompose(vec3& translation, quat& rotation, vec3& scale) const;

	float determinant() const;
	mat4 inverted() const;

	vec4& operator[](int column);
	const vec4& operator[](int column) const;


	static mat4 Translate(const vec4& v);
	static mat4 Translate(float x, float y, float z, float w);
	static mat4 Translate(const vec3& v);
	static mat4 Translate(float x, float y, float z);
	static mat4 Rotate(const quat& q);
	static mat4 Rotate(const vec3& axis, float angle);
	static mat4 Scale(const vec3& v);
	static mat4 Transform(const vec3& position, const quat& rotation, const vec3& scale);

	static mat4 Perspective(float fovy, float aspect, float near);
	static mat4 Orthographic(float left, float right, float bottom, float top, float near, float far);

	static const mat4 Identity;
};


vec4 mul(const mat4& left, const vec4& right);

mat4 operator*(const mat4& left, const mat4& right);
vec4 operator*(const mat4& left, const vec4& right);
vec3 operator*(const mat4& left, const vec3& right);

bool operator==(const mat4& a, const mat4& b);
bool operator!=(const mat4& a, const mat4& b);

void GetFrustumPlanes(const mat4& pv, vec4 planes[6]);
