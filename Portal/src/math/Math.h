#pragma once

#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"
#include "Shape.h"

#include <SDL3/SDL.h>


#define PI 3.14159265359f
#define Deg2Rad (PI / 180.0f)
#define Rad2Deg (180.0f / PI)

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_TRANSPARENT 0x0


int ipow(int base, int exp);
int idivfloor(int a, int b);
float clamp(float f, float min, float max);
float remap(float x, float min, float max, float newMin, float newMax);
float smoothstep(float edge0, float edge1, float x);
int sign(float f);
float fract(float x);
float mod(float x, float y);

inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }
inline float min(float a, float b) { return a < b ? a : b; }
inline float max(float a, float b) { return a > b ? a : b; }
template<typename T>
inline T min(T a, T b) { return a < b ? a : b; }
template<typename T>
inline T max(T a, T b) { return a > b ? a : b; }

float lerpAngle(float a, float b, float t);
float moveTowards(float a, float b, float t);
float moveTowardsAngle(float a, float b, float t);

mat4 interpolate(const mat4& a, const mat4& b, float blend);

vec3 RandomPointOnSphere(struct Random& random);
AABB TransformBoundingBox(const AABB& localBox, const mat4& transform);
ivec2 WorldToScreenSpace(const vec3& p, const mat4& vp, int displayWidth, int displayHeight);
bool IsInBounds(const vec3& p, const vec3& min, const vec3& max);
vec4 ARGBToVector(uint32_t argb);
vec3 SRGBToLinear(vec3 color);
vec4 SRGBToLinear(vec4 color);
vec3 DecodeRG11B10(uint32_t bits);
bool FrustumCulling(const Sphere& boundingSphere, vec4 planes[6]);
bool FrustumCulling(const Sphere& boundingSphere, mat4 transform, vec4 planes[6]);
