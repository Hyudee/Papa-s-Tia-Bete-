#pragma once

#include <SDL3/SDL.h>
#include "inimigos.h"

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

    TipoInimigo tipo;       // 0 = comum (precisa ser zero por causa do memset)
    int    vida;            // golpes que ainda aguenta
    int    vidaMax;
    Uint64 hitInicio;       // instante do ultimo golpe que NAO matou (flash de acerto, Lote 13b)

    bool   temCor;          // true = usa a cor propria abaixo (Granulado); zero = sem cor propria
    unsigned char corR, corG, corB;

    int    ultimoIdDisparo; // id do ultimo disparo que acertou (tipos com golpesFixos); 0 = nenhum

    float  yBase;           // Caramelo: linha central do zigue-zague
    float  fase;            // Caramelo: fase da senoide, em radianos
    float  angulo;          // inclinacao do sprite em graus (so o Caramelo usa; 0 nos demais)

    EstadoPirulito estadoPirulito;
    float  alvoX;           // x onde para
    Uint64 ultimoDisparo;   // inicio da espera pelo proximo tiro (= fim do ultimo disparo)
    Uint64 disparoInicio;   // inicio da animacao de disparo atual
    bool   atirou;          // o tiro deste disparo ja saiu
} Inimigo;

typedef struct
{
    SDL_FRect rect;
    bool   ativo;
    float  velX;
    float  velY;
    float  angulo;          // em graus; o sprite aponta para a direita
    int    frame;
    Uint64 ultimoFrame;
    bool   impacto;         // true = acertou o jogador e toca a animacao de impacto (parado, sem colisao)
} TiroInimigo;

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


    Uint64 lentidaoFim;     // instante em que acaba a lentidão do Glacê (0 = sem lentidão)
    int lentidaoPercent;    // quanto da velocidade ele perde enquanto durar
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
    int perfuracoes;    // quantos minions esta bala ainda pode atravessar (Perfurante)
    int alvoAtravessado;    // minion (indice + 1) que a bala acabou de atravessar; 0 = nenhum (nao acerta o mesmo duas vezes)

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

typedef struct
{
    bool   ativa;
    float  x;       // centro da explosão
    float  y;
    float  raio;    // raio máximo, já multiplicado por escalaJogo
    Uint64 inicio;
} Explosao;

enum FlashBase
{
    FLASH_NENHUM,
    FLASH_DANO,
    FLASH_CURA
};