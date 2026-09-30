#pragma once

#include <SDL3/SDL.h>

#include "GameMemory.h"


#define PROJECT_PATH "D:\\Dev\\ScoopsField\\Skeld"


struct AppState;
struct GraphicsState;
struct PhysicsState;
struct ResourceState;


extern GameMemory* memory;
extern AppState* app;
extern GraphicsState* graphics;
extern PhysicsState* physics;
extern ResourceState* resource;
extern SDL_GPUDevice* device;

extern SDL_GPUCommandBuffer* cmdBuffer;


void* PhysicsMalloc(size_t size);
void PhysicsFree(void* mem);
void* MeshMalloc(size_t size);
void MeshFree(void* mem);
void* ParticleMalloc(size_t size);
void ParticleFree(void* mem);
void* GraphicsMalloc(size_t size);
void GraphicsFree(void* mem);

bool EveryInterval(float seconds, uint32_t h);
bool GetKey(SDL_Scancode key);
bool GetKeyDown(SDL_Scancode key);
bool GetKeyUp(SDL_Scancode key);
bool GetMouseButton(uint32_t button);
bool GetMouseButtonDown(uint32_t button);
bool GetMouseButtonUp(uint32_t button);
int GetMouseScroll();

void DebugTextEx(int x, int y, const char* txt, int len, uint32_t color, uint32_t bgcolor);
void DebugText(int x, int y, uint32_t color, uint32_t bgcolor, const char* fmt, ...);
void DebugText(int x, int y, const char* fmt, ...);
