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

int Jogo::ItemMenuSobMouse(float x, float y) const
{
    if (MouseNoBotao(x, y, botaoJogar))    return 0;
    if (MouseNoBotao(x, y, botaoCreditos)) return 1;
    if (MouseNoBotao(x, y, botaoSair))     return 2;
    if (MouseNoBotao(x, y, abaConfig))     return 3;
    if (MouseNoBotao(x, y, abaControles))  return 4;
    if (MouseNoBotao(x, y, abaRanking))    return 5;
    if (MouseNoBotao(x, y, abaAjuda))      return 6;
    return -1;
}

const SDL_FRect* Jogo::RetanguloItemMenu(int item) const
{
    switch (item)
    {
    case 0: return &botaoJogar;
    case 1: return &botaoCreditos;
    case 2: return &botaoSair;
    case 3: return &abaConfig;
    case 4: return &abaControles;
    case 5: return &abaRanking;
    case 6: return &abaAjuda;
    }
    return nullptr;
}

// vertical: -1 sobe, +1 desce | horizontal: -1 esquerda, +1 direita
// Botões (0 a 2) ficam na coluna da esquerda; abas (3 a 6) na fileira do topo.
void Jogo::MoverItemMenu(int vertical, int horizontal)
{
    int anterior = itemMenu;

    if (itemMenu >= 3)
    {
        // Nas abas: esquerda/direita andam entre elas; para baixo volta a "Jogar"
        if (horizontal < 0 && itemMenu == 3)
            itemMenu = 0;
        else if (horizontal != 0)
            itemMenu = SDL_clamp(itemMenu + horizontal, 3, 6);
        else if (vertical > 0)
            itemMenu = 0;
    }
    else
    {
        // Nos botões: cima/baixo andam entre eles; cima em "Jogar" ou direita vai para as abas
        if (vertical < 0 && itemMenu == 0)
            itemMenu = 3;
        else if (vertical != 0)
            itemMenu = SDL_clamp(itemMenu + vertical, 0, 2);
        else if (horizontal > 0)
            itemMenu = 3;
    }

    if (itemMenu != anterior)
        TocarEfeito(itemMenu < 3 ? somHoverBotao : somHoverAbas);
}

void Jogo::ConfirmarItemMenu(int item)
{
    switch (item)
    {
    case 0: // Jogar
        TocarEfeito(somConfirm);
        estado = JOGAR;
        ReiniciarPartida();
        break;

    case 1: // Créditos
        TocarEfeito(somConfirm);
        estado = CREDITOS;
        yCreditos = (float)alturaTela;
        break;

    case 2: // Sair
        TocarEfeito(somConfirm);
        running = false;
        break;

    case 3: // Engrenagem
        TocarEfeito(somConfirm);
        estado = CONFIGURACOES;
        break;

    case 4: // Controles
        TocarEfeito(somConfirm);
        estado = CONTROLES;
        break;

    case 5: // Troféu
        TocarEfeito(somConfirm);
        estado = RANKING;
        break;

    case 6: // Ajuda
        TocarEfeito(somConfirm);
        estado = AJUDA;
        break;
    }
}

void Jogo::ConfirmarOpcaoPause()
{
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
}

void Jogo::AlternarOpcaoConfig()
{
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
}

void Jogo::ConfirmarResultado()
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

void Jogo::ProcessarEventos()
{
    while (SDL_PollEvent(&event))
    {
        // O mouse chega em coordenadas da janela; converte para as do jogo
        SDL_ConvertEventToRenderCoordinates(renderer, &event);

        // Eventos do menu: clique esquerdo confirma o item sob o cursor
        if (estado == MENU &&
            event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT)
        {
            int itemClicado = ItemMenuSobMouse(event.button.x, event.button.y);

            if (itemClicado != -1)
            {
                itemMenu = itemClicado;
                ConfirmarItemMenu(itemClicado);
                continue;
            }
        }

        // Teclado no menu: setas/WASD movem o destaque, Enter confirma
        if (estado == MENU &&
            event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat)
        {
            bool tratado = true;

            switch (event.key.key)
            {
            case SDLK_W:
            case SDLK_UP:
                MoverItemMenu(-1, 0);
                break;

            case SDLK_S:
            case SDLK_DOWN:
                MoverItemMenu(1, 0);
                break;

            case SDLK_A:
            case SDLK_LEFT:
                MoverItemMenu(0, -1);
                break;

            case SDLK_D:
            case SDLK_RIGHT:
                MoverItemMenu(0, 1);
                break;

            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                ConfirmarItemMenu(itemMenu);
                break;

            default:
                tratado = false;
                break;
            }

            // Evita que o mesmo evento seja tratado de novo pela tela seguinte
            if (tratado)
                continue;
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

        // Configurações: o mouse move o destaque e o clique esquerdo alterna a opção
        if (estado == CONFIGURACOES &&
            event.type == SDL_EVENT_MOUSE_MOTION)
        {
            for (int i = 0; i < 3; i++)
            {
                if (MouseNaOpcao(event.motion.x, event.motion.y, RetanguloOpcaoConfig(i)))
                {
                    opcaoConfig = i;
                    break;
                }
            }
        }

        if (estado == CONFIGURACOES &&
            event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT)
        {
            bool clicou = false;

            for (int i = 0; i < 3; i++)
            {
                if (MouseNaOpcao(event.button.x, event.button.y, RetanguloOpcaoConfig(i)))
                {
                    opcaoConfig = i;
                    AlternarOpcaoConfig();
                    clicou = true;
                    break;
                }
            }

            if (clicou)
                continue;
        }

        if (estado == CONFIGURACOES &&
            event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat)
        {
            switch (event.key.key)
            {
            case SDLK_ESCAPE:
                estado = MENU;
                break;

            case SDLK_W:
            case SDLK_UP:
                if (opcaoConfig > 0)
                    opcaoConfig--;
                break;

            case SDLK_S:
            case SDLK_DOWN:
                if (opcaoConfig < 2)
                    opcaoConfig++;
                break;

            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                AlternarOpcaoConfig();
                break;
            }
        }

        // Pause: o mouse move o destaque e o clique esquerdo confirma
        if (estado == PAUSE &&
            event.type == SDL_EVENT_MOUSE_MOTION)
        {
            for (int i = 0; i < 2; i++)
            {
                if (MouseNaOpcao(event.motion.x, event.motion.y, RetanguloOpcaoPause(i)))
                {
                    opcaoPause = i;
                    break;
                }
            }
        }

        if (estado == PAUSE &&
            event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT)
        {
            bool clicou = false;

            for (int i = 0; i < 2; i++)
            {
                if (MouseNaOpcao(event.button.x, event.button.y, RetanguloOpcaoPause(i)))
                {
                    opcaoPause = i;
                    ConfirmarOpcaoPause();
                    clicou = true;
                    break;
                }
            }

            // Evita que o mesmo clique vire tiro ao voltar para o jogo
            if (clicou)
                continue;
        }

        if (estado == PAUSE &&
            event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat)
        {
            switch (event.key.key)
            {
            case SDLK_W:
            case SDLK_UP:
                if (opcaoPause > 0)
                    opcaoPause--;
                break;

            case SDLK_S:
            case SDLK_DOWN:
                if (opcaoPause < 1)
                    opcaoPause++;
                break;

            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                ConfirmarOpcaoPause();
                break;
            }
        }

        if (estado == UPGRADE &&
            event.type == SDL_EVENT_KEY_DOWN)
        {
            switch (event.key.key)
            {
            case SDLK_A: if (opcaoUpgrade > 0) { opcaoUpgrade--; } break;
            case SDLK_D: if (opcaoUpgrade < totalOfertaCartas - 1) { opcaoUpgrade++; } break;

            case SDLK_RETURN:
            {
                if (opcaoUpgrade >= 0 && opcaoUpgrade < totalOfertaCartas)
                    EscolherCarta(ofertaCartas[opcaoUpgrade]);

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
        // Roda do mouse rola os créditos
        if (estado == CREDITOS &&
            event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            yCreditos += event.wheel.y * 40.0f;

            if (yCreditos > (float)alturaTela)
                yCreditos = (float)alturaTela;
        }

        if (estado == RESULTADO &&
            event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat)
        {
            if (event.key.key == SDLK_RETURN ||
                event.key.key == SDLK_KP_ENTER)
            {
                ConfirmarResultado();
            }
        }

        // Clique esquerdo no botão "Continuar" (com uma trava curta para o
        // clique de quem estava atirando até o fim não pular a tela)
        if (estado == RESULTADO &&
            event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT &&
            SDL_GetTicks() - inicioResultado >= 400 &&
            MouseNaOpcao(event.button.x, event.button.y, RetanguloBotaoResultado()))
        {
            ConfirmarResultado();
            continue;
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
            boss.speed = 1.5f + BossVelocidadeExtra(bossAtivados) * escalaJogo;

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

                            projeteis[i].rect.w = tamanhoProjetil;
                            projeteis[i].rect.h = tamanhoProjetil;

                            // Nasce no centro do jogador, deslocado na direção do tiro
                            float origemX = centroPlayerX + dirX * (player.w / 2);
                            float origemY = centroPlayerY + dirY * (player.w / 2)
                                + (tiroDuplo ? (criados == 0 ? -12.0f * escalaJogo : 12.0f * escalaJogo) : 0.0f);

                            projeteis[i].rect.x = origemX - projeteis[i].rect.w / 2;
                            projeteis[i].rect.y = origemY - projeteis[i].rect.h / 2;

                            // O sprite aponta para a direita (0 graus)
                            projeteis[i].angulo = SDL_atan2f(dirY, dirX) * 57.29578f;

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