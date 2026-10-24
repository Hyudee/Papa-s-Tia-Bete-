#include <string.h>
#include "escala.h"
#include "jogo.h"

// Reinício de partida: tudo que precisa voltar ao valor inicial quando uma nova partida começa.
// (Une os dois resets que existiam: o do botão "Jogar" e o da tela de resultado.)
void Jogo::ReiniciarPartida()
{
    // Jogador
    player.x = 200.0f;
    player.y = 240.0f;
    player.w = tamanhoPlayer;
    player.h = tamanhoPlayer;
    speed = SPEED_NORMAL;
    velocidadeY = 0.0f;
    noChao = true;

    // Placar e progressão
    pontos = 0;
    ondas = 1;
    ondaAntiga = -1;
    pontosAntigos = -1;
    vidas = MAX_VIDAS;
    vidaMaxima = MAX_VIDAS;
    inimigosAtivos = MaximoInimigosVivos(ondas);
    inimigosDerrotados = 0;
    spam = false;
    intervaloSpawn = IntervaloSpawn(ondas);

    // Upgrades
    tiroDuplo = false;
    cacadorBoss = false;
    danoTiro = 1;
    danoTiroBase = 1;
    tirosExtras = 0;
    quantidadeTiros = 3;
    ataqueBuff = false;
    upgradeTiroDuploDisponivel = false;
    upgradeCacadordeBossDisponivel = false;

    // Boss
    boss = { 0 };
    spawnBoss = 1000;
    bossAtivados = 0;
    fimUltimoBoss = SDL_GetTicks();

    // Temporizadores
    inicioOnda = SDL_GetTicks();
    inicioSpam = SDL_GetTicks();
    ultimoSpawn = SDL_GetTicks();
    ultimoSpawnObstaculo = SDL_GetTicks();
    ultimoSpawnPowerUp = SDL_GetTicks();
    ultimoSpawnAtq = SDL_GetTicks();
    intervaloSpawnAtq = 40000;

    // Entidades
    memset(inimigos, 0, sizeof(inimigos));
    memset(projeteis, 0, sizeof(projeteis));
    memset(vidasPowerup, 0, sizeof(vidasPowerup));
    memset(ataquePowerup, 0, sizeof(ataquePowerup));
    memset(obstaculos, 0, sizeof(obstaculos));

    metricas.IniciarPartida();
}

// Grava a partida que acabou em metricas.csv (a própria Metricas ignora chamadas repetidas)
void Jogo::RegistrarFimDePartida(const char* causa)
{
    ResumoPartida resumo;

    resumo.onda = ondas;
    resumo.pontos = pontos;
    resumo.bossesDerrotados = bossAtivados;
    resumo.causa = causa;
    resumo.danoBase = danoTiroBase;
    resumo.tirosExtras = tirosExtras;
    resumo.vidaMaxima = vidaMaxima;
    resumo.tiroDuplo = tiroDuplo;
    resumo.cacadorBoss = cacadorBoss;
    resumo.larguraTela = larguraTela;
    resumo.alturaTela = alturaTela;

    metricas.FinalizarPartida(resumo);
}

// Pause e Upgrade param o jogo, mas SDL_GetTicks() continua correndo.
// Ao voltar ao jogo, todos os instantes guardados são empurrados pelo tempo parado,
// para que ondas, buffs, spawns, stun, obstáculos e animações continuem de onde pararam.
void Jogo::DeslocarTimers(Uint64 delta)
{
    // Partida
    inicioOnda += delta;
    inicioSpam += delta;
    ultimoSpawn += delta;
    ultimoSpawnPowerUp += delta;
    ultimoSpawnAtq += delta;
    ultimoSpawnObstaculo += delta;
    fimUltimoBoss += delta;

    // Jogador
    ataqueBuffInicio += delta;
    sujoInicio += delta;
    stunStart += delta;
    danoStart += delta;
    tempoBordaDir += delta;
    inicioAterrissagem += delta;
    shakeInicio += delta;

    // Animações
    ultimoFramePlayer += delta;
    ultimoFramePowerUp += delta;
    ultimoFrameBase += delta;

    // Entidades
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        inimigos[i].ultimoFrame += delta;
        inimigos[i].flashInicio += delta;
    }

    for (int i = 0; i < MAX_PROJETEIS; i++)
        projeteis[i].ultimoFrame += delta;

    for (int i = 0; i < MAX_POWERUPS; i++)
        vidasPowerup[i].ultimoFrame += delta;

    for (int i = 0; i < MAX_POWERUPSATQ; i++)
        ataquePowerup[i].ultimoFrame += delta;

    for (int i = 0; i < MAX_OBSTACULOS; i++)
    {
        obstaculos[i].ultimoFrame += delta;
        obstaculos[i].inicioEstado += delta;
    }

    boss.ultimoFrame += delta;
}