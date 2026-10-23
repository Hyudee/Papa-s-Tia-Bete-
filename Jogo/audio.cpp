#include "audio.h"

bool musicaLigada = true;
bool efeitosLigados = true;

void TocarMusica(Audio& audio,
    const char* arquivo,
    float volume)
{
    if (!musicaLigada)
        return;

    if (audio.stream != nullptr)
        return;

    // Falhou há pouco: não tenta de novo a cada frame
    if (audio.falhou && SDL_GetTicks() - audio.ultimaFalha < 2000)
        return;

    if (!SDL_LoadWAV(arquivo,
        &audio.spec,
        &audio.buffer,
        &audio.tamanho))
    {
        SDL_Log("Falha ao carregar %s: %s", arquivo, SDL_GetError());

        audio.buffer = nullptr;
        audio.tamanho = 0;
        audio.falhou = true;
        audio.ultimaFalha = SDL_GetTicks();
        return;
    }

    audio.stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &audio.spec,
        NULL,
        NULL
    );

    if (audio.stream == nullptr)
    {
        SDL_Log("Falha ao abrir o dispositivo de audio para %s: %s", arquivo, SDL_GetError());

        SDL_free(audio.buffer);
        audio.buffer = nullptr;
        audio.tamanho = 0;
        audio.falhou = true;
        audio.ultimaFalha = SDL_GetTicks();
        return;
    }

    audio.falhou = false;

    SDL_SetAudioStreamGain(audio.stream, volume);

    SDL_PutAudioStreamData(
        audio.stream,
        audio.buffer,
        audio.tamanho
    );

    SDL_ResumeAudioStreamDevice(audio.stream);
}

void AtualizarMusica(Audio& audio)
{
    if (!musicaLigada)
        return;

    if (audio.stream == nullptr)
        return;

    if (SDL_GetAudioStreamQueued(audio.stream) == 0)
    {
        SDL_PutAudioStreamData(
            audio.stream,
            audio.buffer,
            audio.tamanho
        );
    }
}

void PararMusica(Audio& audio)
{
    if (audio.stream == nullptr)
        return;

    SDL_DestroyAudioStream(audio.stream);
    audio.stream = nullptr;

    SDL_free(audio.buffer);
    audio.buffer = nullptr;

    audio.tamanho = 0;
}

// Efeitos sonoros

void CarregarEfeito(Audio& audio,
    const char* arquivo,
    float volume)
{
    if (!SDL_LoadWAV(arquivo,
        &audio.spec,
        &audio.buffer,
        &audio.tamanho))
    {
        SDL_Log("Falha ao carregar %s: %s", arquivo, SDL_GetError());
        return;
    }

    audio.stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &audio.spec,
        NULL,
        NULL);

    if (audio.stream == nullptr)
        SDL_Log("Falha ao abrir o dispositivo de audio para %s: %s", arquivo, SDL_GetError());

    SDL_SetAudioStreamGain(audio.stream, volume);
}

void TocarEfeito(Audio& audio)
{
    if (!efeitosLigados)
        return;

    if (audio.stream == nullptr)
        return;

    SDL_ClearAudioStream(audio.stream);

    SDL_PutAudioStreamData(
        audio.stream,
        audio.buffer,
        audio.tamanho);

    SDL_ResumeAudioStreamDevice(audio.stream);
}

void LiberarEfeito(Audio& audio)
{
    if (audio.stream)
    {
        SDL_DestroyAudioStream(audio.stream);
        audio.stream = nullptr;
    }

    if (audio.buffer)
    {
        SDL_free(audio.buffer);
        audio.buffer = nullptr;
    }

    audio.tamanho = 0;
}