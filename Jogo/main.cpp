//Movimentação do player com WASD e espaço para pular - ok
//Combate a distancia com projeteis - ok
//Menu de iniciar jogo, sair e acessar rankings
// criação de mapa 2d com tiles
//sistema de pontuação
//sistema de ordas
// spawn de inimigos
//sistema de vida
// animação de sprites

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
#include <stdbool.h>

#define COLUNAS 25
#define LINHAS 19

#define MAX_PROJETEIS 5

#define MAX_INIMIGOS 10

typedef struct
{
    SDL_FRect rect;
    bool ativo;
    float speed;
} Inimigo;

typedef struct
{
    SDL_FRect rect;
    bool ativo;

    float velX;
    float velY;
} Projetil;

int main(int argc, char* argv[])
{
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

    //adaptar a tela
    int larguraTela, alturaTela;

    SDL_GetWindowSize(window, &larguraTela, &alturaTela);

    float tileW = (float)larguraTela / COLUNAS;
    float tileH = (float)alturaTela / LINHAS;

    int mapa[LINHAS][COLUNAS] = { 0 };


    // CARREGAMENTO DOS TILESETS
 
    SDL_Texture* background =
        IMG_LoadTexture(renderer, "assets/background.png");

    if (!background)
    {
        SDL_Log("Erro ao carregar background: %s", SDL_GetError());
    }

    SDL_FRect player = {
        320.0f,
        240.0f,
        32.0f,
        32.0f
    };

    Projetil projeteis[MAX_PROJETEIS] = { 0 };
    Inimigo inimigos[MAX_INIMIGOS] = { 0 };

    //gravidade 
    float speed = 5.0f;

    float velocidadeY = 0.0f;
    float gravidade = 0.5f;

    bool noChao = true;
    float alturaChao = alturaTela - player.h;

    bool running = true;
    SDL_Event event;

    //spawn de inimigos
    Uint64 ultimoSpawn = SDL_GetTicks();
    Uint64 intervaloSpawn = 2000;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSize(
                    window,
                    &larguraTela,
                    &alturaTela
                );

                tileW = (float)larguraTela / COLUNAS;
                tileH = (float)alturaTela / LINHAS;

                alturaChao = alturaTela - player.h;
            }
            //atirar
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
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
        //spawn automatico
        Uint64 tempoAtual = SDL_GetTicks();

        if (tempoAtual - ultimoSpawn >= intervaloSpawn)
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

                    inimigos[i].speed = 2.0f;

                    break;
                }
            }
        }
        //atualizar projeteis
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
        //atualizar inimigos
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
        //colisao projeteis e inimigos
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
                }
            }
        }

        const bool* key = SDL_GetKeyboardState(NULL);


        //fechar o jogo
        if (key[SDL_SCANCODE_ESCAPE])
        {
            running = false;
        }
        //movimentação
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

        // Fundo
        SDL_SetRenderDrawColor(
            renderer,
            20,
            20,
            20,
            255
        );

        SDL_RenderClear(renderer);

        SDL_FRect fundo = {
            0,
            0,  
            (float)larguraTela,
            (float)alturaTela
        };

        SDL_RenderTexture(
            renderer,
            background,
            NULL,
            &fundo
        );

        // DESENHO DO MAPA

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
            
         
                

                // desenha apenas a grade

                SDL_SetRenderDrawColor(
                    renderer,
                    0,
                    255,
                    0,
                    255
                );

                SDL_RenderRect(renderer, &tile);
            }
        }

        // Jogador
        SDL_SetRenderDrawColor(
            renderer,
            255,
            0,
            0,
            255
        );

        SDL_RenderFillRect(renderer, &player);

        //desenhar balas
        SDL_SetRenderDrawColor(
            renderer,
            255,
            0,
            0,
            255
        );

        for (int i = 0; i < MAX_PROJETEIS; i++)
        {
            if (projeteis[i].ativo)
            {
                SDL_RenderFillRect(
                    renderer,
                    &projeteis[i].rect
                );
            }
        }
        //desenhar inimigos
        SDL_SetRenderDrawColor(
            renderer,
            0,
            0,
            255,
            255
        );

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                SDL_RenderFillRect(
                    renderer,
                    &inimigos[i].rect
                );
            }
        }


        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }


    // destruir texturas
   
    SDL_DestroyTexture(background);
    /*
    SDL_DestroyTexture;
    */

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}