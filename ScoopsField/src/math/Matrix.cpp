#include "Matrix.h"

#include "Math.h"

#include <math.h>
#include <string.h>


Matrix::Matrix() :
	m00(1.0f), m10(0.0f), m20(0.0f), m30(0.0f),
	m01(0.0f), m11(1.0f), m21(0.0f), m31(0.0f),
	m02(0.0f), m12(0.0f), m22(1.0f), m32(0.0f),
	m03(0.0f), m13(0.0f), m23(0.0f), m33(1.0f)
{
}

Matrix::Matrix(float diagonal) :
	m00(diagonal), m10(0.0f), m20(0.0f), m30(0.0f),
	m01(0.0f), m11(diagonal), m21(0.0f), m31(0.0f),
	m02(0.0f), m12(0.0f), m22(diagonal), m32(0.0f),
	m03(0.0f), m13(0.0f), m23(0.0f), m33(diagonal)
{
}

Matrix::Matrix(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3)
{
	columns[0] = col0;
	columns[1] = col1;
	columns[2] = col2;
	columns[3] = col3;
}

Matrix::Matrix(const float elements[16])
{
	memcpy(this->elements, elements, sizeof(this->elements));
}

vec3 Matrix::translation() const
{
	return this->columns[3].xyz;
}

vec3 Matrix::scale() const
{
	float x = sqrtf(this->m00 * this->m00 + this->m01 * this->m01 + this->m02 * this->m02);
	float y = sqrtf(this->m10 * this->m10 + this->m11 * this->m11 + this->m12 * this->m12);
	float z = sqrtf(this->m20 * this->m20 + this->m21 * this->m21 + this->m22 * this->m22);
	return vec3(x, y, z);
}

static Quaternion ExtractRotation(Matrix matrix, const vec3& scale)
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

	return Quaternion(x, y, z, width).normalized();
}

Quaternion Matrix::rotation() const
{
	return ExtractRotation(*this, scale());
}

void Matrix::decompose(vec3& translation, Quaternion& rotation, vec3& scale) const
{
	translation = this->translation();
	scale = this->scale();
	rotation = ExtractRotation(*this, scale);
}

float Matrix::determinant() const
{
	const Matrix& m = *this;
	return
		m.elements[3] * m.elements[6] * m.elements[9] * m.elements[12] - m.elements[2] * m.elements[7] * m.elements[9] * m.elements[12] - m.elements[3] * m.elements[5] * m.elements[10] * m.elements[12] + m.elements[1] * m.elements[7] * m.elements[10] * m.elements[12] +
		m.elements[2] * m.elements[5] * m.elements[11] * m.elements[12] - m.elements[1] * m.elements[6] * m.elements[11] * m.elements[12] - m.elements[3] * m.elements[6] * m.elements[8] * m.elements[13] + m.elements[2] * m.elements[7] * m.elements[8] * m.elements[13] +
		m.elements[3] * m.elements[4] * m.elements[10] * m.elements[13] - m.elements[0] * m.elements[7] * m.elements[10] * m.elements[13] - m.elements[2] * m.elements[4] * m.elements[11] * m.elements[13] + m.elements[0] * m.elements[6] * m.elements[11] * m.elements[13] +
		m.elements[3] * m.elements[5] * m.elements[8] * m.elements[14] - m.elements[1] * m.elements[7] * m.elements[8] * m.elements[14] - m.elements[3] * m.elements[4] * m.elements[9] * m.elements[14] + m.elements[0] * m.elements[7] * m.elements[9] * m.elements[14] +
		m.elements[1] * m.elements[4] * m.elements[11] * m.elements[14] - m.elements[0] * m.elements[5] * m.elements[11] * m.elements[14] - m.elements[2] * m.elements[5] * m.elements[8] * m.elements[15] + m.elements[1] * m.elements[6] * m.elements[8] * m.elements[15] +
		m.elements[2] * m.elements[4] * m.elements[9] * m.elements[15] - m.elements[0] * m.elements[6] * m.elements[9] * m.elements[15] - m.elements[1] * m.elements[4] * m.elements[10] * m.elements[15] + m.elements[0] * m.elements[5] * m.elements[10] * m.elements[15];
}

Matrix Matrix::inverted() const
{
	const Matrix& m = *this;
	Matrix result;

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

vec4& Matrix::operator[](int column)
{
	return columns[column];
}

const vec4& Matrix::operator[](int column) const
{
	return columns[column];
}

Matrix Matrix::Translate(const vec4& v)
{
	Matrix matrix = Identity;
	matrix.m30 = v.x;
	matrix.m31 = v.y;
	matrix.m32 = v.z;
	matrix.m33 = v.w;
	return matrix;
}

Matrix Matrix::Translate(float x, float y, float z, float w)
{
	Matrix matrix = Identity;
	matrix.m30 = x;
	matrix.m31 = y;
	matrix.m32 = z;
	matrix.m33 = w;
	return matrix;
}

Matrix Matrix::Translate(const vec3& v)
{
	Matrix matrix = Identity;
	matrix.m30 = v.x;
	matrix.m31 = v.y;
	matrix.m32 = v.z;
	return matrix;
}

Matrix Matrix::Translate(float x, float y, float z)
{
	Matrix matrix = Identity;
	matrix.m30 = x;
	matrix.m31 = y;
	matrix.m32 = z;
	return matrix;
}

Matrix Matrix::Rotate(const Quaternion& q)
{
	Matrix matrix = Identity;

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

Matrix Matrix::Scale(const vec3& v)
{
	Matrix matrix = Identity;
	matrix.m00 = v.x;
	matrix.m11 = v.y;
	matrix.m22 = v.z;
	matrix.m33 = 1.0f;
	return matrix;
}

Matrix Matrix::Transform(const vec3& position, const Quaternion& rotation, const vec3& scale)
{
	return Matrix::Translate(position) * Matrix::Rotate(rotation) * Matrix::Scale(scale);
}

Matrix Matrix::Perspective(float fovy, float aspect, float near, float far)
{
	// TODO homogenous depth check

	Matrix matrix = Identity;

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

Matrix Matrix::Orthographic(float left, float right, float bottom, float top, float near, float far)
{
	Matrix matrix = Identity;

	matrix[0][0] = 2.0f / (right - left);
	matrix[1][1] = 2.0f / (top - bottom);
	matrix[2][2] = -1.0f / (far - near);

	matrix[3][0] = -(right + left) / (right - left);
	matrix[3][1] = -(top + bottom) / (top - bottom);
	matrix[3][2] = (far + near) / (far - near) + 0.5f;

	return matrix;
}

const Matrix Matrix::Identity = Matrix(1.0f);

vec4 mul(const Matrix& left, const vec4& right)
{
	Matrix rightMatrix = Matrix::Translate(right);
	return (left * rightMatrix)[3];
}

Matrix operator*(const Matrix& left, const Matrix& right)
{
	Matrix result = {};

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
}

vec4 operator*(const Matrix& a, const vec4& b)
{
	vec4 result;

	result.x = a.m00 * b.x + a.m10 * b.y + a.m20 * b.z + a.m30 * b.w;
	result.y = a.m01 * b.x + a.m11 * b.y + a.m21 * b.z + a.m31 * b.w;
	result.z = a.m02 * b.x + a.m12 * b.y + a.m22 * b.z + a.m32 * b.w;
	result.w = a.m03 * b.x + a.m13 * b.y + a.m23 * b.z + a.m33 * b.w;

	return result;
}

vec3 operator*(const Matrix& a, const vec3& b)
{
	vec3 result;

	result.x = a.m00 * b.x + a.m10 * b.y + a.m20 * b.z + a.m30;
	result.y = a.m01 * b.x + a.m11 * b.y + a.m21 * b.z + a.m31;
	result.z = a.m02 * b.x + a.m12 * b.y + a.m22 * b.z + a.m32;

	return result;
}

bool operator==(const Matrix& a, const Matrix& b)
{
	return memcmp(a.elements, b.elements, 16 * sizeof(float)) == 0;
}

bool operator!=(const Matrix& a, const Matrix& b)
{
	return memcmp(a.elements, b.elements, 16 * sizeof(float)) != 0;
}

void GetFrustumPlanes(const Matrix& pv, vec4 planes[6])
{
	Matrix matrix = pv;

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
