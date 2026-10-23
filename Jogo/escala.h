#pragma once

#include <SDL3/SDL.h>

// Escala do jogo

inline constexpr float LARGURA_BASE = 840.0f;
inline constexpr float ALTURA_BASE = 480.0f;

extern float escalaJogo;

extern float gravidade;
extern float forcaPulo;

extern float SPEED_NORMAL;
extern float SPEED_SUJO;
extern float speed;

extern float tamanhoPlayer;
extern float tamanhoBoss;
extern float tamanhoInimigo;
extern float tamanhoPowerup;
extern float tamanhoProjetil;
extern float tamanhoObstaculo;

void AtualizarEscala(int larguraTela, int alturaTela);