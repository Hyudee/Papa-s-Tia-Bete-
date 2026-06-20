// ============================================================
// FUNCIONALIDADES
// ============================================================
// Movimentação do player com WASD e espaço para pular - ok
// Combate a distancia com projeteis - ok
// Menu de iniciar jogo, sair e acessar rankings - em dev
// criação de mapa 2d com tiles
// sistema de pontuação - ok
// sistema de ordas - ok
// spawn de inimigos - ok
// sistema de vida - ok
// animação de sprites - ok

// ============================================================
// PARA FAZER
// ============================================================
/*
Desenhar e incluir:

Coração de vida - ok
Personagem com spritesheet - ok
Inimigos com spritesheet - ok
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
#include <SDL3_mixer/SDL_mixer.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

// ============================================================
// DEFINES
// ============================================================

#define COLUNAS      25
#define LINHAS       19
#define MAX_PROJETEIS 5
#define MAX_INIMIGOS  50
#define MAX_VIDAS     4
#define MAX_POWERUPS  2
#define MAX_POWERUPSATQ  1
#define MAX_OBSTACULOS 4

// ============================================================
// TIPOS E STRUCTS
// ============================================================

typedef enum {
    MENU,
    JOGAR,
    CONFIGURACOES,
    CONTROLES,
    RANKING,
    AJUDA,
    DIGITAR_NOME,
    GAMEOVER,
    SAIR
} Sistema;

typedef struct
{
    SDL_FRect rect;
    bool      ativo;
    float     speed;

    int frame;
    Uint64 ultimoFrame;
} Inimigo;

typedef struct
{
    SDL_FRect rect;
    bool ativo;
    float speed;

    int frame;
    Uint64 ultimoFrame;
} PowerupVida;

typedef struct
{
    SDL_FRect rect;
    bool ativo;
    float speed;

    int frame;
    Uint64 ultimoFrame;
} PowerupAtq;

typedef struct {
    SDL_FRect rect;
    bool ativo;
    float speed;

    int vida;
    int vidaMax;

    int frame;
    Uint64 ultimoFrame;
}Boss;

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

typedef struct
{
    char nome[20];
    int pontos;
} Ranking;

typedef enum
{
    OBSTACULO_SOMBRA,
    OBSTACULO_CAINDO,
    OBSTACULO_CHAO,
    OBSTACULO_QUEBRANDO
} EstadoObstaculo;

typedef struct
{
    SDL_FRect rect;

    bool ativo;

    EstadoObstaculo estado;

    int frame;
    Uint64 ultimoFrame;

    float velocidadeY;

    Uint64 inicioEstado;

} Obstaculo;

enum FlashBase
{
    FLASH_NENHUM,
    FLASH_DANO,
    FLASH_CURA
};

// Funções do ranking

void InicializarRanking(Ranking ranking[])
{
    for (int i = 0; i < 3; i++)
    {
        strcpy_s(ranking[i].nome,
            sizeof(ranking[i].nome),
            "???");
        ranking[i].pontos = 0;
    }
}

void CarregarRanking(Ranking ranking[])
{
    FILE* arq;
    fopen_s(&arq,
        "ranking.dat",
        "rb");

    if (arq == NULL)
    {
        InicializarRanking(ranking);
        return;
    }

    fread(ranking, sizeof(Ranking), 3, arq);
    fclose(arq);
}

void SalvarRanking(Ranking ranking[])
{
    FILE* arq;
    fopen_s(&arq,
        "ranking.dat",
        "wb");

    if (arq == NULL)
        return;

    fwrite(ranking, sizeof(Ranking), 3, arq);
    fclose(arq);
}

int AdicionarPontuacao(Ranking ranking[],
    char nome[],
    int pontos)
{
    if (pontos <= ranking[2].pontos)
        return 0;

    strcpy_s(ranking[2].nome,
        sizeof(ranking[2].nome),
        nome);
    ranking[2].pontos = pontos;

    for (int i = 2; i > 0; i--)
    {
        if (ranking[i].pontos >
            ranking[i - 1].pontos)
        {
            Ranking temp = ranking[i];
            ranking[i] = ranking[i - 1];
            ranking[i - 1] = temp;
        }
    }

    return 1;
}

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
    SDL_Color branco = { 255, 255, 255, 255 };

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

    SDL_FRect abaConfig;
    SDL_FRect abaControles;
    SDL_FRect abaRanking;
    SDL_FRect abaAjuda;

    float textoW, textoH;
    SDL_FRect textoRect;

    logoRect.w = larguraTela * 0.35f; 
    logoRect.h = logoRect.w * 0.5f; 

    logoRect.x = larguraTela * 0.07f;
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

    float larguraAba = larguraTela * 0.03f;
    float alturaAba = alturaTela * 0.18f;
    float padding = larguraTela * 0.005f;

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

    Ranking ranking[3];
    char nomeJogador[20] = "";
    int tamanhoNome = 0;

    srand((unsigned)time(NULL));
    CarregarRanking(ranking);

    // ----------------------------------------------------------
    // CARREGAMENTO DOS TILESETS
    // ----------------------------------------------------------

    SDL_Texture* background =
        IMG_LoadTexture(renderer, "assets/cozinha.png");

    if (!background)
    {
        SDL_Log("Erro ao carregar background: %s", SDL_GetError());
    }
    SDL_Texture* backgroundNome =
        IMG_LoadTexture(renderer, "assets/background.png");

    if (!backgroundNome)
    {
        SDL_Log("Erro ao carregar background do ranking: %s", SDL_GetError());
    }

    SDL_Texture* menu =
        IMG_LoadTexture(renderer, "assets/menu.png");

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
        IMG_LoadTexture(renderer, "assets/acucar.png");

    if (!texturaInimigo)
    {
        SDL_Log("Erro ao carregar inimigos: %s", SDL_GetError());
    }
    
    SDL_Texture* texturaPlayer =
            IMG_LoadTexture(renderer, "assets/player.png");

    SDL_Texture* texturaPlayerSujo =
            IMG_LoadTexture(renderer, "assets/playerSujo.png");

    if (!texturaPlayer) {
            SDL_Log("Erro ao carregar player: %s", SDL_GetError());
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
    SDL_Texture* powerUpAtq =
        IMG_LoadTexture(renderer, "assets/powerupatq.png");

    if (!powerUpAtq)
    {
        SDL_Log("Erro ao carregar textura de powerUp de ataque: %s", SDL_GetError());
    }
    SDL_Texture* powerUpVida =
        IMG_LoadTexture(renderer, "assets/powerupvida.png");

    if (!powerUpVida)
    {
        SDL_Log("Erro ao carregar textura de powerUp de vida: %s", SDL_GetError());
    }
    SDL_Texture* texturaTrofeu =
        IMG_LoadTexture(renderer, "assets/rankingaba.png");

    if (!texturaTrofeu)
    {
        SDL_Log("Erro ao carregar textura de aba de ranking: %s", SDL_GetError());
    }
    SDL_Texture* texturaEngrenagem =
        IMG_LoadTexture(renderer, "assets/configuracaoaba.png");

    if (!texturaEngrenagem)
    {
        SDL_Log("Erro ao carregar textura de aba de configuracao: %s", SDL_GetError());
    }
    SDL_Texture* texturaControles =
        IMG_LoadTexture(renderer, "assets/controlesaba.png");

    if (!texturaControles)
    {
        SDL_Log("Erro ao carregar textura de aba de controles: %s", SDL_GetError());
    }
    SDL_Texture* texturaAjuda =
        IMG_LoadTexture(renderer, "assets/interrogacaoaba.png");

    if (!texturaAjuda)
    {
        SDL_Log("Erro ao carregar textura de aba de ajuda: %s", SDL_GetError());
    }

    SDL_Texture* texturaBoss =
        IMG_LoadTexture(renderer, "assets/boss.png");

    SDL_Texture* texturaObstaculo =
        IMG_LoadTexture(renderer, "assets/obstaculo.png");

    if (!texturaObstaculo)
    {
        SDL_Log("Erro ao carregar obstáculo: %s", SDL_GetError());
    }

    SDL_Texture* texturaSombra =
        IMG_LoadTexture(renderer, "assets/sombra.png");

    SDL_Texture* base4 = IMG_LoadTexture(renderer, "assets/base4.png");
    SDL_Texture* base3 = IMG_LoadTexture(renderer, "assets/base3.png");
    SDL_Texture* base2 = IMG_LoadTexture(renderer, "assets/base2.png");
    SDL_Texture* base1 = IMG_LoadTexture(renderer, "assets/base1.png");
    

    // ----------------------------------------------------------
    // VARIÁVEIS DO JOGADOR
    // ----------------------------------------------------------

    SDL_FRect player = {
        320.0f,
        240.0f,
        64.0f,
        64.0f
    };

    const float SPEED_NORMAL = 5.0f;
    const float SPEED_SUJO = 2.5f;
    float speed = SPEED_NORMAL;
    float velocidadeY = 0.0f;
    float gravidade = 0.5f;
    float alturaChao = alturaTela - player.h;
    int framePowerUp = 0;
    int framePlayer = 0;
    int framePulo = 0;
    int danoTiro = 1;
    const int totalFramesPowerUp = 5;
    const int larguraFramePowerUp = 64;
    const int alturaFramePowerUp = 64;
    bool  noChao = true;
    bool atirando = false;
    bool pulando = false;
    bool aterrissando = false;
    bool olhandoDireita = true;
    bool dano = false;
    bool stun = false;
    bool ataqueBuff = false;    
    bool sujo = false;


    Uint64 ultimoFramePowerUp = 0;
    Uint64 sujoInicio = 0;
    Uint64 stunStart = 0;
    Uint64 danoStart = 0;
    Uint64 ultimoFramePlayer = 0;
    Uint64 inicioAterrissagem = 0;
    Uint64 ataqueBuffInicio = 0;
    const Uint64 DURACAO_SUJO = 5000;
    const Uint64 STUN_DURACAO = 1250;
    const Uint64 DURACAO_BUFF = 10000;


    // Variavéis da base

    SDL_FRect base = {
    0.0f,
    0.0f,
    80.0f,
    (float)alturaTela
    };

    float tempoFlashBase = 0.0f;
    FlashBase flashBase = FLASH_NENHUM;

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
    Boss boss = { 0 };
    PowerupVida vidasPowerup[MAX_POWERUPS] = {0};
    PowerupAtq ataquePowerup[MAX_POWERUPSATQ] = { 0 };
    Obstaculo obstaculos[MAX_OBSTACULOS] = { 0 };

    int  mapa[LINHAS][COLUNAS] = { 0 };
    int  pontos = 0;
    int pontosAntigos = -1;
    int  ondas = 1;
    int ondaAntiga = -1;
    int  vidas = MAX_VIDAS;
    int  inimigosDerrotados = 0;
    int  inimigosAtivos = 5;
    int spawnBoss = 1000;
    int bossAtivados = 0;
    int frameBase = 0;
    int opcaoConfig = 0; // 0 = Tela cheia, 1 = Música, 2 = Efeitos



    const int larguraFrameBase = 64;
    const int alturaFrameBase = 384;
    const int totalFramesBase = 4;


    const int larguraFrameTiro = 64;
    const int alturaFrameTiro = 64;
    const int totalFramesTiro = 5;
    const int larguraFrameInimigo = 64;
    const int alturaFrameInimigo = 64;
    const int totalFramesInimigo = 3;

    SDL_Texture* texturaPontos = NULL;
    SDL_Texture* TexturaOndas = NULL;
    SDL_Texture* texturaBuff = NULL;

    bool telaCheia = false;
    bool musicaLigada = true;
    bool efeitosLigados = true;
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

    //Buff
    SDL_Surface* surfaceBuff =
        TTF_RenderText_Blended(
            fontHud,
            "ATK x2",
            0,
            preto
        );

    texturaBuff =
        SDL_CreateTextureFromSurface(
            renderer,
            surfaceBuff
        );

    SDL_DestroySurface(surfaceBuff);

    // ----------------------------------------------------------
    // TEMPORIZAÇÃO DE SPAWN E ONDAS
    // ----------------------------------------------------------

    Uint64 ultimoSpawn = SDL_GetTicks();
    Uint64 ultimoSpawnPowerUp = SDL_GetTicks();
    Uint64 ultimoSpawnAtq = SDL_GetTicks();
    Uint64 intervaloSpawn = 2000 - (ondas * 150);
    Uint64 intervaloSpawnPowerUp = 20000 + (ondas * 150);
    Uint64 intervaloSpawnAtq = 40000;
    Uint64 ultimoSpawnObstaculo = SDL_GetTicks();
    Uint64 intervaloObstaculo = 15000;
    Uint64 tempoAtual = 0;
    Uint64 inicioOnda = SDL_GetTicks();
    Uint64 inicioSpam = SDL_GetTicks();
    Uint64 ultimoFrameBase = 0;

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
                    inicioOnda = SDL_GetTicks();
                    inicioSpam = SDL_GetTicks();
                    ultimoSpawn = SDL_GetTicks();
                    ultimoSpawnPowerUp = SDL_GetTicks();
                    ultimoSpawnAtq = SDL_GetTicks();
                }


                if (mouseX >= botaoSair.x &&
                    mouseX <= botaoSair.x + botaoSair.w &&
                    mouseY >= botaoSair.y &&
                    mouseY <= botaoSair.y + botaoSair.h)
                {
                    running = false;
                }

                // Troféu
                if (mouseX >= abaRanking.x &&
                    mouseX <= abaRanking.x + abaRanking.w &&
                    mouseY >= abaRanking.y &&
                    mouseY <= abaRanking.y + abaRanking.h)
                {
                    estado = RANKING;
                }

                // Engrenagem
                if (mouseX >= abaConfig.x &&
                    mouseX <= abaConfig.x + abaConfig.w &&
                    mouseY >= abaConfig.y &&
                    mouseY <= abaConfig.y + abaConfig.h)
                {
                    estado = CONFIGURACOES;
                }

                // Controles
                if (mouseX >= abaControles.x &&
                    mouseX <= abaControles.x + abaControles.w &&
                    mouseY >= abaControles.y &&
                    mouseY <= abaControles.y + abaControles.h)
                {
                   estado = CONTROLES;
                }

                // Ajuda
                if (mouseX >= abaAjuda.x &&
                    mouseX <= abaAjuda.x + abaAjuda.w &&
                    mouseY >= abaAjuda.y &&
                    mouseY <= abaAjuda.y + abaAjuda.h)
                {
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

                    estado = GAMEOVER;
                    tempoAtual = SDL_GetTicks();
                }
            }

            // Sair das abas
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

                case SDLK_UP:
                    if (opcaoConfig > 0)
                        opcaoConfig--;
                    break;

                case SDLK_DOWN:
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
                    }
                    else if (opcaoConfig == 2)
                    {
                        efeitosLigados = !efeitosLigados;
                    }

                    break;
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

                float larguraAba = larguraTela * 0.03f;
                float alturaAba = alturaTela * 0.18f;
                float padding = larguraTela * 0.005f;

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

        // Atualização da base
        if (SDL_GetTicks() - ultimoFrameBase > 150)
        {
            frameBase++;

            if (frameBase >= totalFramesBase)
                frameBase = 0;

            ultimoFrameBase = SDL_GetTicks();
        }

        // Atualização do flash da base
        if (tempoFlashBase > 0.0f)
        {
            tempoFlashBase -= 1.0f / 60.0f;

            if (tempoFlashBase <= 0.0f)
            {
                tempoFlashBase = 0.0f;
                flashBase = FLASH_NENHUM;
            }
        }

        // Atualização da animação do PowerUp de Vida
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

        // Atualização da animação do PowerUp de Ataque
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
        // ------------------------------------------------------
        // SISTEMA DE ONDAS
        // ------------------------------------------------------

        if (estado == JOGAR && SDL_GetTicks() - inicioOnda >= 30000)
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

        if (estado == JOGAR && spam)
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

        // Acabar buffs
        if (ataqueBuff &&
            SDL_GetTicks() - ataqueBuffInicio >= DURACAO_BUFF)
        {
            ataqueBuff = false;
            danoTiro = 1;
        }

        if (sujo &&
            SDL_GetTicks() - sujoInicio >= DURACAO_SUJO)
        {
            sujo = false;
            speed = SPEED_NORMAL;
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
            SDL_RenderTexture(renderer, texturaEngrenagem, NULL, &abaConfig);
            SDL_RenderTexture(renderer, texturaControles, NULL, &abaControles);
            SDL_RenderTexture(renderer, texturaTrofeu, NULL, &abaRanking);
            SDL_RenderTexture(renderer, texturaAjuda, NULL, &abaAjuda);

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
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;
            char texto[50];

            // Titulo

            surface = TTF_RenderText_Blended(
                font,
                "RANKING",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            for (int i = 0; i < 3; i++)
            {
                sprintf_s(
                    texto,
                    sizeof(texto),
                    "%d. %s - %d",
                    i + 1,
                    ranking[i].nome,
                    ranking[i].pontos
                );

                surface =
                    TTF_RenderText_Blended(
                        font,
                        texto,
                        0,
                        preto
                    );

                texture =
                    SDL_CreateTextureFromSurface(
                        renderer,
                        surface
                    );

                SDL_GetTextureSize(
                    texture,
                    &rect.w,
                    &rect.h
                );

                rect.x =
                    larguraTela / 2 -
                    rect.w / 2;

                rect.y =
                    150 + i * 80;

                SDL_RenderTexture(
                    renderer,
                    texture,
                    NULL,
                    &rect
                );

                SDL_DestroySurface(surface);
                SDL_DestroyTexture(texture);
            }

            // Voltar

            surface = TTF_RenderText_Blended(
                fontHud,
                "ESC - Voltar",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x = 20;
            rect.y = alturaTela - 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
}

        //Nome

        if (estado == DIGITAR_NOME)
        {
            SDL_FRect fundo = {
            0,
            0,
            (float)larguraTela,
            (float)alturaTela
            };

            SDL_RenderTexture(renderer, backgroundNome, NULL, &fundo);

            //desenha os textos aqui

            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;

            surface = TTF_RenderText_Blended(
                font,
                "NOVO RECORDE!",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 120;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            surface = TTF_RenderText_Blended(
                fontHud,
                "Digite seu nome:",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 200;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            surface = TTF_RenderText_Blended(
                font,
                nomeJogador,
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 260;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }

        if (estado == CONFIGURACOES)
        {
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);
            
            // textos

            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;
            char texto[50];

            surface = TTF_RenderText_Blended(
                font,
                "CONFIGURAÇÕES",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            // Tela cheia

            sprintf_s(
                texto,
                sizeof(texto),
                "%s Tela cheia: %s",
                opcaoConfig == 0 ? ">" : " ",
                telaCheia ? "ON" : "OFF"
            );

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = alturaTela * 0.30f;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            // Musica
            sprintf_s(
                texto,
                sizeof(texto),
                "%s Musica: %s",
                opcaoConfig == 1 ? ">" : " ",
                musicaLigada ? "ON" : "OFF"
            );

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = alturaTela * 0.40f;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            // EFeitos Sonoros

            sprintf_s(
                texto,
                sizeof(texto),
                "%s Efeitos sonoros: %s",
                opcaoConfig == 2 ? ">" : " ",
                efeitosLigados ? "ON" : "OFF"
            );

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = alturaTela * 0.50f;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            // Voltar

            surface = TTF_RenderText_Blended(
                fontHud,
                "ESC - Voltar",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x = 20;
            rect.y = alturaTela - 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }
        if (estado == CONTROLES)
        {
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);
            
            // textos
            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;
            char texto[50];

            surface = TTF_RenderText_Blended(
                font,
                "CONTROLES",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            float yInicial = alturaTela * 0.25f;
            float espacamento = alturaTela * 0.07f;

            sprintf_s(texto, sizeof(texto), "W A S D - Movimentar");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            //Controles

            sprintf_s(texto, sizeof(texto), "ESPACO - Pular");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "BOTAO ESQUERDO DO MOUSE - Atirar");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 2;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "ESC - Pausar / Voltar");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);

            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 3;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            // Voltar

            surface = TTF_RenderText_Blended(
                fontHud,
                "ESC - Voltar",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x = 20;
            rect.y = alturaTela - 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }
        if (estado == AJUDA)
        {
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);
            
            // textos

            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;
            char texto[50];

            surface = TTF_RenderText_Blended(
                font,
                "AJUDA",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x =
                larguraTela / 2 - rect.w / 2;
            rect.y = 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            float yInicial = alturaTela * 0.20f;
            float espacamento = alturaTela * 0.055f;

            // Explicação
            sprintf_s(texto, sizeof(texto), "OBJETIVO:");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "Proteja a base dos inimigos.");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "Elimine inimigos para ganhar pontos.");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 2;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "BOSS:");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 4;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "Surge a cada 1000 pontos.");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 5;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "Enquanto estiver vivo, nenhum inimigo aparece.");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 6;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "POWER-UPS:");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 8;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "Colete-os para facilitar a sobrevivencia.");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 9;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            sprintf_s(texto, sizeof(texto), "Se a base perder todas as vidas, o jogo termina.");

            surface = TTF_RenderText_Blended(fontHud, texto, 0, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = larguraTela / 2 - rect.w / 2;
            rect.y = yInicial + espacamento * 11;

            SDL_RenderTexture(renderer, texture, NULL, &rect);

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

            // Voltar

            surface = TTF_RenderText_Blended(
                fontHud,
                "ESC - Voltar",
                0,
                preto
            );

            texture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    surface
                );

            SDL_GetTextureSize(
                texture,
                &rect.w,
                &rect.h
            );

            rect.x = 20;
            rect.y = alturaTela - 50;

            SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &rect
            );

            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);

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

            ultimoSpawnAtq = SDL_GetTicks();

            //Limpar powerUps

            for (int i = 0; i < MAX_POWERUPSATQ; i++)
            {
                ataquePowerup[i].ativo = false;
            }

            for (int i = 0; i < MAX_POWERUPS; i++)
            {
                vidasPowerup[i].ativo = false;
            }

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
                boss.ativo = false;
                boss.vida = 0;
                spawnBoss = 1000;
                bossAtivados = 0;
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

        if (!boss.ativo && tempoAtual - ultimoSpawn >= intervaloSpawn && vivos < inimigosAtivos)
        {
            ultimoSpawn = tempoAtual;

            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (!inimigos[i].ativo)
                {
                    inimigos[i].ativo = true;
                    inimigos[i].frame = 0;
                    inimigos[i].ultimoFrame = SDL_GetTicks();

                    inimigos[i].rect.w = 64;
                    inimigos[i].rect.h = 64;

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

        // Spawn de powerups

        if (tempoAtual - ultimoSpawnPowerUp >= intervaloSpawnPowerUp && (vidas < MAX_VIDAS)) {
            ultimoSpawnPowerUp = tempoAtual;

            for (int i = 0; i < MAX_POWERUPS; i++) {
                if (!vidasPowerup[i].ativo) {
                    vidasPowerup[i].ativo = true;
                    vidasPowerup[i].frame = 0;
                    vidasPowerup[i].ultimoFrame = SDL_GetTicks();

                    vidasPowerup[i].rect.w = 32;
                    vidasPowerup[i].rect.h = 32;

                    vidasPowerup[i].rect.x = larguraTela;

                    vidasPowerup[i].rect.y =
                        rand() % (alturaTela - 32);

                    vidasPowerup[i].speed = 7.0f;

                    break;
                }
            }
        }

        // SPAWN DOS OBSTÁCULOS

        if (ondas >= 3 &&
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

                    obstaculos[i].velocidadeY = 4.0f;

                    obstaculos[i].rect.w = 64.0f;
                    obstaculos[i].rect.h = 64.0f;

                    // posição aleatória
                    obstaculos[i].rect.x = (float)(rand() % (larguraTela - (int)obstaculos[i].rect.w));

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

            ataquePowerup[0].rect.w = 32;
            ataquePowerup[0].rect.h = 32;

            ataquePowerup[0].rect.x = larguraTela;
            ataquePowerup[0].rect.y =
                rand() % (alturaTela - 32);

            ataquePowerup[0].speed = 7.0f;
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

        // Atualização dos powerups

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

        // ATUALIZAÇÃO DO BOSS
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
                    framePlayer = 13;

                    obstaculos[i].rect.y = alturaChao;

                    obstaculos[i].estado = OBSTACULO_CHAO;

                    obstaculos[i].frame = 3;              // primeiro frame da colisão
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

                // Fica parado no chão
                obstaculos[i].frame = 4;
                break;

            case OBSTACULO_QUEBRANDO:

                obstaculos[i].frame = 3;

                if (SDL_GetTicks() - obstaculos[i].inicioEstado >= 300)
                {
                    obstaculos[i].ativo = false;
                }

                break;
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

                    // Empurrão
                    player.x -= 100.0f;
                    player.y -= 50.0f;

                    // Stun
                    stun = true;
                    stunStart = SDL_GetTicks();

                    // animação de dano

                    dano = true;
                    danoStart = SDL_GetTicks();
                    framePlayer = 13;
                        
                    inimigos[i].ativo = false;
                }

                if (stun && SDL_GetTicks() - stunStart >= STUN_DURACAO) {
                    stun = false;
                }
            }
        }

        // Colisão do tiro com o powerup
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
                    if (vidas < MAX_VIDAS){
                        vidas++;
                        flashBase = FLASH_CURA;
                        tempoFlashBase = 0.2f;
                    }

                    vidasPowerup[i].ativo = false;
                    projeteis[j].ativo = false;
                }
            }
        }

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
                    danoTiro = 2;

                    ataquePowerup[i].ativo = false;
                    projeteis[j].ativo = false;
                }
            }
        }

        // Colisão do tiro com o boss
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
                    flashBase = FLASH_DANO;
                    tempoFlashBase = 0.2f; // 200 ms
                    inimigos[i].ativo = false;

                    SDL_Log("Vidas restantes: %d", vidas);


                    if (vidas <= 0)
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
                            estado = GAMEOVER;
                            tempoAtual = SDL_GetTicks();
                        }
                    }
                }
            }
        }
        
        // Colisão do boss com a base

        if (boss.ativo) {
            if (SDL_HasRectIntersectionFloat(
                &base,
                &boss.rect
            ))
            {
                vidas -= 3;
                flashBase = FLASH_DANO;
                tempoFlashBase = 0.2f; // 200 ms
                boss.ativo = false;

                if (vidas <= 0)
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
                        estado = GAMEOVER;
                        tempoAtual = SDL_GetTicks();
                    }
                }
            }
        }

        
        // PROJÉTEIS E OBSTÁCULOS

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
                    pontos += 50;
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

        float playerXAnterior = player.x;
        float playerYAnterior = player.y;
        // Movimentação e efeito de stun
        if (!stun && !dano){

            if (key[SDL_SCANCODE_A])
            {
                player.x -= speed;
                olhandoDireita = false;
            }

            if (key[SDL_SCANCODE_D])
            {
                player.x += speed;
                olhandoDireita = true;
            }

            if ((key[SDL_SCANCODE_W] || key[SDL_SCANCODE_SPACE]) && noChao)
            {
                velocidadeY = -10.0f;
                noChao = false;
                pulando = true;

                framePlayer = 11;
            }

        }
        // Atualização da animação do player

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

        // Atualizando animação de dano
        
        if (dano)
        {
            Uint64 tempo = SDL_GetTicks() - danoStart;

            if (tempo < 300)
                framePlayer = 13;
            else if (tempo < 600)
                framePlayer = 14;
            else if (tempo < 900)
                framePlayer = 15;
            else
                dano = false;
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

            if (pulando)
            {
                pulando = false;
                aterrissando = true;
                inicioAterrissagem = SDL_GetTicks();

                framePlayer = 12;
            }
        }

        // Obstaculos

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

        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

            if (obstaculos[i].estado != OBSTACULO_CHAO)
                continue;

            if (SDL_HasRectIntersectionFloat(&player, &obstaculos[i].rect))
            {
                // Caiu em cima do obstáculo
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

                    noChao = false;
                }
            }
        }

        // Animação de pulo
        if (aterrissando)
        {
            framePlayer = 12;

            if (SDL_GetTicks() - inicioAterrissagem > 200)
            {
                aterrissando = false;
            }
        }

        if (pulando)
        {
            if (velocidadeY < -2)
            {
                framePlayer = 11; // subindo
            }

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

        // Spawn do boss
        if (!boss.ativo && pontos >= spawnBoss) {
            boss.ativo = true;

            boss.rect.w = 128;
            boss.rect.h = 128;

            boss.rect.x = larguraTela;
            boss.rect.y = (float)(rand() % (alturaTela - 128));

            boss.speed = 1.5f + (bossAtivados * 0.5f);

            boss.vidaMax = 30;
            boss.vida = boss.vidaMax;

            boss.frame = 0;
            boss.ultimoFrame = SDL_GetTicks();
            spawnBoss += 1000;
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
        SDL_Texture* texturaBase = NULL;

        switch (vidas)
        {
        case 4:
            texturaBase = base4;
            break;
        case 3:
            texturaBase = base3;
            break;
        case 2:
            texturaBase = base2;
            break;
        default:
            texturaBase = base1;    
            break;
        }

        SDL_FRect srcBase = {
        frameBase * larguraFrameBase,
        0,
        larguraFrameBase,
        alturaFrameBase
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

        SDL_RenderTexture(
            renderer,
            texturaBase,
            &srcBase,
            &base
        );

        // Volta a textura ao normal
        SDL_SetTextureColorMod(texturaBase, 255, 255, 255);

        // Desenho da direção do player
        static Uint64 ultimoFrameAndar = 0;

        bool andando = key[SDL_SCANCODE_A] || key[SDL_SCANCODE_D];

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
        // Jogador
        SDL_FRect srcPlayer = {
        framePlayer * 64,
        0,
        64,
        64
        };

        SDL_Texture* texturaAtualPlayer;

        if (sujo)
            texturaAtualPlayer = texturaPlayerSujo;
        else
            texturaAtualPlayer = texturaPlayer;

        SDL_RenderTexture(
            renderer,
            texturaAtualPlayer,
            &srcPlayer,
            &player
        );
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
                inimigos[i].frame * larguraFrameInimigo,
                0,
                larguraFrameInimigo,
                alturaFrameInimigo
                };

                SDL_RenderTexture(
                    renderer,
                    texturaInimigo,
                    &src,
                    &inimigos[i].rect
                );
            }
        }

        // Obstáculos
        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

                SDL_FRect srcSombra =
                {
                    obstaculos[i].frame * 64,
                    0,
                    64,
                    64
                };

                SDL_FRect sombraRect =
                {
                    obstaculos[i].rect.x,
                    alturaChao,
                    64,
                    64
                };

            if (obstaculos[i].estado == OBSTACULO_SOMBRA)
            {

                SDL_RenderTexture(
                    renderer,
                    texturaSombra,
                    &srcSombra,
                    &sombraRect
                );
            }
            else if (obstaculos[i].estado == OBSTACULO_CAINDO)
            {
                // sombra
                SDL_RenderTexture(
                    renderer,
                    texturaSombra,
                    &srcSombra,
                    &sombraRect
                );

                // obstáculo
                SDL_FRect srcObstaculo =
                {
                    obstaculos[i].frame * 64,
                    0,
                    64,
                    64
                };

                SDL_RenderTexture(
                    renderer,
                    texturaObstaculo,
                    &srcObstaculo,
                    &obstaculos[i].rect
                );
            }
            else
            {
                SDL_FRect srcObstaculo =
                {
                    obstaculos[i].frame * 64,
                    0,
                    64,
                    64
                };

                SDL_RenderTexture(
                    renderer,
                    texturaObstaculo,
                    &srcObstaculo,
                    &obstaculos[i].rect
                );
            }
        }

        // Desenhar boss

        if (boss.ativo){
            SDL_FRect srcBoss =
            {
                boss.frame * 128,
                0,
                128,
                128
            };

            SDL_RenderTexture(
                renderer,
                texturaBoss,
                &srcBoss,
                &boss.rect
            );
        }

        // Barra de vida do boss

        if (boss.ativo){
            SDL_FRect fundoVida = {
                larguraTela / 2 - 200,
                20,
                400,
                25
            };

            SDL_FRect vidaBoss = {
                larguraTela / 2 - 200,
                20,
                400.0f * ((float)(boss.vida) / boss.vidaMax),
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
                10 + (i * 40),
                10,
                32,
                32
            };

            SDL_RenderTexture(renderer, coracao, NULL, &vida);
        }

        //PowerUp

        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            if (!vidasPowerup[i].ativo)
                continue;

            SDL_FRect src = {
                vidasPowerup[i].frame * larguraFramePowerUp,
                0,
                larguraFramePowerUp,
                alturaFramePowerUp
            };

            SDL_FRect dest = vidasPowerup[i].rect;
            dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

            SDL_RenderTexture(
                renderer,
                powerUpVida,
                &src,
                &dest
            );
        }

        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (!ataquePowerup[i].ativo)
                continue;

            SDL_FRect src = {
                ataquePowerup[i].frame * larguraFramePowerUp,
                0,
                larguraFramePowerUp,
                alturaFramePowerUp
            };

            SDL_FRect dest = ataquePowerup[i].rect;
            dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

            SDL_RenderTexture(
                renderer,
                powerUpAtq,
                &src,
                &dest
            );
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

        if (ataqueBuff)
        {
            SDL_FRect buffRect;

            SDL_GetTextureSize(
                texturaBuff,
                &buffRect.w,
                &buffRect.h
            );

            buffRect.x = 10;
            buffRect.y = 50;

            SDL_RenderTexture(
                renderer,
                texturaBuff,
                NULL,
                &buffRect
            );
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    
    } // fim do loop principal

    // ==========================================================
    // LIMPEZA E ENCERRAMENTO
    // ==========================================================

    // Destruir texturas


    SDL_DestroyTexture(texturaPontos);
    SDL_DestroyTexture(TexturaOndas);
    SDL_DestroyTexture(texturaBuff);
    SDL_DestroyTexture(texturaPlayer);
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
    SDL_DestroyTexture(texturaEngrenagem);
    SDL_DestroyTexture(texturaTrofeu);
    SDL_DestroyTexture(texturaControles);
    SDL_DestroyTexture(texturaAjuda);
    SDL_DestroyTexture(base4);
    SDL_DestroyTexture(base3);
    SDL_DestroyTexture(base2);
    SDL_DestroyTexture(base1);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
