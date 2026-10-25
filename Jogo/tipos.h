#pragma once

#include <SDL3/SDL.h>

// Enums e structs do jogo

typedef enum {
    MENU,
    JOGAR,
    PAUSE,
    UPGRADE,
    CREDITOS,
    CONFIGURACOES,
    CONTROLES,
    RANKING,
    AJUDA,
    DIGITAR_NOME,
    RESULTADO,
    GAMEOVER,
    SAIR
} Sistema;

typedef struct
{
    SDL_FRect rect;
    bool      ativo;
    float     speed;

    int frame;
    Uint64 ultimoFrame;

    bool   morrendo;
    Uint64 flashInicio;
} Inimigo;

typedef struct
{
    SDL_FRect rect;
    bool ativo;
    float speed;

    int frame;
    Uint64 ultimoFrame;
} PowerupVida;

typedef struct
{
    SDL_FRect rect;
    bool ativo;
    float speed;

    int frame;
    Uint64 ultimoFrame;
} PowerupAtq;

typedef struct {
    SDL_FRect rect;
    bool ativo;
    float speed;

    int vida;
    int vidaMax;

    int frame;
    Uint64 ultimoFrame;
}Boss;

typedef struct
{
    SDL_FRect rect;
    bool      ativo;
    float     velX;
    float     velY;
    float angulo;   // em graus; o sprite original aponta para a direita
    int idDisparo;  // balas do mesmo clique têm o mesmo id (o Tiro Duplo conta como 1 disparo)
    int dano;       // dano desta bala no boss, definido na hora do disparo

    int frame;
    Uint64 ultimoFrame;
    bool explodindo;
} Projetil;

typedef struct
{
    char nome[20];
    int pontos;
} Ranking;

typedef enum
{
    OBSTACULO_SOMBRA,
    OBSTACULO_CAINDO,
    OBSTACULO_CHAO,
    OBSTACULO_QUEBRANDO
} EstadoObstaculo;

typedef struct
{
    SDL_FRect rect;

    bool ativo;

    EstadoObstaculo estado;

    int frame;
    Uint64 ultimoFrame;

    float velocidadeY;

    Uint64 inicioEstado;

} Obstaculo;

enum FlashBase
{
    FLASH_NENHUM,
    FLASH_DANO,
    FLASH_CURA
};