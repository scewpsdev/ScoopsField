#include "Matrix.h"

#include "Math.h"

#include <SDL3/SDL.h>

#include <immintrin.h>


mat4::mat4() :
	m00(1.0f), m10(0.0f), m20(0.0f), m30(0.0f),
	m01(0.0f), m11(1.0f), m21(0.0f), m31(0.0f),
	m02(0.0f), m12(0.0f), m22(1.0f), m32(0.0f),
	m03(0.0f), m13(0.0f), m23(0.0f), m33(1.0f)
{
}

mat4::mat4(float diagonal) :
	m00(diagonal), m10(0.0f), m20(0.0f), m30(0.0f),
	m01(0.0f), m11(diagonal), m21(0.0f), m31(0.0f),
	m02(0.0f), m12(0.0f), m22(diagonal), m32(0.0f),
	m03(0.0f), m13(0.0f), m23(0.0f), m33(diagonal)
{
}

mat4::mat4(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3)
{
	columns[0] = col0;
	columns[1] = col1;
	columns[2] = col2;
	columns[3] = col3;
}

mat4::mat4(const float elements[16])
{
	memcpy(this->elements, elements, sizeof(this->elements));
}

vec3 mat4::translation() const
{
	return this->columns[3].xyz;
}

vec3 mat4::scale() const
{
	float x = sqrtf(this->m00 * this->m00 + this->m01 * this->m01 + this->m02 * this->m02);
	float y = sqrtf(this->m10 * this->m10 + this->m11 * this->m11 + this->m12 * this->m12);
	float z = sqrtf(this->m20 * this->m20 + this->m21 * this->m21 + this->m22 * this->m22);
	return vec3(x, y, z);
}

static quat ExtractRotation(mat4 matrix, const vec3& scale)
{
	float sx = 1.0f / scale.x;
	float sy = 1.0f / scale.y;
	float sz = 1.0f / scale.z;

	matrix.columns[0].xyz *= sx;
	matrix.columns[1].xyz *= sy;
	matrix.columns[2].xyz *= sz;

	float width = sqrtf(fmaxf(0.0f, 1.0f + matrix.m00 + matrix.m11 + matrix.m22)) / 2.0f;
	float x = sqrtf(fmaxf(0.0f, 1.0f + matrix.m00 - matrix.m11 - matrix.m22)) / 2.0f;
	float y = sqrtf(fmaxf(0.0f, 1.0f - matrix.m00 + matrix.m11 - matrix.m22)) / 2.0f;
	float z = sqrtf(fmaxf(0.0f, 1.0f - matrix.m00 - matrix.m11 + matrix.m22)) / 2.0f;

	x = copysignf(x, matrix.m12 - matrix.m21);
	y = copysignf(y, matrix.m20 - matrix.m02);
	z = copysignf(z, matrix.m01 - matrix.m10);

	return quat(x, y, z, width).normalized();
}

quat mat4::rotation() const
{
	return ExtractRotation(*this, scale());
}

void mat4::decompose(vec3& translation, quat& rotation, vec3& scale) const
{
	translation = this->translation();
	scale = this->scale();
	rotation = ExtractRotation(*this, scale);
}

float mat4::determinant() const
{
	const mat4& m = *this;
	return
		m.elements[3] * m.elements[6] * m.elements[9] * m.elements[12] - m.elements[2] * m.elements[7] * m.elements[9] * m.elements[12] - m.elements[3] * m.elements[5] * m.elements[10] * m.elements[12] + m.elements[1] * m.elements[7] * m.elements[10] * m.elements[12] +
		m.elements[2] * m.elements[5] * m.elements[11] * m.elements[12] - m.elements[1] * m.elements[6] * m.elements[11] * m.elements[12] - m.elements[3] * m.elements[6] * m.elements[8] * m.elements[13] + m.elements[2] * m.elements[7] * m.elements[8] * m.elements[13] +
		m.elements[3] * m.elements[4] * m.elements[10] * m.elements[13] - m.elements[0] * m.elements[7] * m.elements[10] * m.elements[13] - m.elements[2] * m.elements[4] * m.elements[11] * m.elements[13] + m.elements[0] * m.elements[6] * m.elements[11] * m.elements[13] +
		m.elements[3] * m.elements[5] * m.elements[8] * m.elements[14] - m.elements[1] * m.elements[7] * m.elements[8] * m.elements[14] - m.elements[3] * m.elements[4] * m.elements[9] * m.elements[14] + m.elements[0] * m.elements[7] * m.elements[9] * m.elements[14] +
		m.elements[1] * m.elements[4] * m.elements[11] * m.elements[14] - m.elements[0] * m.elements[5] * m.elements[11] * m.elements[14] - m.elements[2] * m.elements[5] * m.elements[8] * m.elements[15] + m.elements[1] * m.elements[6] * m.elements[8] * m.elements[15] +
		m.elements[2] * m.elements[4] * m.elements[9] * m.elements[15] - m.elements[0] * m.elements[6] * m.elements[9] * m.elements[15] - m.elements[1] * m.elements[4] * m.elements[10] * m.elements[15] + m.elements[0] * m.elements[5] * m.elements[10] * m.elements[15];
}

mat4 mat4::inverted() const
{
	const mat4& m = *this;
	mat4 result;

	float f = 1.0f / determinant();
	result.elements[0] = (m.elements[6] * m.elements[11] * m.elements[13] - m.elements[7] * m.elements[10] * m.elements[13] + m.elements[7] * m.elements[9] * m.elements[14] - m.elements[5] * m.elements[11] * m.elements[14] - m.elements[6] * m.elements[9] * m.elements[15] + m.elements[5] * m.elements[10] * m.elements[15]) * f;
	result.elements[1] = (m.elements[3] * m.elements[10] * m.elements[13] - m.elements[2] * m.elements[11] * m.elements[13] - m.elements[3] * m.elements[9] * m.elements[14] + m.elements[1] * m.elements[11] * m.elements[14] + m.elements[2] * m.elements[9] * m.elements[15] - m.elements[1] * m.elements[10] * m.elements[15]) * f;
	result.elements[2] = (m.elements[2] * m.elements[7] * m.elements[13] - m.elements[3] * m.elements[6] * m.elements[13] + m.elements[3] * m.elements[5] * m.elements[14] - m.elements[1] * m.elements[7] * m.elements[14] - m.elements[2] * m.elements[5] * m.elements[15] + m.elements[1] * m.elements[6] * m.elements[15]) * f;
	result.elements[3] = (m.elements[3] * m.elements[6] * m.elements[9] - m.elements[2] * m.elements[7] * m.elements[9] - m.elements[3] * m.elements[5] * m.elements[10] + m.elements[1] * m.elements[7] * m.elements[10] + m.elements[2] * m.elements[5] * m.elements[11] - m.elements[1] * m.elements[6] * m.elements[11]) * f;
	result.elements[4] = (m.elements[7] * m.elements[10] * m.elements[12] - m.elements[6] * m.elements[11] * m.elements[12] - m.elements[7] * m.elements[8] * m.elements[14] + m.elements[4] * m.elements[11] * m.elements[14] + m.elements[6] * m.elements[8] * m.elements[15] - m.elements[4] * m.elements[10] * m.elements[15]) * f;
	result.elements[5] = (m.elements[2] * m.elements[11] * m.elements[12] - m.elements[3] * m.elements[10] * m.elements[12] + m.elements[3] * m.elements[8] * m.elements[14] - m.elements[0] * m.elements[11] * m.elements[14] - m.elements[2] * m.elements[8] * m.elements[15] + m.elements[0] * m.elements[10] * m.elements[15]) * f;
	result.elements[6] = (m.elements[3] * m.elements[6] * m.elements[12] - m.elements[2] * m.elements[7] * m.elements[12] - m.elements[3] * m.elements[4] * m.elements[14] + m.elements[0] * m.elements[7] * m.elements[14] + m.elements[2] * m.elements[4] * m.elements[15] - m.elements[0] * m.elements[6] * m.elements[15]) * f;
	result.elements[7] = (m.elements[2] * m.elements[7] * m.elements[8] - m.elements[3] * m.elements[6] * m.elements[8] + m.elements[3] * m.elements[4] * m.elements[10] - m.elements[0] * m.elements[7] * m.elements[10] - m.elements[2] * m.elements[4] * m.elements[11] + m.elements[0] * m.elements[6] * m.elements[11]) * f;
	result.elements[8] = (m.elements[5] * m.elements[11] * m.elements[12] - m.elements[7] * m.elements[9] * m.elements[12] + m.elements[7] * m.elements[8] * m.elements[13] - m.elements[4] * m.elements[11] * m.elements[13] - m.elements[5] * m.elements[8] * m.elements[15] + m.elements[4] * m.elements[9] * m.elements[15]) * f;
	result.elements[9] = (m.elements[3] * m.elements[9] * m.elements[12] - m.elements[1] * m.elements[11] * m.elements[12] - m.elements[3] * m.elements[8] * m.elements[13] + m.elements[0] * m.elements[11] * m.elements[13] + m.elements[1] * m.elements[8] * m.elements[15] - m.elements[0] * m.elements[9] * m.elements[15]) * f;
	result.elements[10] = (m.elements[1] * m.elements[7] * m.elements[12] - m.elements[3] * m.elements[5] * m.elements[12] + m.elements[3] * m.elements[4] * m.elements[13] - m.elements[0] * m.elements[7] * m.elements[13] - m.elements[1] * m.elements[4] * m.elements[15] + m.elements[0] * m.elements[5] * m.elements[15]) * f;
	result.elements[11] = (m.elements[3] * m.elements[5] * m.elements[8] - m.elements[1] * m.elements[7] * m.elements[8] - m.elements[3] * m.elements[4] * m.elements[9] + m.elements[0] * m.elements[7] * m.elements[9] + m.elements[1] * m.elements[4] * m.elements[11] - m.elements[0] * m.elements[5] * m.elements[11]) * f;
	result.elements[12] = (m.elements[6] * m.elements[9] * m.elements[12] - m.elements[5] * m.elements[10] * m.elements[12] - m.elements[6] * m.elements[8] * m.elements[13] + m.elements[4] * m.elements[10] * m.elements[13] + m.elements[5] * m.elements[8] * m.elements[14] - m.elements[4] * m.elements[9] * m.elements[14]) * f;
	result.elements[13] = (m.elements[1] * m.elements[10] * m.elements[12] - m.elements[2] * m.elements[9] * m.elements[12] + m.elements[2] * m.elements[8] * m.elements[13] - m.elements[0] * m.elements[10] * m.elements[13] - m.elements[1] * m.elements[8] * m.elements[14] + m.elements[0] * m.elements[9] * m.elements[14]) * f;
	result.elements[14] = (m.elements[2] * m.elements[5] * m.elements[12] - m.elements[1] * m.elements[6] * m.elements[12] - m.elements[2] * m.elements[4] * m.elements[13] + m.elements[0] * m.elements[6] * m.elements[13] + m.elements[1] * m.elements[4] * m.elements[14] - m.elements[0] * m.elements[5] * m.elements[14]) * f;
	result.elements[15] = (m.elements[1] * m.elements[6] * m.elements[8] - m.elements[2] * m.elements[5] * m.elements[8] + m.elements[2] * m.elements[4] * m.elements[9] - m.elements[0] * m.elements[6] * m.elements[9] - m.elements[1] * m.elements[4] * m.elements[10] + m.elements[0] * m.elements[5] * m.elements[10]) * f;

	return result;
}

vec4& mat4::operator[](int column)
{
	return columns[column];
}

const vec4& mat4::operator[](int column) const
{
	return columns[column];
}

mat4 mat4::Translate(const vec4& v)
{
	mat4 matrix = Identity;
	matrix.m30 = v.x;
	matrix.m31 = v.y;
	matrix.m32 = v.z;
	matrix.m33 = v.w;
	return matrix;
}

mat4 mat4::Translate(float x, float y, float z, float w)
{
	mat4 matrix = Identity;
	matrix.m30 = x;
	matrix.m31 = y;
	matrix.m32 = z;
	matrix.m33 = w;
	return matrix;
}

mat4 mat4::Translate(const vec3& v)
{
	mat4 matrix = Identity;
	matrix.m30 = v.x;
	matrix.m31 = v.y;
	matrix.m32 = v.z;
	return matrix;
}

mat4 mat4::Translate(float x, float y, float z)
{
	mat4 matrix = Identity;
	matrix.m30 = x;
	matrix.m31 = y;
	matrix.m32 = z;
	return matrix;
}

mat4 mat4::Rotate(const quat& q)
{
	mat4 matrix = Identity;

	matrix.m00 = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
	matrix.m01 = 2.0f * q.x * q.y + 2.0f * q.z * q.w;
	matrix.m02 = 2.0f * q.x * q.z - 2.0f * q.y * q.w;
	matrix.m03 = 0.0f;

	matrix.m10 = 2.0f * q.x * q.y - 2.0f * q.z * q.w;
	matrix.m11 = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
	matrix.m12 = 2.0f * q.y * q.z + 2.0f * q.x * q.w;
	matrix.m13 = 0.0f;

	matrix.m20 = 2.0f * q.x * q.z + 2.0f * q.y * q.w;
	matrix.m21 = 2.0f * q.y * q.z - 2.0f * q.x * q.w;
	matrix.m22 = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
	matrix.m23 = 0.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = 0.0f;
	matrix.m33 = 1.0f;

	return matrix;
}

mat4 mat4::Rotate(const vec3& axis, float angle)
{
	mat4 matrix = {};

	float s = SDL_sinf(angle);
	float c = SDL_cosf(angle);
	float oc = 1 - c;

	matrix.m00 = oc * axis.x * axis.x + c;
	matrix.m01 = oc * axis.x * axis.y + axis.z * s;
	matrix.m02 = oc * axis.z * axis.x - axis.y * s;

	matrix.m10 = oc * axis.x * axis.y - axis.z * s;
	matrix.m11 = oc * axis.y * axis.y + c;
	matrix.m12 = oc * axis.y * axis.z + axis.x * s;

	matrix.m20 = oc * axis.z * axis.x + axis.y * s;
	matrix.m21 = oc * axis.y * axis.z - axis.x * s;
	matrix.m22 = oc * axis.z * axis.z + c;

	matrix.m33 = 1.0f;

	return matrix;
}

mat4 mat4::Scale(const vec3& v)
{
	mat4 matrix = Identity;
	matrix.m00 = v.x;
	matrix.m11 = v.y;
	matrix.m22 = v.z;
	matrix.m33 = 1.0f;
	return matrix;
}

mat4 mat4::Transform(const vec3& position, const quat& rotation, const vec3& scale)
{
	return mat4::Translate(position) * mat4::Rotate(rotation) * mat4::Scale(scale);
}

mat4 mat4::Perspective(float fovy, float aspect, float near, float far)
{
	// TODO homogenous depth check

	mat4 matrix = Identity;

	float y = 1.0f / tanf(0.5f * fovy);
	float x = y / aspect;
	float l = far - near;

	matrix.m00 = x;
	matrix.m11 = y;
	matrix.m22 = (far + near) / -l;
	matrix.m23 = -1.0f;
	matrix.m32 = -2.0f * near * far / l;
	matrix.m33 = 0.0f;

	return matrix;
}

mat4 mat4::Orthographic(float left, float right, float bottom, float top, float near, float far)
{
	mat4 matrix = Identity;

	matrix[0][0] = 2.0f / (right - left);
	matrix[1][1] = 2.0f / (top - bottom);
	matrix[2][2] = -1.0f / (far - near);

	matrix[3][0] = -(right + left) / (right - left);
	matrix[3][1] = -(top + bottom) / (top - bottom);
	matrix[3][2] = (far + near) / (far - near) + 0.5f;

	return matrix;
}

const mat4 mat4::Identity = mat4(1.0f);

vec4 mul(const mat4& left, const vec4& right)
{
	mat4 rightMatrix = mat4::Translate(right);
	return (left * rightMatrix)[3];
}

mat4 operator*(const mat4& left, const mat4& right)
{
	/*
	mat4 result = {};

	result.elements[0] = left.elements[0] * right.elements[0] + left.elements[4] * right.elements[1] + left.elements[8] * right.elements[2] + left.elements[12] * right.elements[3];
	result.elements[1] = left.elements[1] * right.elements[0] + left.elements[5] * right.elements[1] + left.elements[9] * right.elements[2] + left.elements[13] * right.elements[3];
	result.elements[2] = left.elements[2] * right.elements[0] + left.elements[6] * right.elements[1] + left.elements[10] * right.elements[2] + left.elements[14] * right.elements[3];
	result.elements[3] = left.elements[3] * right.elements[0] + left.elements[7] * right.elements[1] + left.elements[11] * right.elements[2] + left.elements[15] * right.elements[3];
	result.elements[4] = left.elements[0] * right.elements[4] + left.elements[4] * right.elements[5] + left.elements[8] * right.elements[6] + left.elements[12] * right.elements[7];
	result.elements[5] = left.elements[1] * right.elements[4] + left.elements[5] * right.elements[5] + left.elements[9] * right.elements[6] + left.elements[13] * right.elements[7];
	result.elements[6] = left.elements[2] * right.elements[4] + left.elements[6] * right.elements[5] + left.elements[10] * right.elements[6] + left.elements[14] * right.elements[7];
	result.elements[7] = left.elements[3] * right.elements[4] + left.elements[7] * right.elements[5] + left.elements[11] * right.elements[6] + left.elements[15] * right.elements[7];
	result.elements[8] = left.elements[0] * right.elements[8] + left.elements[4] * right.elements[9] + left.elements[8] * right.elements[10] + left.elements[12] * right.elements[11];
	result.elements[9] = left.elements[1] * right.elements[8] + left.elements[5] * right.elements[9] + left.elements[9] * right.elements[10] + left.elements[13] * right.elements[11];
	result.elements[10] = left.elements[2] * right.elements[8] + left.elements[6] * right.elements[9] + left.elements[10] * right.elements[10] + left.elements[14] * right.elements[11];
	result.elements[11] = left.elements[3] * right.elements[8] + left.elements[7] * right.elements[9] + left.elements[11] * right.elements[10] + left.elements[15] * right.elements[11];
	result.elements[12] = left.elements[0] * right.elements[12] + left.elements[4] * right.elements[13] + left.elements[8] * right.elements[14] + left.elements[12] * right.elements[15];
	result.elements[13] = left.elements[1] * right.elements[12] + left.elements[5] * right.elements[13] + left.elements[9] * right.elements[14] + left.elements[13] * right.elements[15];
	result.elements[14] = left.elements[2] * right.elements[12] + left.elements[6] * right.elements[13] + left.elements[10] * right.elements[14] + left.elements[14] * right.elements[15];
	result.elements[15] = left.elements[3] * right.elements[12] + left.elements[7] * right.elements[13] + left.elements[11] * right.elements[14] + left.elements[15] * right.elements[15];

	return result;
	*/

	__m128 columns[4];

	__m128* lcolumns = (__m128*)left.columns;
	__m128* rcolumns = (__m128*)right.columns;

	__m128 a0 = lcolumns[0];
	__m128 a1 = lcolumns[1];
	__m128 a2 = lcolumns[2];
	__m128 a3 = lcolumns[3];

	for (int i = 0; i < 4; i++)
	{
		__m128 b = rcolumns[i];

		__m128 x = _mm_shuffle_ps(b, b, 0x00); // xxxx
		__m128 y = _mm_shuffle_ps(b, b, 0x55); // yyyy
		__m128 z = _mm_shuffle_ps(b, b, 0xAA); // zzzz
		__m128 w = _mm_shuffle_ps(b, b, 0xFF); // wwww

		__m128 r =
			_mm_fmadd_ps(a0, x,
				_mm_fmadd_ps(a1, y,
					_mm_fmadd_ps(a2, z,
						_mm_mul_ps(a3, w))));

		columns[i] = r;
	}

	return *(mat4*)&columns;
}

vec4 operator*(const mat4& a, const vec4& b)
{
	vec4 result;

	result.x = a.m00 * b.x + a.m10 * b.y + a.m20 * b.z + a.m30 * b.w;
	result.y = a.m01 * b.x + a.m11 * b.y + a.m21 * b.z + a.m31 * b.w;
	result.z = a.m02 * b.x + a.m12 * b.y + a.m22 * b.z + a.m32 * b.w;
	result.w = a.m03 * b.x + a.m13 * b.y + a.m23 * b.z + a.m33 * b.w;

	return result;
}

vec3 operator*(const mat4& a, const vec3& b)
{
	vec3 result;

	result.x = a.m00 * b.x + a.m10 * b.y + a.m20 * b.z + a.m30;
	result.y = a.m01 * b.x + a.m11 * b.y + a.m21 * b.z + a.m31;
	result.z = a.m02 * b.x + a.m12 * b.y + a.m22 * b.z + a.m32;

	return result;
}

bool operator==(const mat4& a, const mat4& b)
{
	return memcmp(a.elements, b.elements, 16 * sizeof(float)) == 0;
}

bool operator!=(const mat4& a, const mat4& b)
{
	return memcmp(a.elements, b.elements, 16 * sizeof(float)) != 0;
}

void GetFrustumPlanes(const mat4& pv, vec4 planes[6])
{
	mat4 matrix = pv;

	// Left clipping plane
	planes[0].elements[0] = matrix.m03 + matrix.m00;
	planes[0].elements[1] = matrix.m13 + matrix.m10;
	planes[0].elements[2] = matrix.m23 + matrix.m20;
	planes[0].elements[3] = matrix.m33 + matrix.m30;
	// Right clipping plane
	planes[1].elements[0] = matrix.m03 - matrix.m00;
	planes[1].elements[1] = matrix.m13 - matrix.m10;
	planes[1].elements[2] = matrix.m23 - matrix.m20;
	planes[1].elements[3] = matrix.m33 - matrix.m30;
	// Bottom clipping plane
	planes[2].elements[0] = matrix.m03 + matrix.m01;
	planes[2].elements[1] = matrix.m13 + matrix.m11;
	planes[2].elements[2] = matrix.m23 + matrix.m21;
	planes[2].elements[3] = matrix.m33 + matrix.m31;
	// Top clipping plane
	planes[3].elements[0] = matrix.m03 - matrix.m01;
	planes[3].elements[1] = matrix.m13 - matrix.m11;
	planes[3].elements[2] = matrix.m23 - matrix.m21;
	planes[3].elements[3] = matrix.m33 - matrix.m31;
	// Near clipping plane
	planes[4].elements[0] = matrix.m03 + matrix.m02;
	planes[4].elements[1] = matrix.m13 + matrix.m12;
	planes[4].elements[2] = matrix.m23 + matrix.m22;
	planes[4].elements[3] = matrix.m33 + matrix.m32;
	// Far clipping plane
	planes[5].elements[0] = matrix.m03 - matrix.m02;
	planes[5].elements[1] = matrix.m13 - matrix.m12;
	planes[5].elements[2] = matrix.m23 - matrix.m22;
	planes[5].elements[3] = matrix.m33 - matrix.m32;

	for (int i = 0; i < 6; i++)
	{
		float l = 1.0f / planes[i].xyz.length();
		planes[i] *= l;
	}
}
