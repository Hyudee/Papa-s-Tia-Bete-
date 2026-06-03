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
Boss com spritesheet
Fundo do menu (Pode ser online)
Alvo dos inimigos
Sons

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
        SDL_Log("Falha ao inicializar a ttf: %s", SDL_GetError);
    }

    // Fonte do jogo

    TTF_Font* font = TTF_OpenFont("assets/PixelifySans.ttf", 32);

    if (!font)
    {
        SDL_Log("Erro ao carregar fonte");
    }

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

    float larguraBotao = 500.0f;
    float alturaBotao = 50.0f;

    botaoJogar.w = larguraBotao;
    botaoJogar.h = alturaBotao;
    botaoJogar.x = (larguraTela - larguraBotao) / 2;
    botaoJogar.y = alturaTela * 0.35f;

    botaoRanking.w = larguraBotao;
    botaoRanking.h = alturaBotao;
    botaoRanking.x = (larguraTela - larguraBotao) / 2;
    botaoRanking.y = alturaTela * 0.50f;

    botaoSair.w = larguraBotao;
    botaoSair.h = alturaBotao;
    botaoSair.x = (larguraTela - larguraBotao) / 2;
    botaoSair.y = alturaTela * 0.65f;

    srand((unsigned)time(NULL));

    // ----------------------------------------------------------
    // CARREGAMENTO DOS TILESETS
    // ----------------------------------------------------------

    SDL_Texture* background =
        IMG_LoadTexture(renderer, "assets/background.png");

    if (!background)
    {
        SDL_Log("Erro ao carregar background: %s", SDL_GetError());
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

    // ----------------------------------------------------------
    // VARIÁVEIS DE JOGO
    // ----------------------------------------------------------

    Projetil projeteis[MAX_PROJETEIS] = { 0 };
    Inimigo  inimigos[MAX_INIMIGOS] = { 0 };

    int  mapa[LINHAS][COLUNAS] = { 0 };
    int  pontos = 0;
    int  ondas = 1;
    int  vidas = MAX_VIDAS;
    int  inimigosDerrotados = 0;
    int  inimigosAtivos = 5;

    bool running = true;
    bool spam = false;

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

                botaoJogar.x = (larguraTela - botaoJogar.w) / 2;
                botaoJogar.y = alturaTela * 0.35f;

                botaoRanking.x = (larguraTela - botaoRanking.w) / 2;
                botaoRanking.y = alturaTela * 0.50f;

                botaoSair.x = (larguraTela - botaoSair.w) / 2;
                botaoSair.y = alturaTela * 0.65f;
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

                            projeteis[i].rect.x = centroPlayerX;
                            projeteis[i].rect.y = centroPlayerY;

                            projeteis[i].rect.w = 10;
                            projeteis[i].rect.h = 10;

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
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
            SDL_RenderFillRect(renderer, &botaoJogar);

            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &botaoRanking);

            SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
            SDL_RenderFillRect(renderer, &botaoSair);

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

            // Fundo vermelho
            SDL_SetRenderDrawColor(renderer, 180, 0, 0, 255);

            SDL_FRect tela = {
                0,
                0,
                (float)larguraTela,
                (float)alturaTela
            };

            SDL_RenderFillRect(renderer, &tela);
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
            if (projeteis[i].ativo)
            {
                projeteis[i].rect.x += projeteis[i].velX;
                projeteis[i].rect.y += projeteis[i].velY;

                if (projeteis[i].rect.x < 0 ||
                    projeteis[i].rect.x > larguraTela ||
                    projeteis[i].rect.y < 0 ||
                    projeteis[i].rect.y > alturaTela)
                {
                    projeteis[i].ativo = false;
                }
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
                    projeteis[j].ativo = false;
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

        // Jogador
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &player);

        // Desenhar balas
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        for (int i = 0; i < MAX_PROJETEIS; i++)
        {
            if (projeteis[i].ativo)
            {
                SDL_RenderFillRect(renderer, &projeteis[i].rect);
            }
        }

        // Desenhar inimigos
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                SDL_RenderFillRect(renderer, &inimigos[i].rect);
            }
        }

        // Vidas
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        for (int i = 0; i < vidas; i++)
        {
            SDL_FRect vida = {
                10 + (i * 40),
                10,
                30,
                30
            };

            SDL_RenderFillRect(renderer, &vida);
        }

        
        // Renderizar o texto
        SDL_Color preto = { 0, 0, 0, 255 };

        SDL_Surface* textoSurface =
            TTF_RenderText_Blended(font, "JOGAR", 0, preto);

        SDL_Texture* textoTexture =
            SDL_CreateTextureFromSurface(renderer, textoSurface);

        SDL_DestroySurface(textoSurface);

        float textoW;
        float textoH;

        SDL_GetTextureSize(
            textoTexture,
            &textoW,
            &textoH
        );

        SDL_FRect textoRect =
        {
            botaoJogar.x + (botaoJogar.w - textoW) / 2,
            botaoJogar.y + (botaoJogar.h - textoH) / 2,
            textoW,
            textoH
        };

        SDL_RenderTexture(
            renderer,
            textoTexture,
            NULL,
            &textoRect
        );

        SDL_RenderPresent(renderer);
        SDL_Delay(16);

    } // fim do loop principal

    // ==========================================================
    // LIMPEZA E ENCERRAMENTO
    // ==========================================================

    // Destruir texturas
    SDL_DestroyTexture(background);
    /*
    SDL_DestroyTexture;
    */

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
