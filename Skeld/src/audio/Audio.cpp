#include "Audio.h"

#include "utils/Hash.h"

#include <new>


#define InitTrashCppObject(x, T) new(x)T()
#define DestroyTrashCppObject(x, T) x->~T()


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

	audio->randomHash = 12345;

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
	sound->numWavs = 1;
	InitTrashCppObject(&sound->wavs[0], Wav);
	if (result result = sound->wavs[0].load(path))
	{
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load sound %s", path);
		return false;
	}
	sound->wavs[0].set3dAttenuation(SoLoud::AudioSource::INVERSE_DISTANCE, 0.25f);
	sound->wavs[0].set3dMinMaxDistance(2, 1000);
	return true;
}

bool LoadSounds(Sound* sound, const char* name, int count)
{
	SDL_assert(count > 0 && count <= MAX_SOUND_VARIATIONS);

	bool r = true;

	for (int i = 0; i < count; i++)
	{
		char path[256];
		SDL_snprintf(path, 256, "res/%s%d.ogg.bin", name, i + 1);

		InitTrashCppObject(&sound->wavs[i], Wav);
		if (result result = sound->wavs[i].load(path))
		{
			SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load sound %s", path);
			r = false;
		}

		sound->wavs[i].set3dAttenuation(SoLoud::AudioSource::INVERSE_DISTANCE, 0.25f);
		sound->wavs[i].set3dMinMaxDistance(2, 1000);
	}

	sound->numWavs = count;

	return r;
}

void DestroySound(Sound* sound)
{
	for (int i = 0; i < sound->numWavs; i++)
	{
		sound->wavs[i].stop();
		sound->wavs[i].~Wav();
	}
}

static int GetRandom(int count)
{
	int idx = (int)(audio->randomHash % count);
	audio->randomHash = hash(audio->randomHash);
	return idx;
}

uint32_t PlaySound(Sound* sound, float volume)
{
	uint32_t handle = audio->defaultBus.play(sound->wavs[GetRandom(sound->numWavs)], volume);
	return handle;
}

uint32_t PlaySound(Sound* sound, float pan, float volume)
{
	uint32_t handle = audio->defaultBus.play(sound->wavs[GetRandom(sound->numWavs)], volume, pan);
	return handle;
}

uint32_t PlaySound(Sound* sound, vec3 position, float volume)
{
	uint32_t handle = audio->defaultBus.play3d(sound->wavs[GetRandom(sound->numWavs)], position.x, position.y, position.z, 0, 0, 0, volume, false);
	return handle;
}

void StopSound(uint32_t source)
{
	audio->soloud->stop(source);
}

void SetSoundRelativeSpeed(uint32_t handle, float speed)
{
	audio->soloud->setRelativePlaySpeed(handle, speed);
}
