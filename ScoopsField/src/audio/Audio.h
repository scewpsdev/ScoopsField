#pragma once

#include <soloud.h>
#include <soloud_freeverbfilter.h>


struct AudioState
{
	SoLoud::Soloud soloud;
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


bool InitAudio(AudioState* audio);
