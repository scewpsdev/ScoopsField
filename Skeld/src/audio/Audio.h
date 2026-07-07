#pragma once

#include "math/Math.h"

#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>
#include <soloud/soloud_freeverbfilter.h>


struct Sound
{
#define MAX_SOUND_VARIATIONS 10
	SoLoud::Wav wavs[MAX_SOUND_VARIATIONS];
	int numWavs;
};

struct AudioState
{
	SoLoud::Soloud* soloud;
	SoLoud::Bus defaultBus;
	SoLoud::Bus reverbBus;
	SoLoud::Bus musicBus;
	SoLoud::handle musicBusHandle;
	SoLoud::handle reverbBusSource;
	bool reverbEnabled = false;

	SoLoud::FreeverbFilter reverb;

	float _3dVolume = 1.0f;
	float musicVolume = 1.0f;

	uint32_t randomHash;
};


bool InitAudio(AudioState* audio, SoLoud::Soloud* soloud);
void UpdateAudio(AudioState* audio);
void SetAudioListener(const vec3& position, const quat& rotation);

bool LoadSound(Sound* sound, const char* path);
bool LoadSounds(Sound* sound, const char* name, int count);
void DestroySound(Sound* sound);
uint32_t PlaySound(Sound* sound, float volume = -1);
uint32_t PlaySound(Sound* sound, float pan, float volume);
uint32_t PlaySound(Sound* sound, vec3 position, float volume = 1);
void StopSound(uint32_t source);
void SetSoundRelativeSpeed(uint32_t handle, float speed);
