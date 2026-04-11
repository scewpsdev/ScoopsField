#pragma once

#include <stdint.h>


float Simplex1f(float x);
float Simplex2f(float x, float y);
float Simplex2fTiled(float x, float y, float frequency, float loopX, float loopY, int seed);
float Simplex3f(float x, float y, float z);
float Simplex4f(float x, float y, float z, float w);
