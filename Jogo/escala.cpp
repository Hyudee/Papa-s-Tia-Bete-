#include "escala.h"

float escalaJogo = 1.0f;

float gravidade = 0.5f;
float forcaPulo = -10.0f;

float SPEED_NORMAL = 5.0f;
float SPEED_SUJO = 2.5f;
float speed = 5.0f;

float tamanhoPlayer = 64.0f;
float tamanhoBoss = 128.0f;
float tamanhoInimigo = 64.0f;
float tamanhoPowerup = 32.0f;
float tamanhoProjetil = 32.0f;
float tamanhoObstaculo = 64.0f;

void AtualizarEscala(int larguraTela, int alturaTela)
{
    escalaJogo = SDL_min(
        larguraTela / LARGURA_BASE,
        alturaTela / ALTURA_BASE
    );

    tamanhoPlayer = 64.0f * escalaJogo;
    tamanhoBoss = 128.0f * escalaJogo;
    tamanhoInimigo = 64.0f * escalaJogo;
    tamanhoPowerup = 32.0f * escalaJogo;
    tamanhoProjetil = 32.0f * escalaJogo;
    tamanhoObstaculo = 64.0f * escalaJogo;

    SPEED_NORMAL = 5.0f * escalaJogo;
    SPEED_SUJO = 2.5f * escalaJogo;

    speed = SPEED_NORMAL;

    gravidade = 0.5f * escalaJogo;
    forcaPulo = -12.5f * escalaJogo;
}