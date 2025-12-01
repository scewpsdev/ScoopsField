#pragma once

#include <SDL3/SDL.h>


void* LoadASTCImage(const char* path, int* width, int* height, int* size);
void* LoadDDSImage(const char* path, int* width, int* height, int* size);
