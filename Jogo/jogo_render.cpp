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

void Jogo::CalcularShake() {
    shakeX = 0.0f;
    shakeY = 0.0f;

    if (shakeDuracao > 0.0f)
    {
        Uint64 tempoShake = SDL_GetTicks() - shakeInicio;

        if (tempoShake < (Uint64)shakeDuracao)
        {
            float progresso = 1.0f - (tempoShake / shakeDuracao);
            shakeX = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * shakeForca * progresso;
            shakeY = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * shakeForca * progresso;
        }
        else
        {
            shakeDuracao = 0.0f;
            shakeX = 0.0f;
            shakeY = 0.0f;
        }
    }
}

void Jogo::RenderizarMundo() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    // Aplica viewport
    SDL_Rect viewport = {
        (int)shakeX,
        (int)shakeY,
        larguraTela,
        alturaTela
    };
    SDL_SetRenderViewport(renderer, &viewport);

    // Fundo
    SDL_FRect fundo = {
        0,
        0,
        (float)larguraTela,
        (float)alturaTela
    };

    SDL_RenderTexture(renderer, background, NULL, &fundo);

    // Base
    SDL_Texture* texturaBase = baseTexture;
    int frameIndex;

    if (vidas >= 4)
        frameIndex = 0;
    else
        frameIndex = 4 - vidas;

    SDL_FRect srcBase = {
        (float)(frameIndex * larguraFrameBase),
        0.0f,
        (float)larguraFrameBase,
        (float)alturaFrameBase
    };

    if (tempoFlashBase > 0.0f)
    {
        if (flashBase == FLASH_DANO)
            SDL_SetTextureColorMod(texturaBase, 255, 80, 80);
        else if (flashBase == FLASH_CURA)
            SDL_SetTextureColorMod(texturaBase, 80, 255, 80);
    }
    else
    {
        SDL_SetTextureColorMod(texturaBase, 255, 255, 255);
    }

    SDL_RenderTexture(renderer, texturaBase, &srcBase, &base);
    SDL_SetTextureColorMod(texturaBase, 255, 255, 255);

    // Animação do player
    static Uint64 ultimoFrameAndar = 0;

    bool andando = key[SDL_SCANCODE_A] || key[SDL_SCANCODE_D];
    if (estado == JOGAR)
    {
        if (!dano && !atirando && !pulando && !aterrissando)
        {
            if (andando)
            {
                if (SDL_GetTicks() - ultimoFrameAndar > 120)
                {
                    if (olhandoDireita)
                    {
                        framePlayer++;
                        if (framePlayer < 16 || framePlayer > 19)
                            framePlayer = 16;
                    }
                    else
                    {
                        framePlayer++;
                        if (framePlayer < 20 || framePlayer > 23)
                            framePlayer = 20;
                    }
                    ultimoFrameAndar = SDL_GetTicks();
                }
            }
            else
            {
                framePlayer = olhandoDireita ? 0 : 1;
            }
        }
    }

    // Jogador
    SDL_FRect srcPlayer = {
        (float)(framePlayer * 64),
        0,
        64,
        64
    };

    SDL_Texture* texturaAtualPlayer = sujo ? texturaPlayerSujo : texturaPlayer;

    SDL_RenderTexture(renderer, texturaAtualPlayer, &srcPlayer, &player);

    // Balas
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        if (!projeteis[i].ativo)
            continue;

        SDL_FRect srcTiro = {
            (float)(projeteis[i].frame * larguraFrameTiro),
            0,
            (float)larguraFrameTiro,
            (float)alturaFrameTiro
        };

        // Gira o sprite para a direção do tiro (a explosão não gira)
        double anguloTiro = projeteis[i].explodindo ? 0.0 : (double)projeteis[i].angulo;

        SDL_RenderTextureRotated(renderer, projetil, &srcTiro, &projeteis[i].rect,
            anguloTiro, NULL, SDL_FLIP_NONE);
    }

    // Inimigos
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        if (!inimigos[i].ativo && !inimigos[i].morrendo)
            continue;

        if (inimigos[i].morrendo)
            SDL_SetTextureColorMod(texturaInimigo, 255, 80, 80);

        SDL_FRect src = {            // bloco dos inimigos
            (float)(inimigos[i].frame * larguraFrameInimigo),
            0,
            (float)larguraFrameInimigo,
            (float)alturaFrameInimigo
        };

        SDL_RenderTexture(renderer, texturaInimigo, &src, &inimigos[i].rect);

        if (inimigos[i].morrendo)
            SDL_SetTextureColorMod(texturaInimigo, 255, 255, 255);
    }

    // Obstáculos
    for (int i = 0; i < MAX_OBSTACULOS; i++)
    {
        if (!obstaculos[i].ativo)
            continue;

        SDL_FRect srcSombra = {
            (float)(obstaculos[i].frame * 64),
            0,
            64,
            64
        };

        SDL_FRect sombraRect = {
            obstaculos[i].rect.x,
            (float)(alturaTela - tamanhoObstaculo),
            tamanhoObstaculo,
            tamanhoObstaculo
        };

        if (obstaculos[i].estado == OBSTACULO_SOMBRA)
        {
            SDL_RenderTexture(renderer, texturaSombra, &srcSombra, &sombraRect);
        }
        else if (obstaculos[i].estado == OBSTACULO_CAINDO)
        {
            SDL_RenderTexture(renderer, texturaSombra, &srcSombra, &sombraRect);

            SDL_FRect srcObstaculo = {
                (float)(obstaculos[i].frame * 64),
                0,
                64,
                64
            };

            SDL_RenderTexture(renderer, texturaObstaculo, &srcObstaculo, &obstaculos[i].rect);
        }
        else
        {
            SDL_FRect srcObstaculo = {
                obstaculos[i].frame * 64,
                0,
                64,
                64
            };

            SDL_RenderTexture(renderer, texturaObstaculo, &srcObstaculo, &obstaculos[i].rect);
        }
    }

    // Boss
    if (boss.ativo)
    {
        SDL_FRect srcBoss = {
            (float)(boss.frame * 128),
            0,
            128,
            128
        };

        SDL_RenderTexture(renderer, texturaBoss, &srcBoss, &boss.rect);
    }

    // Powerups
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (!vidasPowerup[i].ativo)
            continue;

        SDL_FRect src = {            // powerup de vida
            (float)(vidasPowerup[i].frame * larguraFramePowerUp),
            0,
            (float)larguraFramePowerUp,
            (float)alturaFramePowerUp
        };

        SDL_FRect dest = vidasPowerup[i].rect;
        dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

        SDL_RenderTexture(renderer, powerUpVida, &src, &dest);
    }

    for (int i = 0; i < MAX_POWERUPSATQ; i++)
    {
        if (!ataquePowerup[i].ativo)
            continue;

        SDL_FRect src = {            // powerup de ataque
            (float)(ataquePowerup[i].frame * larguraFramePowerUp),
            0,
            (float)larguraFramePowerUp,
            (float)alturaFramePowerUp
        };

        SDL_FRect dest = ataquePowerup[i].rect;
        dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

        SDL_RenderTexture(renderer, powerUpAtq, &src, &dest);
    }
}

void Jogo::RenderizarHud() {
    SDL_SetRenderViewport(renderer, NULL);

    // Barra de vida do boss
    if (boss.ativo)
    {
        SDL_FRect fundoVida = {
            (float)(larguraTela / 2 - 200),
            20,
            400,
            25
        };

        SDL_FRect vidaBoss = {
            (float)(larguraTela / 2 - 200),
            20,
            400.0f * ((float)boss.vida / boss.vidaMax),
            25
        };

        SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
        SDL_RenderFillRect(renderer, &fundoVida);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &vidaBoss);
    }

    // Vidas
    for (int i = 0; i < vidas; i++)
    {
        SDL_FRect vida = {
            (float)(10 + (i * 40)),
            10,
            32,
            32
        };

        SDL_RenderTexture(renderer, coracao, NULL, &vida);
    }

    // Pontuação
    SDL_GetTextureSize(texturaPontos, &textoW, &textoH);
    textoRect.x = larguraTela - textoW - 10;
    textoRect.y = 10;
    textoRect.w = textoW;
    textoRect.h = textoH;
    SDL_RenderTexture(renderer, texturaPontos, NULL, &textoRect);

    // Ondas
    SDL_GetTextureSize(TexturaOndas, &textoW, &textoH);
    textoRect.x = larguraTela - textoW - 10;
    textoRect.y = 40;
    textoRect.w = textoW;
    textoRect.h = textoH;
    SDL_RenderTexture(renderer, TexturaOndas, NULL, &textoRect);

    // Buff
    if (ataqueBuff)
    {
        SDL_FRect buffRect;
        SDL_GetTextureSize(texturaBuff, &buffRect.w, &buffRect.h);
        buffRect.x = 10;
        buffRect.y = 50;
        SDL_RenderTexture(renderer, texturaBuff, NULL, &buffRect);
    }


    // Em Pause/Upgrade o relógio dos indicadores fica parado em inicioPausa
    Uint64 agora = (estado == JOGAR) ? SDL_GetTicks() : inicioPausa;

    // Barreira da Base: uma caixa por carga (azul = carregada) e barra de recarga
    if (barreirasMax > 0)
    {
        for (int i = 0; i < barreirasMax; i++)
        {
            SDL_FRect caixa = { 10.0f + i * 34.0f, 90.0f, 28.0f, 14.0f };

            if (i < barreiraCargas)
                SDL_SetRenderDrawColor(renderer, 90, 170, 255, 255);
            else
                SDL_SetRenderDrawColor(renderer, 50, 60, 80, 255);

            SDL_RenderFillRect(renderer, &caixa);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderRect(renderer, &caixa);
        }

        if (barreiraCargas < barreirasMax)
        {
            float decorrido = (agora > inicioRecargaBarreira) ? (float)(agora - inicioRecargaBarreira) : 0.0f;
            float progresso = SDL_min(1.0f, decorrido / (float)RECARGA_BARREIRA);
            float larguraTotal = barreirasMax * 34.0f - 6.0f;

            SDL_FRect fundo = { 10.0f, 108.0f, larguraTotal, 4.0f };
            SDL_FRect cheio = { 10.0f, 108.0f, larguraTotal * progresso, 4.0f };

            SDL_SetRenderDrawColor(renderer, 50, 60, 80, 255);
            SDL_RenderFillRect(renderer, &fundo);
            SDL_SetRenderDrawColor(renderer, 90, 170, 255, 255);
            SDL_RenderFillRect(renderer, &cheio);
        }
    }
}

void Jogo::RenderizarPause() {
    if (estado == PAUSE)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);

        SDL_FRect fundoPause = {
            0,
            0,
            (float)larguraTela,
            (float)alturaTela
        };

        SDL_RenderFillRect(renderer, &fundoPause);

        SDL_FRect painel = {
            larguraTela * 0.30f,
            alturaTela * 0.18f,
            larguraTela * 0.40f,
            alturaTela * 0.60f
        };

        SDL_SetRenderDrawColor(renderer, 240, 220, 180, 160);
        SDL_RenderFillRect(renderer, &painel);

        SDL_SetRenderDrawColor(renderer, 90, 60, 30, 255);
        SDL_RenderRect(renderer, &painel);

        DesenharTextoCentrado(renderer, font, "PAUSADO", branco,
            painel.x + painel.w / 2, painel.y + 30);

        for (int i = 0; i < 2; i++)
        {
            SDL_FRect rect = RetanguloOpcaoPause(i);

            if (i == opcaoPause)
                DesenharDestaque(rect, margemDestaqueX, margemDestaqueY, corDestaqueMarrom);

            SDL_Color cor = (i == opcaoPause) ? preto : branco;
            DesenharTextoCentrado(renderer, font, opcoesPause[i], cor,
                rect.x + rect.w / 2, rect.y);
        }
    }
}

// Imagem de cada carta (as texturas pertencem ao Assets)
SDL_Texture* Jogo::ImagemDaCarta(IdCarta id) const
{
    switch (id)
    {
    case CARTA_MAIS_DANO:    return upgradeImgDano;
    case CARTA_MAIS_VIDA:    return upgradeImgVida;
    case CARTA_MAIS_BALAS:   return upgradeImgBalas;
    case CARTA_TIRO_DUPLO:   return upgradeImgDuplo;
    case CARTA_CACADOR_BOSS: return upgradeImgCacador;
    default:                 return nullptr;
    }
}