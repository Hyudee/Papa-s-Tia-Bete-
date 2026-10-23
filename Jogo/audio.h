#pragma once

#include <SDL3/SDL.h>

// Áudio

struct Audio
{
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 tamanho = 0;
    SDL_AudioStream* stream = nullptr;

    // Falha ao carregar/abrir: evita tentar de novo a cada frame
    bool falhou = false;
    Uint64 ultimaFalha = 0;
};

extern bool musicaLigada;
extern bool efeitosLigados;

// Música
void TocarMusica(Audio& audio, const char* arquivo, float volume = 0.05f);
void AtualizarMusica(Audio& audio);
void PararMusica(Audio& audio);

// Efeitos sonoros
void CarregarEfeito(Audio& audio, const char* arquivo, float volume = 0.05f);
void TocarEfeito(Audio& audio);
void LiberarEfeito(Audio& audio);