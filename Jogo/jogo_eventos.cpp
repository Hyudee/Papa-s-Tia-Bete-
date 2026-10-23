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

void Jogo::ProcessarEventos()
{
    while (SDL_PollEvent(&event))
    {

        // Eventos do menu
        if (estado == MENU &&
            event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            float mouseX = event.button.x;
            float mouseY = event.button.y;


            if (MouseNoBotao(mouseX, mouseY, botaoJogar))
            {

                TocarEfeito(somConfirm);
                estado = JOGAR;
                TocarEfeito(somConfirm);
                estado = JOGAR;
                ReiniciarPartida();
                continue;
            }

            if (MouseNoBotao(mouseX, mouseY, botaoCreditos))
            {
                TocarEfeito(somConfirm);
                estado = CREDITOS;
                yCreditos = alturaTela;
            }

            if (MouseNoBotao(mouseX, mouseY, botaoSair))
            {
                TocarEfeito(somConfirm);
                running = false;
            }


            // Troféu
            if (MouseNoBotao(mouseX, mouseY, abaRanking))
            {
                TocarEfeito(somConfirm);
                estado = RANKING;
            }

            // Engrenagem
            if (MouseNoBotao(mouseX, mouseY, abaConfig))
            {
                TocarEfeito(somConfirm);
                estado = CONFIGURACOES;
            }

            // Controles
            if (MouseNoBotao(mouseX, mouseY, abaControles))
            {
                TocarEfeito(somConfirm);
                estado = CONTROLES;
            }

            // Ajuda
            if (MouseNoBotao(mouseX, mouseY, abaAjuda))
            {
                TocarEfeito(somConfirm);
                estado = AJUDA;
            }
        }

        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }

        if (estado == DIGITAR_NOME &&
            event.type == SDL_EVENT_TEXT_INPUT)
        {
            size_t len = strlen(nomeJogador);
            size_t add = strlen(event.text.text);

            if (len + add < sizeof(nomeJogador))
            {
                strcat_s(
                    nomeJogador,
                    sizeof(nomeJogador),
                    event.text.text
                );

                tamanhoNome = strlen(nomeJogador);
            }
        }

        if (estado == DIGITAR_NOME &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_BACKSPACE &&
                tamanhoNome > 0)
            {
                nomeJogador[--tamanhoNome] = '\0';
            }

            if (event.key.key == SDLK_RETURN &&
                tamanhoNome > 0)
            {
                AdicionarPontuacao(
                    ranking,
                    nomeJogador,
                    pontos
                );

                SalvarRanking(ranking);

                SDL_StopTextInput(window);

                estado = MENU;
                tempoAtual = SDL_GetTicks();
            }
        }

        // Funcionamento do esc

        if (estado == JOGAR &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                inicioPausa = SDL_GetTicks();
                estado = PAUSE;
            }
        }

        if (estado == RANKING &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                estado = MENU;
            }
        }

        if (estado == CONFIGURACOES &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            switch (event.key.key)
            {
            case SDLK_ESCAPE:
                estado = MENU;
                break;

            case SDLK_W:
                if (opcaoConfig > 0)
                    opcaoConfig--;
                break;

            case SDLK_S:
                if (opcaoConfig < 2)
                    opcaoConfig++;
                break;

            case SDLK_RETURN:
            case SDLK_KP_ENTER:

                if (opcaoConfig == 0)
                {
                    telaCheia = !telaCheia;

                    SDL_SetWindowFullscreen(
                        window,
                        telaCheia
                    );
                }
                else if (opcaoConfig == 1)
                {
                    musicaLigada = !musicaLigada;

                    if (!musicaLigada)
                    {
                        PararMusica(musicaMenu);
                        PararMusica(musicaGame);
                        PararMusica(musicaBoss);
                        PararMusica(musicaCreditos);
                        PararMusica(musicaGameOver);
                    }
                }
                else if (opcaoConfig == 2)
                {
                    efeitosLigados = !efeitosLigados;
                }

                break;
            }
        }

        if (estado == PAUSE &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            switch (event.key.key)
            {
            case SDLK_W:
                if (opcaoPause > 0)
                    opcaoPause--;
                break;

            case SDLK_S:
                if (opcaoPause < 1)
                    opcaoPause++;
                break;

            case SDLK_RETURN:

                switch (opcaoPause)
                {
                case 0: // Continuar
                    DeslocarTimers(SDL_GetTicks() - inicioPausa);
                    estado = JOGAR;
                    break;

                case 1: // Menu Principal
                    estado = MENU;
                    break;
                }

                break;
            }
        }

        if (estado == UPGRADE &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            int maxOpcao = 2
                + (upgradeTiroDuploDisponivel ? 1 : 0)
                + (upgradeCacadordeBossDisponivel ? 1 : 0);

            switch (event.key.key)
            {
            case SDLK_A: if (opcaoUpgrade > 0) { opcaoUpgrade--; } break;
            case SDLK_D: if (opcaoUpgrade < maxOpcao) { opcaoUpgrade++; } break;

            case SDLK_RETURN:
            {
                int idxDano = 0;
                int idxVida = 1;
                int idxBalas = 2;
                int idxDuplo = upgradeTiroDuploDisponivel ? 3 : -1;
                int idxCacador = upgradeCacadordeBossDisponivel
                    ? 3 + (upgradeTiroDuploDisponivel ? 1 : 0)
                    : -1;

                if (opcaoUpgrade == idxDano)
                {
                    danoTiroBase += 1;
                    danoTiro = ataqueBuff ? danoTiroBase * 2 : danoTiroBase;
                }
                else if (opcaoUpgrade == idxVida)
                {
                    vidaMaxima++;
                    vidas = vidaMaxima;
                }
                else if (opcaoUpgrade == idxBalas)
                {
                    if (quantidadeTiros < MAX_PROJETEIS)
                    {
                        tirosExtras++;
                        quantidadeTiros = 3 + tirosExtras;
                        if (quantidadeTiros > MAX_PROJETEIS)
                            quantidadeTiros = MAX_PROJETEIS;
                    }
                }
                else if (idxDuplo != -1 && opcaoUpgrade == idxDuplo)
                {
                    tiroDuplo = true;
                }
                else if (idxCacador != -1 && opcaoUpgrade == idxCacador)
                {
                    cacadorBoss = true;
                    danoTiroBase += 5;
                    danoTiro = ataqueBuff ? danoTiroBase * 2 : danoTiroBase;
                }

                DeslocarTimers(SDL_GetTicks() - inicioPausa);
                estado = JOGAR;
                break;
            }
            }
        }

        if (estado == CONTROLES &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                estado = MENU;
            }
        }

        if (estado == AJUDA &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                estado = MENU;
            }
        }
        if (estado == CREDITOS &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                estado = MENU;
            }
        }
        if (estado == RESULTADO && event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_RETURN)
            {
                if (pontos > ranking[2].pontos)
                {
                    estado = DIGITAR_NOME;
                    nomeJogador[0] = '\0';
                    tamanhoNome = 0;
                    SDL_StartTextInput(window);
                }
                else
                {
                    ReiniciarPartida();
                    estado = MENU;
                }
            }
        }

        // Redimensionamento da janela
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            SDL_GetWindowSize(window, &larguraTela, &alturaTela);

            AtualizarEscala(larguraTela, alturaTela);

            // Chão

            alturaChao = alturaTela - player.h;

            // Player
            player.w = tamanhoPlayer;
            player.h = tamanhoPlayer;

            // Boss
            boss.rect.w = tamanhoBoss;
            boss.rect.h = tamanhoBoss;
            boss.speed = 1.5f + (bossAtivados * 0.25f) * escalaJogo;

            // Obstáculos
            for (int i = 0; i < MAX_OBSTACULOS; i++)
            {
                if (obstaculos[i].ativo)
                {
                    obstaculos[i].rect.w = tamanhoObstaculo;
                    obstaculos[i].rect.h = tamanhoObstaculo;
                }
            }

            // Inimigos
            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (inimigos[i].ativo)
                {
                    inimigos[i].rect.w = tamanhoInimigo;
                    inimigos[i].rect.h = tamanhoInimigo;
                    inimigos[i].rect.x -= inimigos[i].speed;

                }
            }

            // Projéteis
            for (int i = 0; i < MAX_PROJETEIS; i++)
            {
                if (projeteis[i].ativo)
                {
                    projeteis[i].rect.w = tamanhoProjetil;
                    projeteis[i].rect.h = tamanhoProjetil;
                }
            }

            tileW = (float)larguraTela / COLUNAS;
            tileH = (float)alturaTela / LINHAS;

            alturaChao = alturaTela - player.h;

            logoRect.w = larguraTela * 0.35f;
            logoRect.h = logoRect.w * 0.5f;

            logoRect.x = larguraTela * 0.0678f;
            logoRect.y = alturaTela * 0.05f;

            botaoJogar.w = larguraTela * 0.25f;
            botaoJogar.h = alturaTela * 0.10f;
            botaoJogar.x = larguraTela * 0.12f;
            botaoJogar.y = alturaTela * 0.45f;

            botaoCreditos.w = botaoJogar.w;
            botaoCreditos.h = botaoJogar.h;
            botaoCreditos.x = botaoJogar.x;
            botaoCreditos.y = botaoJogar.y + botaoJogar.h + 20;

            botaoSair.w = botaoJogar.w;
            botaoSair.h = botaoJogar.h;
            botaoSair.x = botaoJogar.x;
            botaoSair.y = botaoCreditos.y + botaoCreditos.h + 20;

            larguraAba = larguraTela * 0.03f;
            alturaAba = alturaTela * 0.18f;
            padding = larguraTela * 0.005f;

            abaConfig.w = abaControles.w = abaRanking.w = abaAjuda.w = larguraAba;
            abaConfig.h = abaControles.h = abaRanking.h = abaAjuda.h = alturaAba;

            abaConfig.y = 0;
            abaControles.y = 0;
            abaRanking.y = 0;
            abaAjuda.y = 0;

            abaConfig.x = larguraTela * 0.74f;
            abaControles.x = abaConfig.x + larguraAba + padding;
            abaRanking.x = abaControles.x + larguraAba + padding;
            abaAjuda.x = abaRanking.x + larguraAba + padding;

            base.h = (float)alturaTela;
        }

        // Atirar
        if (estado == JOGAR && !stun && !dano && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                float mouseX = event.button.x;
                float mouseY = event.button.y;

                float centroPlayerX = player.x + player.w / 2;

                if (mouseX < centroPlayerX)
                    olhandoDireita = false;
                else
                    olhandoDireita = true;

                float centroPlayerY = player.y + player.h / 2;

                float dirX = mouseX - centroPlayerX;
                float dirY = mouseY - centroPlayerY;

                float tamanho =
                    SDL_sqrtf(dirX * dirX + dirY * dirY);

                if (tamanho > 0.0f)
                {
                    dirX /= tamanho;
                    dirY /= tamanho;
                }

                atirando = true;

                if (olhandoDireita)
                    framePlayer = 2;
                else
                    framePlayer = 6;

                ultimoFramePlayer = SDL_GetTicks();

                int projeteisAtivos = 0;
                for (int i = 0; i < MAX_PROJETEIS; i++)
                {
                    if (projeteis[i].ativo)
                        projeteisAtivos++;
                }

                int tirosCriar = tiroDuplo ? 2 : 1;

                // Conta slots livres
                int slotsLivres = 0;
                for (int i = 0; i < MAX_PROJETEIS; i++)
                    if (!projeteis[i].ativo) slotsLivres++;

                // Conta disparos ativos (pares se duplo)
                int disparosAtivos = 0;
                for (int i = 0; i < MAX_PROJETEIS; i++)
                    if (projeteis[i].ativo) disparosAtivos++;
                int disparosAtivosCont = tiroDuplo ? (disparosAtivos / 2) : disparosAtivos;

                if (slotsLivres >= tirosCriar && disparosAtivosCont < quantidadeTiros)
                {
                    int criados = 0;
                    for (int i = 0; i < MAX_PROJETEIS && criados < tirosCriar; i++)
                    {
                        if (!projeteis[i].ativo)
                        {
                            projeteis[i].ativo = true;

                            if (criados == 0)
                                TocarEfeito(somTiro);

                            projeteis[i].frame = 0;
                            projeteis[i].explodindo = false;
                            projeteis[i].ultimoFrame = SDL_GetTicks();

                            projeteis[i].rect.x = player.x + player.w;
                            projeteis[i].rect.y = player.y + (tiroDuplo ? (criados == 0 ? -12.0f * escalaJogo : 12.0f * escalaJogo) : 0.0f);
                            projeteis[i].rect.w = tamanhoProjetil;
                            projeteis[i].rect.h = tamanhoProjetil;

                            float velocidade = 12.0f * escalaJogo;
                            projeteis[i].velX = dirX * velocidade;
                            projeteis[i].velY = dirY * velocidade;

                            criados++;
                        }
                    }
                }

            }
        }
    }

}