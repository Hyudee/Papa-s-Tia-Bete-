// ============================================================
// FUNCIONALIDADES
// ============================================================
// Movimentação do player com WASD e espaço para pular - ok
// Combate a distancia com projeteis - ok
// Menu de iniciar jogo, sair e acessar rankings - em dev
// criação de mapa 2d com tiles
// sistema de pontuação
// sistema de ordas - ok
// spawn de inimigos - ok
// sistema de vida - ok
// animação de sprites

// ============================================================
// PARA FAZER
// ============================================================
/*
Desenhar e incluir:

Coração de vida
Personagem com spritesheet
Inimigos com spritesheet
Base com spritesheet
Boss com spritesheet
Sons
Efeitos de se bater nos inimigos (pensei em ele ser empurrado e perder pontos, ou um stun, sla)
*/

// ============================================================
// INCLUDES
// ============================================================

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

// ============================================================
// DEFINES
// ============================================================

#define COLUNAS      25
#define LINHAS       19
#define MAX_PROJETEIS 5
#define MAX_INIMIGOS  50
#define MAX_VIDAS     3

// ============================================================
// TIPOS E STRUCTS
// ============================================================

typedef enum {
    MENU,
    JOGAR,
    RANKING,
    GAMEOVER,
    SAIR
} Sistema;

typedef struct
{
    SDL_FRect rect;
    bool      ativo;
    float     speed;
} Inimigo;

typedef struct
{
    SDL_FRect rect;
    bool      ativo;
    float     velX;
    float     velY;

    int frame;
    Uint64 ultimoFrame;
    bool explodindo;
} Projetil;

// ============================================================
// MAIN
// ============================================================

int main(int argc, char* argv[])
{
    // ----------------------------------------------------------
    // INICIALIZAÇÃO DO SDL
    // ----------------------------------------------------------

    SDL_Log("Inicializando o jogo...");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("Falha ao inicializar SDL: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Jogo",
        840,
        480,
        SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        SDL_Log("Falha ao criar a janela: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer)
    {
        SDL_Log("Falha ao criar renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if (!TTF_Init()) {
        SDL_Log("Falha ao inicializar a ttf: %s", SDL_GetError());
        return -1;
    }

    // Fonte do jogo

    TTF_Font* font = TTF_OpenFont("assets/PixelifySans.ttf", 32);
    TTF_Font* fontHud = TTF_OpenFont("assets/PixelifySans.ttf", 18);

    if (!font)
    {
        SDL_Log("Erro ao carregar fonte");
    }


    // Textos da tela

    SDL_Color preto = { 0, 0, 0, 255 };

    // ----------------------------------------------------------
    // ADAPTAR A TELA
    // ----------------------------------------------------------

    int larguraTela, alturaTela;
    SDL_GetWindowSize(window, &larguraTela, &alturaTela);

    float tileW = (float)larguraTela / COLUNAS;
    float tileH = (float)alturaTela / LINHAS;

    // ----------------------------------------------------------
    // ESTADO DO SISTEMA / MENU
    // ----------------------------------------------------------

    Sistema estado = MENU;

    SDL_FRect botaoJogar;
    SDL_FRect botaoRanking;
    SDL_FRect botaoSair;
    SDL_FRect logoRect;

    float textoW, textoH;
    SDL_FRect textoRect;

    logoRect.w = larguraTela * 0.35f; 
    logoRect.h = logoRect.w * 0.5f; 

    logoRect.x = larguraTela * 0.05f;
    logoRect.y = alturaTela * 0.05f;

    botaoJogar.w = larguraTela * 0.25f;
    botaoJogar.h = alturaTela * 0.10f;
    botaoJogar.x = larguraTela * 0.12f;
    botaoJogar.y = alturaTela * 0.45f;

    botaoRanking.w = botaoJogar.w;
    botaoRanking.h = botaoJogar.h;
    botaoRanking.x = botaoJogar.x;
    botaoRanking.y = botaoJogar.y + botaoJogar.h + 20;

    botaoSair.w = botaoJogar.w;
    botaoSair.h = botaoJogar.h;
    botaoSair.x = botaoJogar.x;
    botaoSair.y = botaoRanking.y + botaoRanking.h + 20;

    srand((unsigned)time(NULL));

    // ----------------------------------------------------------
    // CARREGAMENTO DOS TILESETS
    // ----------------------------------------------------------

    SDL_Texture* background =
        IMG_LoadTexture(renderer, "assets/cozinha.png");

    if (!background)
    {
        SDL_Log("Erro ao carregar background: %s", SDL_GetError());
    }

    SDL_Texture* menu =
        IMG_LoadTexture(renderer, "assets/menu.jpeg");

    if (!menu)
    {
        SDL_Log("Erro ao carregar menu: %s", SDL_GetError());
    }

    SDL_Texture* logo =
        IMG_LoadTexture(renderer, "assets/logo.png");

    SDL_Texture* jogar =
        IMG_LoadTexture(renderer, "assets/jogar.png");

    if (!jogar)
    {
        SDL_Log("Erro ao carregar Botão de jogar: %s", SDL_GetError());
    }
    SDL_Texture* creditos =
        IMG_LoadTexture(renderer, "assets/creditos.png");

    if (!creditos)
    {
        SDL_Log("Erro ao carregar Botão de creditos: %s", SDL_GetError());
    }
    SDL_Texture* sair =
        IMG_LoadTexture(renderer, "assets/sair.png");

    if (!sair)
    {
        SDL_Log("Erro ao carregar Botão de sair: %s", SDL_GetError());
    }
    SDL_Texture* texturaInimigo =
        IMG_LoadTexture(renderer, "assets/Acucar.png");

    if (!texturaInimigo)
    {
        SDL_Log("Erro ao carregar inimigos: %s", SDL_GetError());
    }
    SDL_Texture* projetil =
        IMG_LoadTexture(renderer, "assets/tiro.png");

    if (!projetil)
    {
        SDL_Log("Erro ao carregar projeteis: %s", SDL_GetError());
    }
    SDL_Texture* gameover =
        IMG_LoadTexture(renderer, "assets/gameover.png");

    if (!gameover)
    {
        SDL_Log("Erro ao carregar tela de Game over: %s", SDL_GetError());
    }
    SDL_Texture* coracao =
        IMG_LoadTexture(renderer, "assets/coracao.png");

    if (!coracao)
    {
        SDL_Log("Erro ao carregar corações de vida: %s", SDL_GetError());
    }
    




    // ----------------------------------------------------------
    // VARIÁVEIS DO JOGADOR
    // ----------------------------------------------------------

    SDL_FRect player = {
        320.0f,
        240.0f,
        32.0f,
        32.0f
    };

    float speed = 5.0f;
    float velocidadeY = 0.0f;
    float gravidade = 0.5f;
    bool  noChao = true;
    float alturaChao = alturaTela - player.h;

    // Variavéis da base

    SDL_FRect base = {
    0.0f,
    0.0f,
    80.0f,
    (float)alturaTela
    };

    // Spritesheets

     

    //Inimigos
    SDL_FRect srcInimigo = {
        0,
        0,
        32,
        32
    };
    // ----------------------------------------------------------
    // VARIÁVEIS DE JOGO
    // ----------------------------------------------------------

    Projetil projeteis[MAX_PROJETEIS] = { 0 };
    Inimigo  inimigos[MAX_INIMIGOS] = { 0 };

    int  mapa[LINHAS][COLUNAS] = { 0 };
    int  pontos = 0;
    int pontosAntigos = -1;
    int  ondas = 1;
    int ondaAntiga = -1;
    int  vidas = MAX_VIDAS;
    int  inimigosDerrotados = 0;
    int  inimigosAtivos = 5;

    const int larguraFrameTiro = 223;
    const int alturaFrameTiro = 223;
    const int totalFramesTiro = 5;

    SDL_Texture* texturaPontos = NULL;
    SDL_Texture* TexturaOndas = NULL;

    bool running = true;
    bool spam = false;


    // ----------------------------------------------------------
    // Textura de pontos
    // ----------------------------------------------------------
    char buffer[50];
    sprintf_s(buffer, sizeof(buffer), "Pontos: %d", pontos);

    SDL_Surface* surfacePontos =
        TTF_RenderText_Blended(fontHud, buffer, 0, preto);

    texturaPontos =
        SDL_CreateTextureFromSurface(renderer, surfacePontos);

    SDL_DestroySurface(surfacePontos);

    pontosAntigos = pontos;

    // Ondas

    char bufferOnda[50];
    sprintf_s(bufferOnda, sizeof(bufferOnda), "Onda: %d", ondas);

    SDL_Surface* surfaceOnda =
        TTF_RenderText_Blended(fontHud, bufferOnda, 0, preto);

    TexturaOndas =
        SDL_CreateTextureFromSurface(renderer, surfaceOnda);

    SDL_DestroySurface(surfaceOnda);

    // ----------------------------------------------------------
    // TEMPORIZAÇÃO DE SPAWN E ONDAS
    // ----------------------------------------------------------

    Uint64 ultimoSpawn = SDL_GetTicks();
    Uint64 intervaloSpawn = 2000 - (ondas * 150);
    Uint64 tempoAtual = 0;
    Uint64 inicioOnda = SDL_GetTicks();
    Uint64 inicioSpam = SDL_GetTicks();

    SDL_Event event;

    // ==========================================================
    // LOOP PRINCIPAL
    // ==========================================================

    while (running)
    {
        // ------------------------------------------------------
        // PROCESSAMENTO DE EVENTOS
        // ------------------------------------------------------

        while (SDL_PollEvent(&event))
        {
            // Eventos do menu
            if (estado == MENU &&
                event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                float mouseX = event.button.x;
                float mouseY = event.button.y;

                if (mouseX >= botaoJogar.x &&
                    mouseX <= botaoJogar.x + botaoJogar.w &&
                    mouseY >= botaoJogar.y &&
                    mouseY <= botaoJogar.y + botaoJogar.h)
                {
                    estado = JOGAR;
                }

                if (mouseX >= botaoRanking.x &&
                    mouseX <= botaoRanking.x + botaoRanking.w &&
                    mouseY >= botaoRanking.y &&
                    mouseY <= botaoRanking.y + botaoRanking.h)
                {
                    estado = RANKING;
                }

                if (mouseX >= botaoSair.x &&
                    mouseX <= botaoSair.x + botaoSair.w &&
                    mouseY >= botaoSair.y &&
                    mouseY <= botaoSair.y + botaoSair.h)
                {
                    running = false;
                }
            }


            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            // Redimensionamento da janela
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSize(window, &larguraTela, &alturaTela);

                tileW = (float)larguraTela / COLUNAS;
                tileH = (float)alturaTela / LINHAS;

                alturaChao = alturaTela - player.h;

                logoRect.w = larguraTela * 0.35f;
                logoRect.h = logoRect.w * 0.5f;

                logoRect.x = larguraTela * 0.05f;
                logoRect.y = alturaTela * 0.05f;

                botaoJogar.w = larguraTela * 0.25f;
                botaoJogar.h = alturaTela * 0.10f;
                botaoJogar.x = larguraTela * 0.12f;
                botaoJogar.y = alturaTela * 0.45f;

                botaoRanking.w = botaoJogar.w;
                botaoRanking.h = botaoJogar.h;
                botaoRanking.x = botaoJogar.x;
                botaoRanking.y = botaoJogar.y + botaoJogar.h + 20;

                botaoSair.w = botaoJogar.w;
                botaoSair.h = botaoJogar.h;
                botaoSair.x = botaoJogar.x;
                botaoSair.y = botaoRanking.y + botaoRanking.h + 20;

                base.h = (float)alturaTela;
            }

            // Atirar
            if (estado == JOGAR && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    float mouseX = event.button.x;
                    float mouseY = event.button.y;

                    float centroPlayerX = player.x + player.w / 2;
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

                    for (int i = 0; i < MAX_PROJETEIS; i++)
                    {
                        if (!projeteis[i].ativo)
                        {
                            projeteis[i].ativo = true;
                            projeteis[i].frame = 0;
                            projeteis[i].explodindo = false;
                            projeteis[i].ultimoFrame = SDL_GetTicks();

                            projeteis[i].rect.x = centroPlayerX;
                            projeteis[i].rect.y = centroPlayerY;

                            projeteis[i].rect.w = 32;
                            projeteis[i].rect.h = 32;

                            float velocidade = 12.0f;

                            projeteis[i].velX = dirX * velocidade;
                            projeteis[i].velY = dirY * velocidade;

                            break;
                        }
                    }
                }
            }
        }

        // ------------------------------------------------------
        // SISTEMA DE ONDAS
        // ------------------------------------------------------

        if (SDL_GetTicks() - inicioOnda >= 30000)
        {
            ondas++;

            inicioOnda = SDL_GetTicks();

            spam = true;
            inicioSpam = SDL_GetTicks();
            inimigosAtivos += 10;
            intervaloSpawn -= 150;

            SDL_Log("SPAM!!");

            if (inimigosAtivos > MAX_INIMIGOS)
            {
                inimigosAtivos = MAX_INIMIGOS;
            }

            SDL_Log("Onda %d", ondas);
        }

        if (spam)
        {
            if (SDL_GetTicks() - inicioSpam >= 10000)
            {
                spam = false;

                intervaloSpawn = 2000 - (ondas * 150);

                // Limitador
                if (intervaloSpawn < 500)
                {
                    intervaloSpawn = 500;
                }

                SDL_Log("SPAM de inimigos terminou!");
            }
        }

        // ------------------------------------------------------
        // TELAS DE ESTADO (MENU / RANKING / GAMEOVER)
        // ------------------------------------------------------

        // Desenhar o menu
        if (estado == MENU)
        {

            SDL_RenderClear(renderer);

            SDL_FRect menuRect = {
                0,
                0,
                (float)larguraTela,
                (float)alturaTela
            };

            SDL_RenderTexture(renderer, menu, NULL, &menuRect);

            //Desenhar logo e botões

            SDL_RenderTexture(renderer, logo, NULL, &logoRect);
            SDL_RenderTexture(renderer, jogar, NULL, &botaoJogar);
            SDL_RenderTexture(renderer, creditos, NULL, &botaoRanking);
            SDL_RenderTexture(renderer, sair, NULL, &botaoSair);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }

        if (estado == RANKING)
        {
            SDL_SetRenderDrawColor(renderer, 20, 20, 80, 255);
            SDL_RenderClear(renderer);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }

        // Tela de game over
        if (estado == GAMEOVER)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_FRect gameoverRect = {
                0,
                0,
                (float)larguraTela,
                (float)alturaTela
            };

            SDL_RenderTexture(renderer, gameover, NULL, &gameoverRect);
            SDL_RenderPresent(renderer);

            if (SDL_GetTicks() - tempoAtual >= 3000)
            {
                estado = MENU;

                // Reiniciar jogador
                player.x = 320.0f;
                player.y = 240.0f;

                vidas = MAX_VIDAS;
                velocidadeY = 0.0f;
                noChao = true;

                // Reiniciar as variáveis de jogo
                pontos = 0;
                ondas = 1;
                inimigosAtivos = 5;
                inimigosDerrotados = 0;
                spam = false;
                intervaloSpawn = 2000 - (ondas * 150);

                inicioOnda = SDL_GetTicks();
                inicioSpam = SDL_GetTicks();
                ultimoSpawn = SDL_GetTicks();

                // Limpar projéteis
                for (int i = 0; i < MAX_PROJETEIS; i++)
                {
                    projeteis[i].ativo = false;
                }

                // Limpar inimigos
                for (int i = 0; i < MAX_INIMIGOS; i++)
                {
                    inimigos[i].ativo = false;
                }
            }

            SDL_Delay(16);
            continue;
        }

        // ------------------------------------------------------
        // CONTROLE DE SPAWN
        // ------------------------------------------------------

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

        if (tempoAtual - ultimoSpawn >= intervaloSpawn && vivos < inimigosAtivos)
        {
            ultimoSpawn = tempoAtual;

            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (!inimigos[i].ativo)
                {
                    inimigos[i].ativo = true;

                    inimigos[i].rect.w = 32;
                    inimigos[i].rect.h = 32;

                    inimigos[i].rect.x = larguraTela;
                    inimigos[i].rect.x -= inimigos[i].speed;
                    inimigos[i].rect.y =
                        (float)(rand() % (alturaTela - 32));

                    inimigos[i].speed = 2.0f + (ondas * 0.5f);

                    if (spam)
                    {
                        inimigos[i].speed += 2.0f;
                    }

                    break;
                }
            }
        }

        // ------------------------------------------------------
        // ATUALIZAÇÃO DE PROJETEIS
        // ------------------------------------------------------
        
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


        // ------------------------------------------------------
        // ATUALIZAÇÃO DE INIMIGOS
        // ------------------------------------------------------

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                inimigos[i].rect.x -= inimigos[i].speed;

                if (inimigos[i].rect.x + inimigos[i].rect.w < 0)
                {
                    inimigos[i].ativo = false;
                }
            }
        }

        // ------------------------------------------------------
        // COLISÃO: PLAYER E INIMIGOS
        // ------------------------------------------------------

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                if (SDL_HasRectIntersectionFloat(
                    &player,
                    &inimigos[i].rect))
                {
                    pontos -= 50;
                    player.x -= 100.0f;
                    player.y -= 50.0f;

                    inimigos[i].ativo = false;
                }
            }
        }

        // Colisão base e inimigos

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                if (SDL_HasRectIntersectionFloat(
                    &base,
                    &inimigos[i].rect))
                {
                    vidas--;

                    inimigos[i].ativo = false;

                    SDL_Log("Vidas restantes: %d", vidas);

                    if (vidas <= 0)
                    {
                        SDL_Log("GAME OVER");
                        estado = GAMEOVER;
                        tempoAtual = SDL_GetTicks();
                    }
                }
            }
        }


        // ------------------------------------------------------
        // COLISÃO: PROJETEIS E INIMIGOS
        // ------------------------------------------------------

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (!inimigos[i].ativo)
                continue;

            for (int j = 0; j < MAX_PROJETEIS; j++)
            {
                if (!projeteis[j].ativo)
                    continue;

                if (SDL_HasRectIntersectionFloat(
                    &inimigos[i].rect,
                    &projeteis[j].rect))
                {
                    inimigos[i].ativo = false;
                    projeteis[j].explodindo = true;
                    projeteis[j].frame = 3;
                    projeteis[j].velX = 0;
                    projeteis[j].velY = 0;
                    projeteis[j].ultimoFrame = SDL_GetTicks();
                    pontos += 10;
                    inimigosDerrotados++;
                }
            }
        }

        // ------------------------------------------------------
        // INPUT DO TECLADO
        // ------------------------------------------------------

        const bool* key = SDL_GetKeyboardState(NULL);

        // Fechar o jogo
        if (key[SDL_SCANCODE_ESCAPE])
        {
            running = false;
        }

        // Movimentação
        if (key[SDL_SCANCODE_A])
        {
            player.x -= speed;
        }

        if (key[SDL_SCANCODE_D])
        {
            player.x += speed;
        }

        if (key[SDL_SCANCODE_W] && noChao)
        {
            velocidadeY = -10.0f;
            noChao = false;
        }

        if (key[SDL_SCANCODE_SPACE] && noChao)
        {
            velocidadeY = -10.0f;
            noChao = false;
        }

        //------------------------------------------------------
        // Colisões da tela
        //------------------------------------------------------

        if(player.x < 0){
            player.x = 0;
        }

        if (player.x + player.w > larguraTela){
            player.x = larguraTela - player.w;
        }

        // ------------------------------------------------------
        // FÍSICA: GRAVIDADE E CHÃO
        // ------------------------------------------------------

        // Gravidade
        velocidadeY += gravidade;
        player.y += velocidadeY;

        // Chão
        if (player.y >= alturaChao)
        {
            player.y = alturaChao;
            velocidadeY = 0.0f;
            noChao = true;
        }

        // Teto
        if (player.y <0){
            player.y = 0;
            velocidadeY = 0.0f;

        }

        // Atualizando textura de pontos
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
        
        // Atualizando textura de onda
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

        // ------------------------------------------------------
        // RENDERIZAÇÃO
        // ------------------------------------------------------



        // Fundo
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_FRect fundo = {
            0,
            0,
            (float)larguraTela,
            (float)alturaTela
        };

        SDL_RenderTexture(renderer, background, NULL, &fundo);

        // Desenho do mapa
        for (int y = 0; y < LINHAS; y++)
        {
            for (int x = 0; x < COLUNAS; x++)
            {
                SDL_FRect tile = {
                    x * tileW,
                    y * tileH,
                    tileW,
                    tileH
                };

                // AQUI VOCÊ DESENHARÁ OS PNGS

                // Desenha apenas a grade, quando precisar, só descomentar
                /*SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderRect(renderer, &tile);*/
            }
        }
        // Base
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &base);

        // Jogador
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &player);

        // Desenhar balas

        for (int i = 0; i < MAX_PROJETEIS; i++)
        {
            if (!projeteis[i].ativo)
                continue;

            SDL_FRect srcTiro = {
                projeteis[i].frame * larguraFrameTiro,
                0,
                larguraFrameTiro,
                alturaFrameTiro
            };

            SDL_RenderTexture(
                renderer,
                projetil,
                &srcTiro,
                &projeteis[i].rect
            );
        }

        // Desenhar inimigos
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                SDL_FRect src = {
                    0.0f,
                    0.0f,
                    32.0f,
                    32.0f
                };

                SDL_RenderTexture(
                    renderer,
                    texturaInimigo,
                    &src,
                    &inimigos[i].rect
                );
            }
        }

        // Vidas    

        for (int i = 0; i < vidas; i++)
        {
            SDL_FRect vida = {
                10 + (i * 40),
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

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    
    } // fim do loop principal

    // ==========================================================
    // LIMPEZA E ENCERRAMENTO
    // ==========================================================

    // Destruir texturas


    SDL_DestroyTexture(texturaPontos);
    SDL_DestroyTexture(TexturaOndas);
    SDL_DestroyTexture(menu);
    SDL_DestroyTexture(logo);
    SDL_DestroyTexture(jogar);
    SDL_DestroyTexture(creditos);
    SDL_DestroyTexture(sair);
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(texturaInimigo);
    SDL_DestroyTexture(projetil);
    SDL_DestroyTexture(coracao);
    SDL_DestroyTexture(gameover);



    /*
    SDL_DestroyTexture;
    */

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
