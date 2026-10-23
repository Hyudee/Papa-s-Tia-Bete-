#pragma once

#include "sdl_raii.h"

// Desenho de texto: cria surface e textura temporárias, desenha e destrói.
// Mesmo comportamento dos blocos que existiam repetidos em main.cpp.

// x do retângulo = centroX - largura / 2
void DesenharTextoCentrado(SDL_Renderer* renderer,
    TTF_Font* fonte,
    const char* texto,
    SDL_Color cor,
    float centroX,
    float y);

// x do retângulo = x
void DesenharTextoEsquerda(SDL_Renderer* renderer,
    TTF_Font* fonte,
    const char* texto,
    SDL_Color cor,
    float x,
    float y);