#pragma once

#include "math/Math.h"

#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>
#include <soloud/soloud_freeverbfilter.h>


struct Sound
{
	SoLoud::Wav wav;
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
};


bool InitAudio(AudioState* audio, SoLoud::Soloud* soloud);
void UpdateAudio(AudioState* audio);
void SetAudioListener(const vec3& position, float pitch, float yaw);

bool LoadSound(Sound* sound, const char* path);
uint32_t PlaySound(Sound* sound, float volume = -1);
uint32_t PlaySound(Sound* sound, float pan, float volume);
uint32_t PlaySound(Sound* sound, vec3 position, float volume = 1);
void StopSound(uint32_t source);
