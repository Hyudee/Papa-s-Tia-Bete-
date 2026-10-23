#pragma once

#include <SDL3/SDL.h>

// Botões
inline bool MouseNoBotao(float x, float y, SDL_FRect botao)
{
    return x >= botao.x &&
        x <= botao.x + botao.w &&
        y >= botao.y &&
        y <= botao.y + botao.h;
}