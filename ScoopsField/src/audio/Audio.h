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

bool LoadSound(Sound* sound, const char* path);
uint32_t PlaySound(Sound* sound);
uint32_t PlaySound(Sound* sound, vec3 position);
