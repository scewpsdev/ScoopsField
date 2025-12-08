#include "Audio.h"

#include <soloud/soloud_backend_data_sdl3.h>


// we need this because c++ is a garbage language that cant allocate simple structs from a preallocated buffer.
// good job bjarne. clap clap.
#define InitTrashCppObject(x, T) { T t; SDL_memcpy(x, &t, sizeof(T)); }


using namespace SoLoud;


bool InitAudio(AudioState* audio)
{
	SoLoudClientDataSdl3 clientData(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
	result result = audio->soloud.init(1, Soloud::BACKENDS::SDL3, 0, 0, 2, &clientData);
	if (result)
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to initialize audio backend: %s", audio->soloud.getErrorString(result));
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "%s", SDL_GetError());
		return false;
	}

	SDL_Log("Audio backend: %s", audio->soloud.getBackendString());

	InitTrashCppObject(&audio->reverb, FreeverbFilter);
	audio->reverb.setParams(0.0f, 0.5f, 0.5f, 1.0f);

	InitTrashCppObject(&audio->defaultBus, Bus);

	InitTrashCppObject(&audio->reverbBus, Bus);
	audio->reverbBus.setFilter(1, &audio->reverb);

	InitTrashCppObject(&audio->musicBus, Bus);

	// We need to play 3d sounds over a default bus,
	// otherwise sound attenuation will glitch for the first frame of playing (yikes)
	//defaultBus.set3dAttenuation(SoLoud::AudioSource::INVERSE_DISTANCE, 10);
	audio->soloud.play(audio->defaultBus);

	audio->reverbBusSource = audio->soloud.play(audio->reverbBus);

	audio->musicBusHandle = audio->soloud.playBackground(audio->musicBus);

	return true;
}
