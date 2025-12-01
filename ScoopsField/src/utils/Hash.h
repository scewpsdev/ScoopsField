#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"

#include <stdint.h>


uint32_t hash(uint32_t i);
uint32_t hash(int i);
uint32_t hash(float f);

uint32_t hash(const vec3& v);
uint32_t hash(const Quaternion& q);

uint32_t hash(const char* str);

uint32_t hash(const void* ptr);

uint32_t hashCombine(uint32_t h0, uint32_t h1);
