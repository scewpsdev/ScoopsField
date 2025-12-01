#pragma once

#include "Vector.h"
#include "Quaternion.h"


struct Matrix
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

	Matrix();
	Matrix(float diagonal);
	Matrix(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3);
	Matrix(const float elements[16]);

	vec3 translation() const;
	vec3 scale() const;
	Quaternion rotation() const;

	void decompose(vec3& translation, Quaternion& rotation, vec3& scale) const;

	float determinant() const;
	Matrix inverted() const;

	vec4& operator[](int column);
	const vec4& operator[](int column) const;


	static Matrix Translate(const vec4& v);
	static Matrix Translate(float x, float y, float z, float w);
	static Matrix Translate(const vec3& v);
	static Matrix Translate(float x, float y, float z);
	static Matrix Rotate(const Quaternion& q);
	static Matrix Scale(const vec3& v);
	static Matrix Transform(const vec3& position, const Quaternion& rotation, const vec3& scale);

	static Matrix Perspective(float fovy, float aspect, float near, float far);
	static Matrix Orthographic(float left, float right, float bottom, float top, float near, float far);

	static const Matrix Identity;
};


vec4 mul(const Matrix& left, const vec4& right);

Matrix operator*(const Matrix& left, const Matrix& right);
vec4 operator*(const Matrix& left, const vec4& right);
vec3 operator*(const Matrix& left, const vec3& right);

bool operator==(const Matrix& a, const Matrix& b);
bool operator!=(const Matrix& a, const Matrix& b);

void GetFrustumPlanes(const Matrix& pv, vec4 planes[6]);
