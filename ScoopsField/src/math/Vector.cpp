#include "Vector.h"

#include "Math.h"

#include <math.h>
#include <SDL3/SDL.h>


vec2::vec2()
	: x(0.0f), y(0.0f)
{
}

vec2::vec2(float f)
	: x(f), y(f)
{
}

vec2::vec2(float x, float y)
	: x(x), y(y)
{
}

float vec2::lengthSquared() const
{
	return x * x + y * y;
}

float vec2::length() const
{
	return sqrtf(x * x + y * y);
}

vec2 vec2::normalized() const
{
	return *this / length();
}

float vec2::angle() const
{
	return atan2f(y, x);
}

vec2 vec2::rotate(float angle) const
{
	float currentAngle = this->angle();
	float length = this->length();
	float newAngle = currentAngle + angle;
	return vec2(length * cosf(newAngle), length * sinf(newAngle));
}

vec2 vec2::operator-() const
{
	return vec2(-x, -y);
}

const vec2 vec2::Zero = vec2(0.0f, 0.0f);
const vec2 vec2::One = vec2(1.0f, 1.0f);
const vec2 vec2::Left = vec2(-1.0f, 0.0f);
const vec2 vec2::Right = vec2(1.0f, 0.0f);
const vec2 vec2::Down = vec2(0.0f, -1.0f);
const vec2 vec2::Up = vec2(0.0f, 1.0f);
const vec2 vec2::AxisX = vec2(1.0f, 0.0f);
const vec2 vec2::AxisY = vec2(0.0f, 1.0f);

vec3::vec3()
	: x(0.0f), y(0.0f), z(0.0f)
{
}

vec3::vec3(float xyz)
	: x(xyz), y(xyz), z(xyz)
{
}

vec3::vec3(float x, float y, float z)
	: x(x), y(y), z(z)
{
}

vec3::vec3(const vec2& xy, float z)
	: x(xy.x), y(xy.y), z(z)
{
}

vec3::vec3(float x, const vec2& yz)
	: x(x), y(yz.x), z(yz.y)
{
}

float vec3::lengthSquared() const
{
	return x * x + y * y + z * z;
}

float vec3::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

vec3 vec3::normalized() const
{
	if (fabsf(this->lengthSquared() - 1.0f) > 0.001f && this->lengthSquared() != 0.0f)
		return *this / length();
	else
		return *this;
}

float vec3::max() const
{
	return ::max(x, ::max(y, z));
}

vec3 vec3::operator-() const
{
	return vec3(-x, -y, -z);
}

vec3& vec3::operator+=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

vec3& vec3::operator-=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

vec3& vec3::operator*=(const vec3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

vec3& vec3::operator/=(const vec3& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

vec3::operator ivec3() const
{
	return ivec3((int)x, (int)y, (int)z);
}

void vec3::OrthoNormalize(const vec3& normal, vec3& tangent)
{
	vec3 norm = normal;
	vec3 tan = tangent.normalized();
	tangent = tan - (norm * dot(norm, tan));
	tangent = tangent.normalized();
}

const vec3 vec3::Zero = vec3(0.0f, 0.0f, 0.0f);
const vec3 vec3::One = vec3(1.0f, 1.0f, 1.0f);
const vec3 vec3::Left = vec3(-1.0f, 0.0f, 0.0f);
const vec3 vec3::Right = vec3(1.0f, 0.0f, 0.0f);
const vec3 vec3::Down = vec3(0.0f, -1.0f, 0.0f);
const vec3 vec3::Up = vec3(0.0f, 1.0f, 0.0f);
const vec3 vec3::Forward = vec3(0.0f, 0.0f, -1.0f);
const vec3 vec3::Back = vec3(0.0f, 0.0f, 1.0f);
const vec3 vec3::AxisX = vec3(1.0f, 0.0f, 0.0f);
const vec3 vec3::AxisY = vec3(0.0f, 1.0f, 0.0f);
const vec3 vec3::AxisZ = vec3(0.0f, 0.0f, 1.0f);

vec4::vec4()
	: x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
}

vec4::vec4(float f)
	: x(f), y(f), z(f), w(f)
{
}

vec4::vec4(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

vec4::vec4(const vec3& xyz, float w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{
}

vec4::vec4(float x, const vec3& yzw)
	: x(x), y(yzw.x), z(yzw.y), w(yzw.z)
{
}

float& vec4::operator[](int index)
{
	return elements[index];
}

const float& vec4::operator[](int index) const
{
	return elements[index];
}

vec4& vec4::operator+=(const vec4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

vec4& vec4::operator-=(const vec4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

vec4& vec4::operator*=(const vec4& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

vec4& vec4::operator/=(const vec4& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	return *this;
}

vec4& vec4::operator+=(float f)
{
	x += f;
	y += f;
	z += f;
	w += f;
	return *this;
}

vec4& vec4::operator-=(float f)
{
	x -= f;
	y -= f;
	z -= f;
	w -= f;
	return *this;
}

vec4& vec4::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

vec4& vec4::operator/=(float f)
{
	x /= f;
	y /= f;
	z /= f;
	w /= f;
	return *this;
}

const vec4 vec4::Zero = vec4(0.0f, 0.0f, 0.0f, 0.0f);
const vec4 vec4::One = vec4(1.0f, 1.0f, 1.0f, 1.0f);

ivec2::ivec2()
	: x(0), y(0)
{
}

ivec2::ivec2(int i)
	: x(i), y(i)
{
}

ivec2::ivec2(int x, int y)
	: x(x), y(y)
{
}

ivec3::ivec3()
	: x(0), y(0), z(0)
{
}

ivec3::ivec3(int i)
	: x(i), y(i), z(i)
{
}

ivec3::ivec3(int x, int y, int z)
	: x(x), y(y), z(z)
{
}

int& ivec3::operator[](int idx)
{
	return elements[idx];
}

ivec3 ivec3::operator-() const
{
	return ivec3(-x, -y, -z);
}

ivec3::operator vec3() const
{
	return vec3((float)x, (float)y, (float)z);
}

const ivec3 ivec3::Zero = ivec3(0, 0, 0);
const ivec3 ivec3::One = ivec3(1, 1, 1);
const ivec3 ivec3::Left = ivec3(-1, 0, 0);
const ivec3 ivec3::Right = ivec3(1, 0, 0);
const ivec3 ivec3::Down = ivec3(0, -1, 0);
const ivec3 ivec3::Up = ivec3(0, 1, 0);
const ivec3 ivec3::Forward = ivec3(0, 0, -1);
const ivec3 ivec3::Back = ivec3(0, 0, 1);
const ivec3 ivec3::AxisX = ivec3(1, 0, 0);
const ivec3 ivec3::AxisY = ivec3(0, 1, 0);
const ivec3 ivec3::AxisZ = ivec3(0, 0, 1);

ivec4::ivec4()
	: x(0), y(0), z(0), w(0)
{
}

ivec4::ivec4(int x, int y, int z, int w)
	: x(x), y(y), z(z), w(w)
{
}

ivec4::ivec4(const ivec3& xyz, int w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{
}

vec2 operator+(vec2 a, vec2 b)
{
	return vec2(a.x + b.x, a.y + b.y);
}

vec2 operator-(vec2  a, vec2  b)
{
	return vec2(a.x - b.x, a.y - b.y);
}

vec2 operator*(vec2 a, vec2 b)
{
	return vec2(a.x * b.x, a.y * b.y);
}

vec2 operator/(vec2 a, vec2 b)
{
	return vec2(a.x / b.x, a.y / b.y);
}

vec2 operator+(vec2 a, float b)
{
	return vec2(a.x + b, a.y + b);
}

vec2 operator-(vec2 a, float b)
{
	return vec2(a.x - b, a.y - b);
}

vec2 operator*(vec2 a, float b)
{
	return vec2(a.x * b, a.y * b);
}

vec2 operator/(vec2 a, float b)
{
	return vec2(a.x / b, a.y / b);
}

vec2 operator+(float a, vec2 b)
{
	return vec2(a + b.x, a + b.y);
}

vec2 operator-(float a, vec2 b)
{
	return vec2(a - b.x, a - b.y);
}

vec2 operator*(float a, vec2 b)
{
	return vec2(a * b.x, a * b.y);
}

vec2 operator/(float a, vec2 b)
{
	return vec2(a / b.x, a / b.y);
}

vec2 operator+(ivec2 a, vec2 b)
{
	return vec2(a.x + b.x, a.y + b.y);
}

vec2 operator-(ivec2 a, vec2 b)
{
	return vec2(a.x - b.x, a.y - b.y);
}

vec2 operator*(ivec2 a, vec2 b)
{
	return vec2(a.x * b.x, a.y * b.y);
}

vec2 operator/(ivec2 a, vec2 b)
{
	return vec2(a.x / b.x, a.y / b.y);
}

vec2& operator+=(vec2& a, const vec2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

vec2& operator-=(vec2& a, const vec2& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

bool operator==(const vec2& a, const vec2& b)
{
	return a.x == b.x && a.y == b.y;
}

vec3 operator+(vec3 a, vec3 b)
{
	return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3 operator-(vec3 a, vec3 b)
{
	return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3 operator*(vec3 a, vec3 b)
{
	return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

vec3 operator/(vec3 a, vec3 b)
{
	return vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

vec3 operator+(vec3 a, float b)
{
	return vec3(a.x + b, a.y + b, a.z + b);
}

vec3 operator-(vec3 a, float b)
{
	return vec3(a.x - b, a.y - b, a.z - b);
}

vec3 operator*(vec3 a, float b)
{
	return vec3(a.x * b, a.y * b, a.z * b);
}

vec3 operator/(vec3 a, float b)
{
	return vec3(a.x / b, a.y / b, a.z / b);
}

vec3 operator+(vec3 a, int b)
{
	return vec3(a.x + b, a.y + b, a.z + b);
}

vec3 operator-(vec3 a, int b)
{
	return vec3(a.x - b, a.y - b, a.z - b);
}

vec3 operator*(vec3 a, int b)
{
	return vec3(a.x * b, a.y * b, a.z * b);
}

vec3 operator/(vec3 a, int b)
{
	return vec3(a.x / b, a.y / b, a.z / b);
}

vec3 operator+(float a, vec3 b)
{
	return vec3(a + b.x, a + b.y, a + b.z);
}

vec3 operator-(float a, vec3 b)
{
	return vec3(a - b.x, a - b.y, a - b.z);
}

vec3 operator*(float a, vec3 b)
{
	return vec3(a * b.x, a * b.y, a * b.z);
}

vec3 operator/(float a, vec3 b)
{
	return vec3(a / b.x, a / b.y, a / b.z);
}

bool operator==(const vec3& a, const vec3& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

vec3& operator*=(vec3& a, float b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
	return a;
}

vec4 operator+(vec4 a, vec4 b)
{
	return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

vec4 operator-(vec4 a, vec4 b)
{
	return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

vec4 operator*(vec4 a, vec4 b)
{
	return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

vec4 operator/(vec4 a, vec4 b)
{
	return vec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

vec4 operator+(vec4 a, float b)
{
	return vec4(a.x + b, a.y + b, a.z + b, a.w + b);
}

vec4 operator-(vec4 a, float b)
{
	return vec4(a.x - b, a.y - b, a.z - b, a.w - b);
}

vec4 operator*(vec4 a, float b)
{
	return vec4(a.x * b, a.y * b, a.z * b, a.w * b);
}

vec4 operator/(vec4 a, float b)
{
	return vec4(a.x / b, a.y / b, a.z / b, a.w / b);
}

vec4 operator+(float a, vec4 b)
{
	return vec4(a + b.x, a + b.y, a + b.z, a + b.w);
}

vec4 operator-(float a, vec4 b)
{
	return vec4(a - b.x, a - b.y, a - b.z, a - b.w);
}

vec4 operator*(float a, vec4 b)
{
	return vec4(a * b.x, a * b.y, a * b.z, a * b.w);
}

vec4 operator/(float a, vec4 b)
{
	return vec4(a / b.x, a / b.y, a / b.z, a / b.w);
}

bool operator==(const vec4& a, const vec4& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

vec4& operator*=(vec4& a, float b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
	a.w *= b;
	return a;
}

ivec3 operator+(ivec3 a, ivec3 b)
{
	return ivec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

ivec3 operator-(ivec3 a, ivec3 b)
{
	return ivec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

ivec3 operator*(ivec3 a, ivec3 b)
{
	return ivec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

ivec3 operator/(ivec3 a, ivec3 b)
{
	return ivec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

ivec3 operator+(ivec3 a, int b)
{
	return ivec3(a.x + b, a.y + b, a.z + b);
}

ivec3 operator-(ivec3 a, int b)
{
	return ivec3(a.x - b, a.y - b, a.z - b);
}

ivec3 operator*(ivec3 a, int b)
{
	return ivec3(a.x * b, a.y * b, a.z * b);
}

ivec3 operator/(ivec3 a, int b)
{
	return ivec3(a.x / b, a.y / b, a.z / b);
}

vec3 operator+(ivec3 a, float b)
{
	return vec3(a.x + b, a.y + b, a.z + b);
}

vec3 operator-(ivec3 a, float b)
{
	return vec3(a.x - b, a.y - b, a.z - b);
}

vec3 operator*(ivec3 a, float b)
{
	return vec3(a.x * b, a.y * b, a.z * b);
}

vec3 operator/(ivec3 a, float b)
{
	return vec3(a.x / b, a.y / b, a.z / b);
}

bool operator==(const ivec3& a, const ivec3& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!=(const ivec3& a, const ivec3& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

float dot(const vec3& a, const vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dot(const vec4& a, const vec4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

vec3 cross(const vec3& a, const vec3& b)
{
	float x = a.y * b.z - a.z * b.y;
	float y = a.z * b.x - a.x * b.z;
	float z = a.x * b.y - a.y * b.x;
	return vec3(x, y, z);
}

ivec3 abs(const ivec3& v)
{
	return ivec3(abs(v.x), abs(v.y), abs(v.z));
}

vec3 abs(const vec3& v)
{
	return vec3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

vec2 abs(const vec2& v)
{
	return vec2(fabsf(v.x), fabsf(v.y));
}

vec2 min(const vec2& a, const vec2& b)
{
	return vec2(fminf(a.x, b.x), fminf(a.y, b.y));
}

vec2 max(const vec2& a, const vec2& b)
{
	return vec2(fmaxf(a.x, b.x), fmaxf(a.y, b.y));
}

vec3 min(const vec3& a, const vec3& b)
{
	return vec3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}

vec3 max(const vec3& a, const vec3& b)
{
	return vec3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}

ivec2 sign(const vec2& v)
{
	return ivec2(fsign(v.x), fsign(v.y));
}

ivec3 sign(const ivec3& v)
{
	return ivec3(fsign((float)v.x), fsign((float)v.y), fsign((float)v.z));
}

vec2 mix(const vec2& a, const vec2& b, float t)
{
	return vec2(
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t
	);
}

vec3 mix(const vec3& a, const vec3& b, float t)
{
	return vec3(
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t
	);
}

vec3 floor(const vec3& v)
{
	return vec3(SDL_floorf(v.x), SDL_floorf(v.y), SDL_floorf(v.z));
}
