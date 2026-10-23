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

void Jogo::AtualizarHoverMenu() 
{
    if (estado == MENU)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int novoHoverBotao = -1;

        if (MouseNoBotao(mouseX, mouseY, botaoJogar))
            novoHoverBotao = 0;
        else if (MouseNoBotao(mouseX, mouseY, botaoCreditos))
            novoHoverBotao = 1;
        else if (MouseNoBotao(mouseX, mouseY, botaoSair))
            novoHoverBotao = 2;

        if (novoHoverBotao != hoverBotaoAtual &&
            novoHoverBotao != -1)
        {
            TocarEfeito(somHoverBotao);
            itemMenu = novoHoverBotao; // 0 a 2 coincidem com os botões
        }

        hoverBotaoAtual = novoHoverBotao;

        int novoHoverAba = -1;

        if (MouseNoBotao(mouseX, mouseY, abaRanking))
            novoHoverAba = 0;
        else if (MouseNoBotao(mouseX, mouseY, abaConfig))
            novoHoverAba = 1;
        else if (MouseNoBotao(mouseX, mouseY, abaControles))
            novoHoverAba = 2;
        else if (MouseNoBotao(mouseX, mouseY, abaAjuda))
            novoHoverAba = 3;

        if (novoHoverAba != hoverAbaAtual &&
            novoHoverAba != -1)
        {
            TocarEfeito(somHoverAbas);

            // Ranking, Engrenagem, Controles, Ajuda -> itens do menu
            const int itemPorAba[4] = { 5, 3, 4, 6 };
            itemMenu = itemPorAba[novoHoverAba];
        }

        hoverAbaAtual = novoHoverAba;
    }
}

void Jogo::GerenciarMusica()
{
    if (estado == MENU)
    {
        TocarMusica(musicaMenu,
            "assets/menu.wav");

        AtualizarMusica(musicaMenu);
    }
    else
    {
        PararMusica(musicaMenu);
    }

    // Musica da Gameplay
    if (estado == JOGAR && !boss.ativo) {
        TocarMusica(musicaGame, "assets/jogar.wav");
        AtualizarMusica(musicaGame);
    }
    else { PararMusica(musicaGame); }

    // Musica do Boss
    if (estado == JOGAR && boss.ativo) {
        TocarMusica(musicaBoss, "assets/boss.wav");
        AtualizarMusica(musicaBoss);
    }
    else { PararMusica(musicaBoss); }

    // Musica da tela de gameover
    if (estado == GAMEOVER)
    {
        TocarMusica(musicaGameOver,
            "assets/gameover.wav");

        AtualizarMusica(musicaGameOver);
    }
    else
    {
        PararMusica(musicaGameOver);
    }

    // Musica dos creditos
    if (estado == CREDITOS)
    {
        TocarMusica(musicaCreditos,
            "assets/creditos.wav");

        AtualizarMusica(musicaCreditos);
    }
    else
    {
        PararMusica(musicaCreditos);
    }
}

void Jogo::AtualizarAnimacoes()
{
    if (estado == JOGAR) {

        if (SDL_GetTicks() - ultimoFrameBase > 150)
        {
            frameBase++;

            if (frameBase >= totalFramesBase)
                frameBase = 0;

            ultimoFrameBase = SDL_GetTicks();
        }
    }

    // Atualização do flash da base
    if (estado == JOGAR) {

        if (tempoFlashBase > 0.0f)
        {
            tempoFlashBase -= 1.0f / 60.0f;

            if (tempoFlashBase <= 0.0f)
            {
                tempoFlashBase = 0.0f;
                flashBase = FLASH_NENHUM;
            }
        }
    }

    // Atualização da animação do PowerUp de Vida
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            if (!vidasPowerup[i].ativo)
                continue;

            if (SDL_GetTicks() - vidasPowerup[i].ultimoFrame > 120)
            {
                vidasPowerup[i].frame++;

                if (vidasPowerup[i].frame >= totalFramesPowerUp)
                    vidasPowerup[i].frame = 0;

                vidasPowerup[i].ultimoFrame = SDL_GetTicks();
            }
        }
    }
    // Atualização da animação do PowerUp de Ataque

    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (!ataquePowerup[i].ativo)
                continue;

            if (SDL_GetTicks() - ataquePowerup[i].ultimoFrame > 120)
            {
                ataquePowerup[i].frame++;

                if (ataquePowerup[i].frame >= totalFramesPowerUp)
                    ataquePowerup[i].frame = 0;

                ataquePowerup[i].ultimoFrame = SDL_GetTicks();
            }
        }
    }
}

void Jogo::AtualizarOndasEBuffs()
{
    if (estado == JOGAR && SDL_GetTicks() - inicioOnda >= 30000)
    {
        ondas++;

        inicioOnda = SDL_GetTicks();

        spam = true;
        inicioSpam = SDL_GetTicks();
        inimigosAtivos += 2;

        if (inimigosAtivos > MAX_INIMIGOS)
            inimigosAtivos = MAX_INIMIGOS;

        if (intervaloSpawn > 150)
            intervaloSpawn -= 150;
        else
            intervaloSpawn = 500;

        if (intervaloSpawn < 500)
            intervaloSpawn = 500;

        if (inimigosAtivos > MAX_INIMIGOS)
        {
            inimigosAtivos = MAX_INIMIGOS;
        }
    }
}

void Jogo::AtualizarSpawn() {
    if (estado == JOGAR) {
        int vivos = 0;

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                vivos++;
            }
        }

        // Spawn automático
        tempoAtual = SDL_GetTicks();

        if (!boss.ativo && tempoAtual - ultimoSpawn >= (Uint64)intervaloSpawn && vivos < inimigosAtivos)
        {
            ultimoSpawn = tempoAtual;

            int quantidadeSpawn = 1;

            if (ondas >= 10) quantidadeSpawn = 2;
            if (ondas >= 20) quantidadeSpawn = 3;
            if (ondas >= 30) quantidadeSpawn = 4;

            for (int s = 0; s < quantidadeSpawn; s++)
            {
                for (int i = 0; i < MAX_INIMIGOS; i++)
                {
                    if (!inimigos[i].ativo)
                    {
                        inimigos[i].ativo = true;
                        inimigos[i].frame = 0;
                        inimigos[i].ultimoFrame = SDL_GetTicks();

                        inimigos[i].rect.w = tamanhoInimigo;
                        inimigos[i].rect.h = tamanhoInimigo;

                        inimigos[i].speed = (2.0f + (ondas * 0.2f)) * escalaJogo;

                        if (inimigos[i].speed > 6.0f * escalaJogo)
                            inimigos[i].speed = 6.0f * escalaJogo;

                        inimigos[i].rect.x = larguraTela + (s * 70) + (rand() % 20);

                        inimigos[i].rect.y =
                            (rand() % (alturaTela - (int)inimigos[i].rect.h));

                        if (spam)
                            inimigos[i].speed += 0.5f;

                        break;
                    }
                }
            }
        }
        // Spawn de powerups

        if (tempoAtual - ultimoSpawnPowerUp >= intervaloSpawnPowerUp && (vidas < vidaMaxima)) {
            ultimoSpawnPowerUp = tempoAtual;

            for (int i = 0; i < MAX_POWERUPS; i++) {
                if (!vidasPowerup[i].ativo) {
                    vidasPowerup[i].ativo = true;
                    vidasPowerup[i].frame = 0;
                    vidasPowerup[i].ultimoFrame = SDL_GetTicks();

                    vidasPowerup[i].rect.w = tamanhoPowerup;

                    vidasPowerup[i].rect.h = tamanhoPowerup;

                    vidasPowerup[i].rect.x = larguraTela;

                    vidasPowerup[i].rect.y =
                        rand() % (alturaTela - (int)vidasPowerup[i].rect.h);

                    vidasPowerup[i].speed = 7.0f * escalaJogo;

                    break;
                }
            }
        }

        // SPAWN DOS OBSTÁCULOS

        if (ondas >= 2 &&
            !boss.ativo &&
            SDL_GetTicks() - ultimoSpawnObstaculo >= intervaloObstaculo)
        {
            ultimoSpawnObstaculo = SDL_GetTicks();

            for (int i = 0; i < MAX_OBSTACULOS; i++)
            {
                if (!obstaculos[i].ativo)
                {
                    obstaculos[i].ativo = true;

                    obstaculos[i].estado = OBSTACULO_SOMBRA;

                    obstaculos[i].frame = 0;
                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                    obstaculos[i].inicioEstado = SDL_GetTicks();

                    obstaculos[i].velocidadeY = 4.0f * escalaJogo;

                    obstaculos[i].rect.w = tamanhoObstaculo;
                    obstaculos[i].rect.h = tamanhoObstaculo;

                    // posição aleatória (à direita da base, nunca dentro dela)
                    int faixaSpawn = larguraTela - (int)base.w - (int)obstaculos[i].rect.w;
                    if (faixaSpawn < 1)
                        faixaSpawn = 1;
                    obstaculos[i].rect.x = base.w + (float)(rand() % faixaSpawn);

                    // começa acima da tela
                    obstaculos[i].rect.y = -obstaculos[i].rect.h;

                    break;
                }
            }
        }

        int buffsAtivos = 0;

        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (ataquePowerup[i].ativo)
                buffsAtivos++;
        }

        if (tempoAtual - ultimoSpawnAtq >= intervaloSpawnAtq &&
            buffsAtivos == 0)
        {
            ultimoSpawnAtq = tempoAtual;

            intervaloSpawnAtq =
                20000 + rand() % 30000;

            ataquePowerup[0].ativo = true;
            ataquePowerup[0].frame = 0;
            ataquePowerup[0].ultimoFrame = SDL_GetTicks();

            ataquePowerup[0].rect.w = tamanhoPowerup;
            ataquePowerup[0].rect.h = tamanhoPowerup;

            ataquePowerup[0].rect.x = larguraTela;
            ataquePowerup[0].rect.y =
                rand() % (alturaTela - (int)ataquePowerup[0].rect.h);

            ataquePowerup[0].speed = 7.0f * escalaJogo;
        }
    }
}

void Jogo::AtualizarProjeteis() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_PROJETEIS; i++)
        {
            if (!projeteis[i].ativo)
                continue;

            // Se não estiver explodindo, movimenta
            if (!projeteis[i].explodindo)
            {
                projeteis[i].rect.x += projeteis[i].velX;
                projeteis[i].rect.y += projeteis[i].velY;

                // Frames de voo (0,1,2)
                if (SDL_GetTicks() - projeteis[i].ultimoFrame > 80)
                {
                    projeteis[i].frame++;

                    if (projeteis[i].frame > 2)
                        projeteis[i].frame = 0;

                    projeteis[i].ultimoFrame = SDL_GetTicks();
                }
            }
            else
            {
                // Frames de explosão (3,4)
                if (SDL_GetTicks() - projeteis[i].ultimoFrame > 100)
                {
                    projeteis[i].frame++;

                    if (projeteis[i].frame > 4)
                    {
                        projeteis[i].ativo = false;
                    }

                    projeteis[i].ultimoFrame = SDL_GetTicks();
                }
            }

            if (projeteis[i].rect.x < 0 ||
                projeteis[i].rect.x > larguraTela ||
                projeteis[i].rect.y < 0 ||
                projeteis[i].rect.y > alturaTela)
            {
                projeteis[i].ativo = false;
            }
        }
    }
}

void Jogo::AtualizarInimigos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].morrendo)
            {
                if (SDL_GetTicks() - inimigos[i].flashInicio >= 120)
                {
                    inimigos[i].morrendo = false;
                    inimigos[i].ativo = false;
                }
                continue;
            }

            if (inimigos[i].ativo)
            {
                if (SDL_GetTicks() - inimigos[i].ultimoFrame > 150)
                {
                    inimigos[i].frame++;

                    if (inimigos[i].frame >= totalFramesInimigo)
                    {
                        inimigos[i].frame = 0;
                    }

                    inimigos[i].ultimoFrame = SDL_GetTicks();
                }
                inimigos[i].rect.x -= inimigos[i].speed;

                if (inimigos[i].rect.x + inimigos[i].rect.w < 0)
                {
                    inimigos[i].ativo = false;
                }
            }
        }
    }
}

void Jogo::AtualizarPowerups() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            if (!vidasPowerup[i].ativo)
                continue;

            vidasPowerup[i].rect.x -= vidasPowerup[i].speed;

            if (vidasPowerup[i].rect.x +
                vidasPowerup[i].rect.w < 0)
            {
                vidasPowerup[i].ativo = false;
            }
        }
    }

    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (!ataquePowerup[i].ativo)
                continue;

            ataquePowerup[i].rect.x -= ataquePowerup[i].speed;

            if (ataquePowerup[i].rect.x +
                ataquePowerup[i].rect.w < 0)
            {
                ataquePowerup[i].ativo = false;
            }
        }
    }
}

void Jogo::AtualizarBoss (){
    if (estado == JOGAR) {
            if (boss.ativo)
            {
                boss.rect.x -= boss.speed;

                if (SDL_GetTicks() - boss.ultimoFrame > 120)
                {
                    boss.frame++;

                    if (boss.frame >= 6)
                        boss.frame = 0;

                    boss.ultimoFrame = SDL_GetTicks();
                }
            }
    }
}

void Jogo::AtualizarObstaculos()
{
    if (estado == JOGAR) {

        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

            switch (obstaculos[i].estado)
            {
            case OBSTACULO_SOMBRA:

                if (SDL_GetTicks() - obstaculos[i].inicioEstado >= 1000)
                {
                    obstaculos[i].estado = OBSTACULO_CAINDO;
                    obstaculos[i].inicioEstado = SDL_GetTicks();
                }

                break;

            case OBSTACULO_CAINDO:

                obstaculos[i].rect.y += obstaculos[i].velocidadeY;

                // Animação
                if (SDL_GetTicks() - obstaculos[i].ultimoFrame > 120)
                {
                    if (obstaculos[i].frame < 2)
                        obstaculos[i].frame++;

                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                }

                // Colisão com o player
                if (SDL_HasRectIntersectionFloat(&player, &obstaculos[i].rect))
                {

                    if (player.x + player.w / 2 <
                        obstaculos[i].rect.x + obstaculos[i].rect.w / 2)
                        player.x = obstaculos[i].rect.x - player.w;
                    else
                        player.x = obstaculos[i].rect.x + obstaculos[i].rect.w;

                    stun = true;
                    stunStart = SDL_GetTicks();

                    dano = true;
                    danoStart = SDL_GetTicks();

                    if (olhandoDireita)
                        framePlayer = 13;
                    else
                        framePlayer = 27;

                    obstaculos[i].rect.y = alturaChao;

                    obstaculos[i].estado = OBSTACULO_CHAO;

                    obstaculos[i].frame = 3;
                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                    obstaculos[i].inicioEstado = SDL_GetTicks();
                }

                // Tocou no chão
                if (obstaculos[i].rect.y >= alturaChao)
                {
                    obstaculos[i].rect.y = alturaChao;

                    obstaculos[i].estado = OBSTACULO_CHAO;

                    obstaculos[i].frame = 3;
                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                    obstaculos[i].inicioEstado = SDL_GetTicks();
                }

                break;

            case OBSTACULO_CHAO:
            {
                Uint64 tempoNoChao = SDL_GetTicks() - obstaculos[i].inicioEstado;

                if (tempoNoChao < 8000)
                {
                    obstaculos[i].frame = 4; // parado normal
                }
                else if (tempoNoChao < 8700)
                {
                    obstaculos[i].frame = 3; // frame quebrando
                }
                else
                {
                    obstaculos[i].ativo = false;
                    TocarEfeito(somQuebra);
                    continue;
                }
                break;
            }

            case OBSTACULO_QUEBRANDO:

                obstaculos[i].frame = 3;

                if (SDL_GetTicks() - obstaculos[i].inicioEstado >= 300)
                {
                    obstaculos[i].ativo = false;
                    TocarEfeito(somQuebra);
                }

                break;
            }
        }
    }
}