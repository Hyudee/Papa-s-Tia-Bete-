#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include "config.h"
#include "tipos.h"
#include "ranking.h"
#include "audio.h"
#include "escala.h"
#include "sdl_raii.h"
#include "assets.h"
#include "texto.h"
#include "botoes.h"
#include "jogo.h"

void Jogo::ColisaoPlayerInimigos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo && !inimigos[i].morrendo)
            {
                if (SDL_HasRectIntersectionFloat(
                    &player,
                    &inimigos[i].rect))
                {
                    pontos -= 50;
                    if (pontos < 0) pontos = 0;

                    // Empurrão
                    player.x -= 100.0f * escalaJogo;
                    player.y -= 50.0f * escalaJogo;

                    // Stun
                    stun = true;
                    stunStart = SDL_GetTicks();

                    // animação de dano

                    dano = true;
                    danoStart = SDL_GetTicks();

                    if (olhandoDireita)
                        framePlayer = 13;
                    else
                        framePlayer = 27;

                    inimigos[i].ativo = false;
                }
            }
        }
        if (stun && SDL_GetTicks() - stunStart >= STUN_DURACAO) {
            stun = false;
            if (!dano)
                framePlayer = olhandoDireita ? 0 : 1;
        }
    }
}

void Jogo::ColisaoPlayerBase() {
    if (estado == JOGAR) {
        if (SDL_HasRectIntersectionFloat(
            &player,
            &base))
        {
            player.x = base.x + base.w;

        }
    }
}

void Jogo::ColisaoTiroPowerups() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            if (!vidasPowerup[i].ativo)
                continue;

            for (int j = 0; j < MAX_PROJETEIS; j++)
            {
                if (!projeteis[j].ativo)
                    continue;

                if (SDL_HasRectIntersectionFloat(
                    &projeteis[j].rect,
                    &vidasPowerup[i].rect))
                {
                    if (vidas < vidaMaxima) {
                        vidas++;
                        flashBase = FLASH_CURA;
                        tempoFlashBase = 0.2f;
                    }

                    vidasPowerup[i].ativo = false;
                    projeteis[j].ativo = false;
                    TocarEfeito(somPowerup);
                }
            }
        }
    }
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (!ataquePowerup[i].ativo)
                continue;

            for (int j = 0; j < MAX_PROJETEIS; j++)
            {
                if (!projeteis[j].ativo)
                    continue;

                if (SDL_HasRectIntersectionFloat(
                    &projeteis[j].rect,
                    &ataquePowerup[i].rect))
                {
                    ataqueBuff = true;
                    ataqueBuffInicio = SDL_GetTicks();
                    danoTiro = danoTiroBase * 2;
                    ataquePowerup[i].ativo = false;
                    projeteis[j].ativo = false;
                    TocarEfeito(somPowerup);
                }
            }
        }
    }
}

void Jogo::ColisaoTiroBoss() {
    if (estado == JOGAR) {
        for (int j = 0; j < MAX_PROJETEIS; j++) {
            if (!projeteis[j].ativo)
                continue;

            if (boss.ativo && SDL_HasRectIntersectionFloat(
                &boss.rect,
                &projeteis[j].rect
            ))
            {
                projeteis[j].explodindo = true;
                projeteis[j].frame = 3;
                projeteis[j].velX = 0;
                projeteis[j].velY = 0;
                projeteis[j].ultimoFrame = SDL_GetTicks();

                boss.vida -= danoTiro;
                projeteis[j].ativo = false;

                if (boss.vida <= 0)
                {
                    boss.ativo = false;
                    pontos += 100;
                    bossAtivados++;
                    fimUltimoBoss = SDL_GetTicks();

                    // A oferta já conta o boss que acabou de morrer
                    totalOfertaCartas = MontarOfertaCartas(cartas, bossAtivados, ofertaCartas, CARTAS_POR_OFERTA);
                    opcaoUpgrade = 0;

                    // Se não sobrou carta para oferecer, o jogo segue sem a tela de escolha
                    if (totalOfertaCartas > 0)
                    {
                        inicioPausa = SDL_GetTicks();
                        estado = UPGRADE;
                    }
                }
            }
        }
    }
}

void Jogo::ColisaoBaseInimigos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo && !inimigos[i].morrendo)
            {
                if (SDL_HasRectIntersectionFloat(
                    &base,
                    &inimigos[i].rect))
                {
                    vidas--;
                    flashBase = FLASH_DANO;
                    tempoFlashBase = 0.2f; // 200 ms
                    inimigos[i].ativo = false;

                    if (vidas <= 0)
                    {
                        estado = GAMEOVER;
                        tempoAtual = SDL_GetTicks();
                        RegistrarFimDePartida("inimigo_na_base");
                    }
                }
            }
        }
    }
}

void Jogo::ColisaoBossBase() {
    if (estado == JOGAR) {
        if (boss.ativo) {
            if (SDL_HasRectIntersectionFloat(
                &base,
                &boss.rect
            ))
            {
                vidas -= 3;
                if (vidas < 0) vidas = 0;
                flashBase = FLASH_DANO;
                tempoFlashBase = 0.2f; // 200 ms
                boss.ativo = false;
                spawnBoss = pontos + 1000;
                fimUltimoBoss = SDL_GetTicks();

                if (vidas <= 0)
                {
                    estado = GAMEOVER;
                    tempoAtual = SDL_GetTicks();
                    RegistrarFimDePartida("boss_na_base");
                }
            }
        }

    }
}

void Jogo::ColisaoProjeteisObstaculos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

            for (int j = 0; j < MAX_PROJETEIS; j++)
            {
                if (!projeteis[j].ativo || projeteis[j].explodindo)
                    continue;

                if (SDL_HasRectIntersectionFloat(
                    &projeteis[j].rect,
                    &obstaculos[i].rect))
                {
                    projeteis[j].explodindo = true;
                    projeteis[j].frame = 3;
                    projeteis[j].velX = 0;
                    projeteis[j].velY = 0;
                    projeteis[j].ultimoFrame = SDL_GetTicks();
                }
            }
        }
    }
}

void Jogo::ColisaoProjeteisInimigos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (!inimigos[i].ativo || inimigos[i].morrendo)
                continue;

            for (int j = 0; j < MAX_PROJETEIS; j++)
            {
                if (!projeteis[j].ativo || projeteis[j].explodindo)
                    continue;

                if (SDL_HasRectIntersectionFloat(
                    &inimigos[i].rect,
                    &projeteis[j].rect))
                {
                    inimigos[i].morrendo = true;
                    inimigos[i].flashInicio = SDL_GetTicks();
                    projeteis[j].explodindo = true;
                    projeteis[j].frame = 3;
                    projeteis[j].velX = 0;
                    projeteis[j].velY = 0;
                    projeteis[j].ultimoFrame = SDL_GetTicks();
                    pontos += 50;
                    inimigosDerrotados++;
                }
            }
        }
    }
}
