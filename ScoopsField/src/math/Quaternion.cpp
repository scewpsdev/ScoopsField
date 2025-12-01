#include "Quaternion.h"

#include "Math.h"

#include <math.h>


Quaternion::Quaternion()
	: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Quaternion::Quaternion(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

Quaternion::Quaternion(const vec3& xyz, float w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{
}

void Quaternion::normalize()
{
	if (fabsf(x * x + y * y + z * z + w * w - 1.0f) < 0.001f)
		return;
	if (x * x + y * y + z * z + w * w == 0.0f)
		return;

	float l = 1.0f / this->length();
	x *= l;
	y *= l;
	z *= l;
	w *= l;
}

float Quaternion::length() const
{
	return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);
}

Quaternion Quaternion::normalized() const
{
	if (fabsf(x * x + y * y + z * z + w * w - 1.0f) < 0.00000001f)
		return *this;
	if (x * x + y * y + z * z + w * w == 0.0f)
		return *this;

	float l = 1.0f / this->length();
	float x = this->x * l;
	float y = this->y * l;
	float z = this->z * l;
	float w = this->w * l;
	return Quaternion(x, y, z, w);
}

Quaternion Quaternion::conjugated() const
{
	return Quaternion(-x, -y, -z, w);
}

vec4 Quaternion::toAxisAngle() const
{
	float angle = 2.0f * acosf(w);
	float s = 1.0f / sqrtf(1.0f - w * w);
	if (isinf(s))
	{
		return vec4(1.0f, 0.0f, 0.0f, 0.0f);
	}
	else if (s < 0.001f)
	{
		return vec4(1.0f, 0.0f, 0.0f, angle);
	}
	else
	{
		return vec4(x * s, y * s, z * s, angle);
	}
}

vec3 Quaternion::eulers() const
{
	float ry, rx, rz;
	float test = x * w + y * z;
	if (test > 0.499f)
	{ // singularity at north pole
		ry = 2 * atan2f(y, w);
		rx = PI / 2;
		rz = 0;
		return vec3(rx, ry, rz);
	}
	if (test < -0.499)
	{ // singularity at south pole
		ry = -2 * atan2f(y, w);
		rx = -PI / 2;
		rz = 0;
		return vec3(rx, ry, rz);
	}
	float sqx = x * x;
	float sqy = y * y;
	float sqz = z * z;
	ry = atan2f(2 * y * w - 2 * x * z, 1 - 2 * sqy - 2 * sqx);
	//rx = MathF.Atan2(2 * x * w - 2 * y * z, 1 - 2 * sqx - 2 * sqz);
	rx = asinf(2 * test);
	rz = atan2f(2 * z * w - 2 * x * y, 1 - 2 * sqz - 2 * sqx);
	return vec3(rx, ry, rz);
}

float Quaternion::getAngle() const
{
	return 2.0f * acosf(w);
}

vec3 Quaternion::getAxis() const
{
	if (w < 1)
	{
		float s = 1.0f / sqrtf(1.0f - w * w);
		return vec3(x * s, y * s, z * s);
	}
	return vec3(1, 0, 0);
}

vec3 Quaternion::forward() const
{
	return *this * vec3::Forward;
}

vec3 Quaternion::back() const
{
	return *this * vec3::Back;
}

vec3 Quaternion::left() const
{
	return *this * vec3::Left;
}

vec3 Quaternion::right() const
{
	return *this * vec3::Right;
}

vec3 Quaternion::down() const
{
	return *this * vec3::Down;
}

vec3 Quaternion::up() const
{
	return *this * vec3::Up;
}

Quaternion Quaternion::FromAxisAngle(vec3 axis, float angle)
{
	float half = angle * 0.5f;
	float s = sinf(half);
	float x = axis.x * s;
	float y = axis.y * s;
	float z = axis.z * s;
	float w = cosf(half);

	return Quaternion(x, y, z, w);
}

Quaternion Quaternion::LookAt(const vec3& eye, const vec3& at, const vec3& up)
{
	vec3 forward = (at - eye).normalized();
	vec3 right = cross(forward, up).normalized();
	vec3 up2 = cross(right, forward);

	// Build rotation matrix
	float m00 = right.x; float m01 = right.y; float m02 = right.z;
	float m10 = up2.x; float m11 = up2.y; float m12 = up2.z;
	float m20 = -forward.x; float m21 = -forward.y; float m22 = -forward.z;

	float trace = m00 + m11 + m22;

	Quaternion q;
	if (trace > 0.0f)
	{
		float s = sqrtf(trace + 1.0f) * 2.0f;
		q.w = 0.25f * s;
		q.x = (m12 - m21) / s;
		q.y = (m20 - m02) / s;
		q.z = (m01 - m10) / s;
	}
	else if ((m00 > m11) && (m00 > m22))
	{
		float s = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
		q.w = (m12 - m21) / s;
		q.x = 0.25f * s;
		q.y = (m10 + m01) / s;
		q.z = (m20 + m02) / s;
	}
	else if (m11 > m22)
	{
		float s = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
		q.w = (m20 - m02) / s;
		q.x = (m10 + m01) / s;
		q.y = 0.25f * s;
		q.z = (m21 + m12) / s;
	}
	else
	{
		float s = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
		q.w = (m01 - m10) / s;
		q.x = (m20 + m02) / s;
		q.y = (m21 + m12) / s;
		q.z = 0.25f * s;
	}

	return q.normalized();

	/*
	float d = dot(vec3::Forward, forward);

	if (fabsf(d - -1.0f) < 0.000001f)
		return Quaternion(0.0f, 1.0f, 0.0f, PI);
	if (fabsf(d - 1.0f) < 0.000001f)
		return Quaternion::Identity;

	float angle = acosf(d);
	vec3 axis = cross(vec3::Forward, forward).normalized();
	Quaternion q = FromAxisAngle(axis, angle).normalized();

	return q;
	*/
}

Quaternion Quaternion::FromEulers(vec3 eulers)
{
	float c1 = cosf(eulers.y / 2.0f);
	float s1 = sinf(eulers.y / 2.0f);
	float c2 = cosf(eulers.z / 2.0f);
	float s2 = sinf(eulers.z / 2.0f);
	float c3 = cosf(eulers.x / 2.0f);
	float s3 = sinf(eulers.x / 2.0f);
	float c1c2 = c1 * c2;
	float s1s2 = s1 * s2;
	float x = c1c2 * s3 + s1s2 * c3;
	float y = s1 * c2 * c3 + c1 * s2 * s3;
	float z = c1 * s2 * c3 - s1 * c2 * s3;
	float w = c1c2 * c3 - s1s2 * s3;

	return Quaternion(x, y, z, w);
}

const Quaternion Quaternion::Identity = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion operator*(const Quaternion& a, const Quaternion& b)
{
	float w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	float x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	float y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	float z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	return Quaternion(x, y, z, w);
}

Quaternion operator*(const Quaternion& a, const float& b)
{
	return Quaternion(a.x * b, a.y * b, a.z * b, a.w * b);
}

Quaternion operator*(const float& a, const Quaternion& b)
{
	return Quaternion(a * b.x, a * b.y, a * b.z, a * b.w);
}

Quaternion operator+(const Quaternion& a, const Quaternion& b)
{
	return Quaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

vec3 operator*(const Quaternion& a, const vec3& b)
{
	Quaternion a1 = a.normalized();
	Quaternion a2 = a1.conjugated();

	Quaternion q;
	q.w = -a1.x * b.x - a1.y * b.y - a1.z * b.z;
	q.x = +a1.w * b.x + a1.y * b.z - a1.z * b.y;
	q.y = +a1.w * b.y - a1.x * b.z + a1.z * b.x;
	q.z = +a1.w * b.z + a1.x * b.y - a1.y * b.x;

	Quaternion q2;
	q2.w = q.w * a2.w - q.x * a2.x - q.y * a2.y - q.z * a2.z;
	q2.x = q.w * a2.x + q.x * a2.w + q.y * a2.z - q.z * a2.y;
	q2.y = q.w * a2.y - q.x * a2.z + q.y * a2.w + q.z * a2.x;
	q2.z = q.w * a2.z + q.x * a2.y - q.y * a2.x + q.z * a2.w;

	return { q2.x, q2.y, q2.z };
}

bool operator==(const Quaternion& a, const Quaternion& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

Quaternion slerp(const Quaternion& left, Quaternion right, float t)
{
	float cosHalfTheta = left.w * right.w + left.x * right.x + left.y * right.y + left.z * right.z;
	if (fabsf(cosHalfTheta) >= 1.0f)
		return left;
	if (cosHalfTheta < 0.0f)
	{
		right = right * -1.0f;
		cosHalfTheta = -cosHalfTheta;
	}

	float halfTheta = acosf(cosHalfTheta);
	float sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
	if (fabsf(sinHalfTheta) < 0.001f)
		return 0.5f * left + 0.5f * right;

	float ratioA = sinf((1.0f - t) * halfTheta) / sinHalfTheta;
	float ratioB = sinf(t * halfTheta) / sinHalfTheta;

	float w = left.w * ratioA + right.w * ratioB;
	float x = left.x * ratioA + right.x * ratioB;
	float y = left.y * ratioA + right.y * ratioB;
	float z = left.z * ratioA + right.z * ratioB;

	return Quaternion(x, y, z, w);
}
