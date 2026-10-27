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

void Jogo::LerTeclado() {
    key = SDL_GetKeyboardState(NULL);

    playerXAnterior = player.x;
    playerYAnterior = player.y;
}

void Jogo::MoverJogador() {
    if (estado == JOGAR) {
        if (!stun && !dano) {

            if (key[SDL_SCANCODE_A])
            {
                player.x -= speed * velocidadeJogadorPercent / 100.0f;
                olhandoDireita = false;
            }

            if (key[SDL_SCANCODE_D])
            {
                player.x += speed * velocidadeJogadorPercent / 100.0f;
                olhandoDireita = true;
            }

            if ((key[SDL_SCANCODE_W] || key[SDL_SCANCODE_SPACE]) && noChao)
            {
                velocidadeY = forcaPulo;
                noChao = false;
                pulando = true;
                TocarEfeito(somPulo);

                if (olhandoDireita)
                    framePlayer = 11;
                else
                    framePlayer = 24;
            }

        }
    }
}

void Jogo::AnimarJogador() {
    if (estado == JOGAR) {
        if (atirando)
        {
            if (SDL_GetTicks() - ultimoFramePlayer > 80)
            {
                framePlayer++;

                if (olhandoDireita)
                {
                    if (framePlayer > 5)
                    {
                        atirando = false;
                        framePlayer = 0;
                    }
                }
                else
                {
                    if (framePlayer > 9)
                    {
                        atirando = false;
                        framePlayer = 1;
                    }
                }
                ultimoFramePlayer = SDL_GetTicks();
            }
        }
    }
}

void Jogo::AnimarDano() {
    if (estado == JOGAR) {
        if (dano)
        {
            Uint64 tempo = SDL_GetTicks() - danoStart;

            // 3 quadros de dano espalhados pela duracao atual (900 ms = 300/600/900, como antes;
            // o stun do tiro do Pirulito usa 700 ms e a animacao acaba junto com ele)
            int passo = (int)(tempo * 3 / danoDuracaoAtual);

            if (passo >= 3)
                dano = false;
            else
                framePlayer = (olhandoDireita ? 13 : 27) + passo;
        }
    }
}

void Jogo::ColisoesTela() {
    if (player.x < 0) {
        player.x = 0;
    }

    if (estado == JOGAR) {
        if (player.x + player.w >= larguraTela)
        {
            player.x = larguraTela - player.w;

            if (!naBordaDir)
            {
                naBordaDir = true;
                tempoBordaDir = SDL_GetTicks();
            }
            else if (SDL_GetTicks() - tempoBordaDir >= LIMITE_BORDA)
            {
                // Knockback
                player.x -= 200.0f * escalaJogo;
                velocidadeY = forcaPulo * 0.9f;

                // Stun
                stun = true;
                stunStart = SDL_GetTicks();
                stunDuracaoAtual = STUN_DURACAO;
                danoDuracaoAtual = DURACAO_DANO;
                dano = true;
                danoStart = SDL_GetTicks();

                if (olhandoDireita)
                    framePlayer = 13;
                else
                    framePlayer = 27;

                naBordaDir = false;
            }
        }
        else
        {
            naBordaDir = false;
        }
    }
}

void Jogo::FisicaJogador() {
    if (estado == JOGAR) {
        // Gravidade
        velocidadeY += gravidade;
        player.y += velocidadeY;

        // Chão
        if (player.y >= alturaChao)
        {
            player.y = alturaChao;
            velocidadeY = 0.0f;
            noChao = true;

            if (pulando)
            {
                pulando = false;
                aterrissando = true;
                TocarEfeito(somAterrissagem);
                inicioAterrissagem = SDL_GetTicks();

                if (olhandoDireita)
                    framePlayer = 12;
                else
                    framePlayer = 26;
            }
        }
    }
}

void Jogo::ColisaoJogadorObstaculos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

            if (obstaculos[i].estado != OBSTACULO_CHAO)
                continue;

            if (SDL_HasRectIntersectionFloat(&player, &obstaculos[i].rect))
            {
                // Veio pela esquerda
                if (playerXAnterior + player.w <= obstaculos[i].rect.x)
                {
                    player.x = obstaculos[i].rect.x - player.w;
                }
                // Veio pela direita
                else if (playerXAnterior >= obstaculos[i].rect.x + obstaculos[i].rect.w)
                {
                    player.x = obstaculos[i].rect.x + obstaculos[i].rect.w;
                }
            }
        }
    }
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

            if (obstaculos[i].estado != OBSTACULO_CHAO)
                continue;

            if (SDL_HasRectIntersectionFloat(&player, &obstaculos[i].rect))
            {
                // Caiu em cima do obstáculo
                if (velocidadeY > 0 &&
                    player.y + player.h - velocidadeY <= obstaculos[i].rect.y)
                {
                    sujo = true;
                    sujoInicio = SDL_GetTicks();
                    speed = SPEED_SUJO;

                    // Quebra o obstáculo
                    obstaculos[i].frame = 3;
                    obstaculos[i].estado = OBSTACULO_QUEBRANDO;
                    obstaculos[i].inicioEstado = SDL_GetTicks();

                    // Aplica knockback
                    if (olhandoDireita)
                    {
                        player.x -= 35.0f * knockbackPercent / 100.0f;
                        velocidadeY = -3.5f;
                    }
                    else
                    {
                        player.x += 35.0f * knockbackPercent / 100.0f;
                        velocidadeY = -3.5f;
                    }

                    noChao = false;
                    pulando = true;
                }
            }
        }
    }
}

void Jogo::AnimarPulo() {
    if (estado == JOGAR) {
        if (aterrissando)
        {
            if (olhandoDireita)
                framePlayer = 12;
            else
                framePlayer = 26;

            if (SDL_GetTicks() - inicioAterrissagem > 200)
            {
                aterrissando = false;
            }
        }

        if (pulando)
        {
            if (velocidadeY < -2)
            {
                if (olhandoDireita)
                    framePlayer = 11;
                else
                    framePlayer = 24;
            }

        }
        // Teto
        if (player.y < 0) {
            player.y = 0;
            velocidadeY = 0.0f;

        }
    }
}

void Jogo::AtualizarTexturaPontos() {
    if (pontos != pontosAntigos)
    {
        pontosAntigos = pontos;

        char buffer[50];
        sprintf_s(buffer, sizeof(buffer), "Pontos: %d", pontos);

        SDL_Surface* surfacePontos =
            TTF_RenderText_Blended(fontHud, buffer, 0, preto);

        SDL_DestroyTexture(texturaPontos);

        texturaPontos =
            SDL_CreateTextureFromSurface(renderer, surfacePontos);

        SDL_DestroySurface(surfacePontos);

    }
}

void Jogo::SpawnarBoss() {
    if (estado == JOGAR) {
        if (!boss.ativo && pontos >= spawnBoss &&
            SDL_GetTicks() - fimUltimoBoss >= INTERVALO_MINIMO_BOSS) {
            boss.ativo = true;
            boss.tipo = SortearTipoBoss();       // sorteio a cada spawn, sem restricao por onda
            contadores.bossesEnfrentados[boss.tipo]++;
            boss.estado = BOSS_ANDANDO;
            boss.estadoInicio = SDL_GetTicks();
            boss.proximaInvocacao = SDL_GetTicks() + CHURROS_MS_PRIMEIRA;   // so o Churros usa            
            boss.investiu = false;
            boss.invocou = false;

            shakeDuracao = 600.0f;
            shakeForca = 8.0f * escalaJogo;
            shakeInicio = SDL_GetTicks();

            boss.rect.w = tamanhoBoss;
            boss.rect.h = tamanhoBoss;

            boss.rect.x = larguraTela;

            float maxYBoss = alturaChao - boss.rect.h;
            if (maxYBoss < 0) maxYBoss = 0;
            boss.rect.y = (float)(rand() % (int)(maxYBoss + 1));

            boss.speed = 1.5f + BossVelocidadeExtra(bossAtivados) * escalaJogo;

            boss.vidaMax = BossVidaMax(bossAtivados);
            boss.vida = boss.vidaMax;

            boss.frame = 0;
            boss.ultimoFrame = SDL_GetTicks();
            boss.lentidaoFim = 0;
            spawnBoss += 1000;
        }
    }
}

void Jogo::AtualizarTexturaOnda() {
    if (ondas != ondaAntiga) {

        ondaAntiga = ondas;

        char bufferOndas[50];
        sprintf_s(bufferOndas, sizeof(bufferOndas), "Ondas: %d", ondas);

        SDL_Surface* surfaceOndas =
            TTF_RenderText_Blended(fontHud, bufferOndas, 0, preto);

        SDL_DestroyTexture(TexturaOndas);

        TexturaOndas =
            SDL_CreateTextureFromSurface(renderer, surfaceOndas);

        SDL_DestroySurface(surfaceOndas);
    }
}