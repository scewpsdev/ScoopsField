#include "Audio.h"

#include <new>


#define InitTrashCppObject(x, T) new(x)T()


using namespace SoLoud;


extern AudioState* audio;


bool InitAudio(AudioState* audio, SoLoud::Soloud* soloud)
{
	audio->soloud = soloud;

	InitTrashCppObject(&audio->reverb, FreeverbFilter);
	audio->reverb.setParams(0.0f, 0.5f, 0.5f, 1.0f);

	InitTrashCppObject(&audio->defaultBus, Bus);

	InitTrashCppObject(&audio->reverbBus, Bus);
	audio->reverbBus.setFilter(0, &audio->reverb);

	InitTrashCppObject(&audio->musicBus, Bus);

	// We need to play 3d sounds over a default bus,
	// otherwise sound attenuation will glitch for the first frame of playing (yikes)
	soloud->play(audio->defaultBus);

	audio->reverbBusSource = soloud->play(audio->reverbBus);

	audio->musicBusHandle = soloud->playBackground(audio->musicBus);

	return true;
}

void UpdateAudio(AudioState* audio)
{
	audio->soloud->update3dAudio();
}

void SetAudioListener(const vec3& position, const quat& rotation)
{
	vec3 forward = rotation.forward();
	vec3 up = rotation.up();
	audio->soloud->set3dListenerParameters(position.x, position.y, position.z, forward.x, forward.y, forward.z, up.x, up.y, up.z);
}

bool LoadSound(Sound* sound, const char* path)
{
	InitTrashCppObject(&sound->wav, Wav);
	if (result result = sound->wav.load(path))
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load sound %s", path);
		return false;
	}
	return true;
}

uint32_t PlaySound(Sound* sound, float volume)
{
	uint32_t handle = audio->soloud->playBackground(sound->wav, volume);
	return handle;
}

uint32_t PlaySound(Sound* sound, float pan, float volume)
{
	uint32_t handle = audio->soloud->play(sound->wav, volume, pan);
	return handle;
}

uint32_t PlaySound(Sound* sound, vec3 position, float volume)
{
	uint32_t handle = audio->defaultBus.play3d(sound->wav, position.x, position.y, position.z, 0, 0, 0, volume, false);
	return handle;
}

void StopSound(uint32_t source)
{
	audio->soloud->stop(source);
}
