
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
#include <string.h>
#include <Windows.h>

// ============================================================
// DEFINES
// ============================================================

#define COLUNAS      25
#define LINHAS       19
#define MAX_PROJETEIS 20
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
    PAUSE,
    UPGRADE,
    CREDITOS,
    CONFIGURACOES,
    CONTROLES,
    RANKING,
    AJUDA,
    DIGITAR_NOME,
    RESULTADO,
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

    bool   morrendo;
    Uint64 flashInicio;
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

struct Audio
{
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 tamanho = 0;
    SDL_AudioStream* stream = nullptr;
};

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

// Funções de musica
bool musicaLigada = true;
bool efeitosLigados = true;

void TocarMusica(Audio& audio,
    const char* arquivo,
    float volume = 0.05f)
{
    if (!musicaLigada)
        return;

    if (audio.stream != nullptr)
        return;

    if (!SDL_LoadWAV(arquivo,
        &audio.spec,
        &audio.buffer,
        &audio.tamanho))
    {
        return;
    }

    audio.stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &audio.spec,
        NULL,
        NULL
    );

    SDL_SetAudioStreamGain(audio.stream, volume);

    SDL_PutAudioStreamData(
        audio.stream,
        audio.buffer,
        audio.tamanho
    );

    SDL_ResumeAudioStreamDevice(audio.stream);
}

void AtualizarMusica(Audio& audio)
{
    if (!musicaLigada)
        return;

    if (audio.stream == nullptr)
        return;

    if (SDL_GetAudioStreamQueued(audio.stream) == 0)
    {
        SDL_PutAudioStreamData(
            audio.stream,
            audio.buffer,
            audio.tamanho
        );
    }
}

void PararMusica(Audio& audio)
{
    if (audio.stream == nullptr)
        return;

    SDL_DestroyAudioStream(audio.stream);
    audio.stream = nullptr;

    SDL_free(audio.buffer);
    audio.buffer = nullptr;

    audio.tamanho = 0;
}

// Efeitos sonoros

void CarregarEfeito(Audio& audio,
    const char* arquivo,
    float volume = 0.05f)
{
    if (!SDL_LoadWAV(arquivo,
        &audio.spec,
        &audio.buffer,
        &audio.tamanho))
    {
        return;
    }

    audio.stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &audio.spec,
        NULL,
        NULL);

    SDL_SetAudioStreamGain(audio.stream, volume);
}

void TocarEfeito(Audio& audio)
{
    if (!efeitosLigados)
        return;

    if (audio.stream == nullptr)
        return;

    SDL_ClearAudioStream(audio.stream);

    SDL_PutAudioStreamData(
        audio.stream,
        audio.buffer,
        audio.tamanho);

    SDL_ResumeAudioStreamDevice(audio.stream);
}

void LiberarEfeito(Audio& audio)
{
    if (audio.stream)
    {
        SDL_DestroyAudioStream(audio.stream);
        audio.stream = nullptr;
    }

    if (audio.buffer)
    {
        SDL_free(audio.buffer);
        audio.buffer = nullptr;
    }

    audio.tamanho = 0;
}

// Botões
bool MouseNoBotao(float x, float y, SDL_FRect botao)
{
    return x >= botao.x &&
        x <= botao.x + botao.w &&
        y >= botao.y &&
        y <= botao.y + botao.h;
}

// Escala
const float LARGURA_BASE = 840.0f;
const float ALTURA_BASE = 480.0f;

float escalaJogo = 1.0f;

float gravidade = 0.5f;
float forcaPulo = -10.0f;

float SPEED_NORMAL = 5.0f;
float SPEED_SUJO = 2.5f;
float speed = 5.0f;

float tamanhoPlayer = 64.0f;
float tamanhoBoss = 128.0f;
float tamanhoInimigo = 64.0f;
float tamanhoPowerup = 32.0f;
float tamanhoProjetil = 32.0f;
float tamanhoObstaculo = 64.0f;

void AtualizarEscala(int larguraTela, int alturaTela)
{
    escalaJogo = SDL_min(
        larguraTela / LARGURA_BASE,
        alturaTela / ALTURA_BASE
    );

    tamanhoPlayer = 64.0f * escalaJogo;
    tamanhoBoss = 128.0f * escalaJogo;
    tamanhoInimigo = 64.0f * escalaJogo;
    tamanhoPowerup = 32.0f * escalaJogo;
    tamanhoProjetil = 32.0f * escalaJogo;
    tamanhoObstaculo = 64.0f * escalaJogo;

    SPEED_NORMAL = 5.0f * escalaJogo;
    SPEED_SUJO = 2.5f * escalaJogo;

    speed = SPEED_NORMAL;

    gravidade = 0.5f * escalaJogo;
    forcaPulo = -12.5f * escalaJogo;
}

// ============================================================
// MAIN
// ============================================================

int main(int argc, char* argv[])
{
#ifdef NDEBUG
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    // ----------------------------------------------------------
    // INICIALIZAÇÃO DO SDL
    // ----------------------------------------------------------

    //Icon
    SDL_Surface* icon = IMG_Load("assets/icon.ico");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Papa's Tia Bete",
        840,
        480,
        SDL_WINDOW_FULLSCREEN
    );

    if (icon)
    {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }

    if (!window)
    {
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if (!TTF_Init()) {
        return -1;
    }

    //Musicas
    SDL_AudioStream* streamMenu = NULL;

    // Fonte do jogo

    TTF_Font* font = TTF_OpenFont("assets/PixelifySans.ttf", 32);
    TTF_Font* fontHud = TTF_OpenFont("assets/PixelifySans.ttf", 18);

    // Textos da tela

    SDL_Color preto = { 0, 0, 0, 255 };
    SDL_Color branco = { 255, 255, 255, 255 };

    // ----------------------------------------------------------
    // ADAPTAR A TELA
    // ----------------------------------------------------------

    int larguraTela, alturaTela;

    SDL_GetWindowSize(window, &larguraTela, &alturaTela);

    AtualizarEscala(larguraTela, alturaTela);

    float tileW = (float)larguraTela / COLUNAS;
    float tileH = (float)alturaTela / LINHAS;

    // ----------------------------------------------------------
    // ESTADO DO SISTEMA / MENU
    // ----------------------------------------------------------

    Sistema estado = MENU;

    SDL_FRect botaoJogar;
    SDL_FRect botaoCreditos;
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

    botaoCreditos.w = botaoJogar.w;
    botaoCreditos.h = botaoJogar.h;
    botaoCreditos.x = botaoJogar.x;
    botaoCreditos.y = botaoJogar.y + botaoJogar.h + 20;

    botaoSair.w = botaoJogar.w;
    botaoSair.h = botaoJogar.h;
    botaoSair.x = botaoJogar.x;
    botaoSair.y = botaoCreditos.y + botaoCreditos.h + 20;

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
    // CARREGAMENTO DOS SPRITES
    // ----------------------------------------------------------

    SDL_Texture* background =
        IMG_LoadTexture(renderer, "assets/cozinha.png");

    SDL_Texture* backgroundNome =
        IMG_LoadTexture(renderer, "assets/background.png");

    SDL_Texture* menu =
        IMG_LoadTexture(renderer, "assets/menu.png");

    SDL_Texture* logo =
        IMG_LoadTexture(renderer, "assets/logo.png");

    SDL_Texture* jogar =
        IMG_LoadTexture(renderer, "assets/jogar.png");

    SDL_Texture* creditos =
        IMG_LoadTexture(renderer, "assets/creditos.png");

    SDL_Texture* sair =
        IMG_LoadTexture(renderer, "assets/sair.png");

    SDL_Texture* texturaInimigo =
        IMG_LoadTexture(renderer, "assets/acucar.png");

    SDL_Texture* texturaPlayer =
        IMG_LoadTexture(renderer, "assets/player.png");

    SDL_Texture* texturaPlayerSujo =
        IMG_LoadTexture(renderer, "assets/playerSujo.png");

    SDL_Texture* projetil =
        IMG_LoadTexture(renderer, "assets/tiro.png");

    SDL_Texture* gameover =
        IMG_LoadTexture(renderer, "assets/gameover.png");

    SDL_Texture* coracao =
        IMG_LoadTexture(renderer, "assets/coracao.png");

    SDL_Texture* powerUpAtq =
        IMG_LoadTexture(renderer, "assets/powerupatq.png");

    SDL_Texture* powerUpVida =
        IMG_LoadTexture(renderer, "assets/powerupvida.png");

    SDL_Texture* texturaTrofeu =
        IMG_LoadTexture(renderer, "assets/rankingaba.png");

    SDL_Texture* texturaEngrenagem =
        IMG_LoadTexture(renderer, "assets/configuracaoaba.png");

    SDL_Texture* texturaControles =
        IMG_LoadTexture(renderer, "assets/controlesaba.png");

    SDL_Texture* texturaAjuda =
        IMG_LoadTexture(renderer, "assets/interrogacaoaba.png");

    SDL_Texture* texturaBoss =
        IMG_LoadTexture(renderer, "assets/boss.png");

    SDL_Texture* texturaObstaculo =
        IMG_LoadTexture(renderer, "assets/obstaculo.png");

    SDL_Texture* texturaSombra =
        IMG_LoadTexture(renderer, "assets/sombra.png");

    SDL_Texture* baseTexture = IMG_LoadTexture(renderer, "assets/base.png");

    // Texturas das cartas de upgrade
    SDL_Texture* upgradeImgDano = IMG_LoadTexture(renderer, "assets/mais_dano.png");
    SDL_Texture* upgradeImgVida = IMG_LoadTexture(renderer, "assets/mais_vida.png");
    SDL_Texture* upgradeImgBalas = IMG_LoadTexture(renderer, "assets/mais_balas.png");
    SDL_Texture* upgradeImgDuplo = IMG_LoadTexture(renderer, "assets/tiro_duplo.png");
    SDL_Texture* upgradeImgCacador = IMG_LoadTexture(renderer, "assets/matador_de_boss.png");

    // Cursor

    SDL_Surface* cursorSurface = IMG_Load("assets/cursor.png");

    SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 0, 0);

    SDL_SetCursor(cursor);

    SDL_DestroySurface(cursorSurface);

    // ----------------------------------------------------------
    // VARIÁVEIS DO JOGADOR
    // ----------------------------------------------------------

    SDL_FRect player = {
        320.0f,
        240.0f,
        64,
        64
    };

    float velocidadeY = forcaPulo;
    float alturaChao = alturaTela - player.h;
    int framePowerUp = 0;
    int framePlayer = 0;
    int framePulo = 0;
    int danoTiro = 1;
    int danoTiroBase = 1;
    int vidaMaxima = MAX_VIDAS;
    int tirosExtras = 0;
    int quantidadeTiros = 3 + tirosExtras;
    const int totalFramesPowerUp = 5;
    const int larguraFramePowerUp = 64;
    const int alturaFramePowerUp = 64;
    bool  noChao = true;
    bool tiroDuplo = false;
    bool cacadorBoss = false;
    bool atirando = false;
    bool pulando = false;
    bool aterrissando = false;
    bool olhandoDireita = true;
    bool dano = false;
    bool stun = false;
    bool ataqueBuff = false;
    bool sujo = false;
    bool naBordaDir = false;


    Uint64 tempoBordaDir = 0;
    Uint64 ultimoFramePowerUp = 0;
    Uint64 sujoInicio = 0;
    Uint64 stunStart = 0;
    Uint64 danoStart = 0;
    Uint64 ultimoFramePlayer = 0;
    Uint64 inicioAterrissagem = 0;
    Uint64 ataqueBuffInicio = 0;
    const Uint64 LIMITE_BORDA = 2000;
    const Uint64 DURACAO_SUJO = 5000;
    const Uint64 STUN_DURACAO = 1250;
    const Uint64 DURACAO_BUFF = 10000;


    // Variavéis da base

    SDL_FRect base = {
    0.0f,
    0.0f,
    140.0f,
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
    PowerupVida vidasPowerup[MAX_POWERUPS] = { 0 };
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
    int opcaoPause = 0; // 0 = Continuar, 2 = Menu
    int opcaoUpgrade = 0;
    bool upgradeTiroDuploDisponivel = false;
    bool upgradeCacadordeBossDisponivel = false;

    const char* opcoesPause[] =
    {
        "Continuar",
        "Menu Principal"
    };

    float yCreditos = 0.0f;
    float shakeX = 0.0f;
    float shakeY = 0.0f;
    float shakeDuracao = 0.0f;
    float shakeForca = 0.0f;
    Uint64 shakeInicio = 0;

    Audio musicaMenu;
    Audio musicaGame;
    Audio musicaBoss;
    Audio musicaCreditos;
    Audio musicaGameOver;

    Audio somPulo;
    Audio somTiro;
    Audio somHoverBotao;
    Audio somHoverAbas;
    Audio somConfirm;
    Audio somPowerup;
    Audio somQuebra;
    Audio somAterrissagem;

    const int larguraFrameBase = 128;
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

    bool telaCheia = true;
    bool running = true;
    bool spam = false;

    int hoverBotaoAtual = -1;
    int hoverAbaAtual = -1;


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

    // Efeitos Sonoros
    CarregarEfeito(somPulo, "assets/pulo.wav");
    CarregarEfeito(somTiro, "assets/tiro.wav");
    CarregarEfeito(somHoverBotao, "assets/HoverBotao.wav");
    CarregarEfeito(somHoverAbas, "assets/HoverAbas.wav");
    CarregarEfeito(somConfirm, "assets/click.wav");
    CarregarEfeito(somPowerup, "assets/powerup.wav");
    CarregarEfeito(somQuebra, "assets/obstaculo.wav");
    CarregarEfeito(somAterrissagem, "assets/aterrissar.wav");

    // ----------------------------------------------------------
    // TEMPORIZAÇÃO DE SPAWN E ONDAS
    // ----------------------------------------------------------

    int intervaloSpawn = 2000 - (ondas * 150);
    Uint64 ultimoSpawn = SDL_GetTicks();
    Uint64 ultimoSpawnPowerUp = SDL_GetTicks();
    Uint64 ultimoSpawnAtq = SDL_GetTicks();
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

                if (MouseNoBotao(mouseX, mouseY, botaoJogar))
                {

                    TocarEfeito(somConfirm);
                    estado = JOGAR;
                    inicioOnda = SDL_GetTicks();
                    inicioSpam = SDL_GetTicks();
                    ultimoSpawn = SDL_GetTicks();
                    ultimoSpawnPowerUp = SDL_GetTicks();
                    ultimoSpawnAtq = SDL_GetTicks();
                    pontos = 0;
                    ondas = 1;
                    vidas = MAX_VIDAS;

                    // Upgrades
                    tiroDuplo = false;
                    cacadorBoss = false;
                    danoTiro = 1;
                    danoTiroBase = 1;
                    tirosExtras = 0;
                    quantidadeTiros = 3;
                    ataqueBuff = false;
                    upgradeTiroDuploDisponivel = false;
                    upgradeCacadordeBossDisponivel = false;
                    vidaMaxima = MAX_VIDAS;


                    player.x = 200.0f;
                    player.y = 240.0f;

                    player.w = tamanhoPlayer;
                    player.h = tamanhoPlayer;

                    speed = SPEED_NORMAL;
                    velocidadeY = 0.0f;

                    boss = { 0 };

                    spawnBoss = 1000;
                    bossAtivados = 0;

                    memset(inimigos, 0, sizeof(inimigos));
                    memset(projeteis, 0, sizeof(projeteis));
                    memset(vidasPowerup, 0, sizeof(vidasPowerup));
                    memset(ataquePowerup, 0, sizeof(ataquePowerup));
                    memset(obstaculos, 0, sizeof(obstaculos));
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
                        player.x = 200.0f;
                        player.y = 240.0f;
                        vidas = MAX_VIDAS;
                        velocidadeY = 0.0f;
                        noChao = true;
                        pontos = 0;
                        ondas = 1;
                        ondaAntiga = -1;
                        pontosAntigos = -1;
                        inimigosAtivos = 5;
                        boss.ativo = false;
                        boss.vida = 0;
                        bossAtivados = 0;
                        spawnBoss = 1000;
                        inimigosDerrotados = 0;
                        spam = false;
                        intervaloSpawn = 2000 - (ondas * 150);
                        tiroDuplo = false;
                        cacadorBoss = false;
                        danoTiro = 1;
                        danoTiroBase = 1;
                        tirosExtras = 0;
                        quantidadeTiros = 3;
                        ataqueBuff = false;
                        upgradeTiroDuploDisponivel = false;
                        upgradeCacadordeBossDisponivel = false;
                        vidaMaxima = MAX_VIDAS;
                        inicioOnda = SDL_GetTicks();
                        inicioSpam = SDL_GetTicks();
                        ultimoSpawn = SDL_GetTicks();
                        ultimoSpawnObstaculo = SDL_GetTicks();
                        ultimoSpawnPowerUp = SDL_GetTicks();
                        ultimoSpawnAtq = SDL_GetTicks();
                        memset(obstaculos, 0, sizeof(obstaculos));
                        memset(inimigos, 0, sizeof(inimigos));
                        for (int i = 0; i < MAX_PROJETEIS; i++) projeteis[i].ativo = false;

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

                logoRect.x = larguraTela * 0.05f;
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
        // Hover
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
            }

            hoverAbaAtual = novoHoverAba;
        }

        // Musica do Menu
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
                "assets/Gameover.wav");

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


        // Atualização da base
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
        // ------------------------------------------------------
        // SISTEMA DE ONDAS
        // ------------------------------------------------------

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

        if (estado == JOGAR && spam)
        {
            if (SDL_GetTicks() - inicioSpam >= 10000)
            {
                spam = false;

                intervaloSpawn = 2000 - (ondas * 150);

                if (intervaloSpawn < 500)
                    intervaloSpawn = 500;

                if (intervaloSpawn > 2000)
                    intervaloSpawn = 2000;
            }
        }

        // Acabar buffs
        if (estado == JOGAR) {
            if (ataqueBuff &&
                SDL_GetTicks() - ataqueBuffInicio >= DURACAO_BUFF)
            {
                ataqueBuff = false;
                danoTiro = danoTiroBase;
            }

            if (sujo &&
                SDL_GetTicks() - sujoInicio >= DURACAO_SUJO)
            {
                sujo = false;
                speed = SPEED_NORMAL;
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
            SDL_RenderTexture(renderer, texturaEngrenagem, NULL, &abaConfig);
            SDL_RenderTexture(renderer, texturaControles, NULL, &abaControles);
            SDL_RenderTexture(renderer, texturaTrofeu, NULL, &abaRanking);
            SDL_RenderTexture(renderer, texturaAjuda, NULL, &abaAjuda);

            //Desenhar logo e botões

            SDL_RenderTexture(renderer, logo, NULL, &logoRect);
            SDL_RenderTexture(renderer, jogar, NULL, &botaoJogar);
            SDL_RenderTexture(renderer, creditos, NULL, &botaoCreditos);
            SDL_RenderTexture(renderer, sair, NULL, &botaoSair);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }

        if (estado == CREDITOS) {
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

            yCreditos -= 1.0f;

            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;
            char texto[50];


            // Textos

            float espacamento = alturaTela * 0.07f;

            const char* creditos[] =
            {
                "Desenvolvido por:",
                "",


                "Direcao",
                "Davi Hyude Nunes de Almeida",
                "",

                "Programacao",
                "Davi Hyude Nunes de Almeida",
                "",

                "Game Design",
                "Davi Hyude Nunes de Almeida,",
                "Danah Bezerra de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Direcao de Arte",
                "Danah Bezerra de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Arte 2D",
                "Danah Bezerra de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Animacao",
                "Danah Bezerra de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Design de Interface (UI)",
                "Davi Hyude Nunes de Almeida,",
                "Danah Bezerra de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Efeitos Sonoros",
                "Davi Hyude Nunes de Almeida e",
                "João Pedro Souza de Oliveira",
                "",

                "Balanceamento",
                "Davi Hyude Nunes de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Testes",
                "Davi Hyude Nunes de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "Danah Bezerra de Almeida e"
                "Ian Victor de Miranda e Carvalho",
                "",


                "Suporte Moral",
                "Lucas Natanael de Oliveira Costa",
                "",

                "Apoio Psicologico",
                "Lucas Natanael de Oliveira Costa",
                "",

                "Responsaveis por Impedir",
                "os Desenvolvedores de Desistir",
                "Lucas Natanael de Oliveira Costa"
                ""
                "",


                "Ferramentas Utilizadas",
                "C++",
                "SDL3",
                "SDL_image",
                "SDL_ttf",
                "Microsoft Visual Studio 2026",
                "",

                "© 2026",
                "Davi Hyude Nunes de Almeida,",
                "Danah Bezerra de Almeida e",
                "Joao Pedro Souza de Oliveira",
                "",

                "Obrigado por jogar!"
            };

            int quantidadeLinhas = sizeof(creditos) / sizeof(creditos[0]);

            for (int i = 0; i < quantidadeLinhas; i++)
            {
                if (strlen(creditos[i]) == 0)
                    continue;

                SDL_Surface* surface = TTF_RenderText_Blended(
                    fontHud,
                    creditos[i],
                    0,
                    preto
                );

                SDL_Texture* texture =
                    SDL_CreateTextureFromSurface(renderer, surface);

                SDL_GetTextureSize(texture, &rect.w, &rect.h);

                rect.x = larguraTela / 2 - rect.w / 2;
                rect.y = yCreditos + i * espacamento;

                SDL_RenderTexture(renderer, texture, NULL, &rect);

                SDL_DestroySurface(surface);
                SDL_DestroyTexture(texture);
            }

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
                estado = RESULTADO;
            }

            SDL_Delay(16);
            continue;
        }

        if (estado == RESULTADO)
        {
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

            // Evento — Enter avança

            auto DrawCentrado = [&](TTF_Font* f, const char* txt, SDL_Color cor, float y) {
                SDL_Surface* s = TTF_RenderText_Blended(f, txt, 0, cor);
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                SDL_FRect r; SDL_GetTextureSize(t, &r.w, &r.h);
                r.x = larguraTela / 2.0f - r.w / 2.0f; r.y = y;
                SDL_RenderTexture(renderer, t, NULL, &r);
                SDL_DestroySurface(s); SDL_DestroyTexture(t);
                };

            SDL_Color amarelo = { 255, 220,  50, 255 };
            SDL_Color cinza = { 180, 180, 180, 255 };

            float cy = alturaTela * 0.15f;
            float esp = alturaTela * 0.10f;

            DrawCentrado(font, "RESULTADO", branco, cy);

            char buf[64];

            sprintf_s(buf, sizeof(buf), "Pontos:              %d", pontos);
            DrawCentrado(fontHud, buf, amarelo, cy + esp * 1.5f);

            sprintf_s(buf, sizeof(buf), "Onda alcancada:      %d", ondas);
            DrawCentrado(fontHud, buf, cinza, cy + esp * 2.2f);

            sprintf_s(buf, sizeof(buf), "Inimigos derrotados: %d", inimigosDerrotados);
            DrawCentrado(fontHud, buf, cinza, cy + esp * 2.9f);

            sprintf_s(buf, sizeof(buf), "Bosses derrotados:   %d", bossAtivados);
            DrawCentrado(fontHud, buf, cinza, cy + esp * 3.6f);

            DrawCentrado(fontHud, "ENTER - Continuar", branco, alturaTela * 0.82f);

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }
        // ------------------------------------------------------
        // CONTROLE DE SPAWN
        // ------------------------------------------------------

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

            if (!boss.ativo && tempoAtual - ultimoSpawn >= intervaloSpawn && vivos < inimigosAtivos)
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

                ataquePowerup[0].rect.w = tamanhoPowerup;
                ataquePowerup[0].rect.h = tamanhoPowerup;

                ataquePowerup[0].rect.x = larguraTela;
                ataquePowerup[0].rect.y =
                    rand() % (alturaTela - (int)ataquePowerup[0].rect.h);

                ataquePowerup[0].speed = 7.0f * escalaJogo;
            }
        }
        // ------------------------------------------------------
        // ATUALIZAÇÃO DE PROJETEIS
        // ------------------------------------------------------

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

        // ------------------------------------------------------
        // ATUALIZAÇÃO DE INIMIGOS
        // ------------------------------------------------------
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
        // Atualização dos powerups
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
        // ATUALIZAÇÃO DO BOSS
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
        // ------------------------------------------------------
        // COLISÃO: PLAYER E INIMIGOS
        // ------------------------------------------------------
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
        //Colisão do player e a base
        if (estado == JOGAR) {
            if (SDL_HasRectIntersectionFloat(
                &player,
                &base)) 
            {
                player.x = base.x + base.w;

            }
        }
        // Colisão do tiro com o powerup

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
                        if (vidas < MAX_VIDAS) {
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
        // Colisão do tiro com o boss
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
                        estado = UPGRADE;
                        upgradeTiroDuploDisponivel = !tiroDuplo && (rand() % 100 < 25);
                        upgradeCacadordeBossDisponivel = !cacadorBoss && (rand() % 100 < 5);
                        opcaoUpgrade = 0;
                        pontos += 100;
                        bossAtivados++;
                    }
                }
            }
        }
        // Colisão base e inimigos
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
                        }
                    }
                }
            }
        }
        // Colisão do boss com a base
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

                    if (vidas <= 0)
                    {
                        estado = GAMEOVER;
                        tempoAtual = SDL_GetTicks();
                    }
                }
            }

        }
        // PROJÉTEIS E OBSTÁCULOS

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
        // ------------------------------------------------------
        // COLISÃO: PROJETEIS E INIMIGOS
        // ------------------------------------------------------

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
        // ------------------------------------------------------
        // INPUT DO TECLADO
        // ------------------------------------------------------

        const bool* key = SDL_GetKeyboardState(NULL);

        float playerXAnterior = player.x;
        float playerYAnterior = player.y;
        // Movimentação e efeito de stun
        if (estado == JOGAR) {
            if (!stun && !dano) {

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
        // Atualização da animação do player

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
        // Atualizando animação de dano
        if (estado == JOGAR) {
            if (dano)
            {
                Uint64 tempo = SDL_GetTicks() - danoStart;

                if (olhandoDireita)
                {
                    if (tempo < 300)
                        framePlayer = 13;
                    else if (tempo < 600)
                        framePlayer = 14;
                    else if (tempo < 900)
                        framePlayer = 15;
                    else
                        dano = false;
                }
                else
                {
                    if (tempo < 300)
                        framePlayer = 27;
                    else if (tempo < 600)
                        framePlayer = 28;
                    else if (tempo < 900)
                        framePlayer = 29;
                    else
                        dano = false;
                }
            }
        }
        //------------------------------------------------------
        // Colisões da tela
        //------------------------------------------------------

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
        // ------------------------------------------------------
        // FÍSICA: GRAVIDADE E CHÃO
        // ------------------------------------------------------
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

        // Obstaculos
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
                            player.x -= 35.0f;
                            velocidadeY = -3.5f;
                        }
                        else
                        {
                            player.x += 35.0f;
                            velocidadeY = -3.5f;
                        }

                        noChao = false;
                        pulando = true;
                    }
                }
            }
        }

        // Animação de pulo
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
        if (estado == JOGAR) {
            if (!boss.ativo && pontos >= spawnBoss) {
                boss.ativo = true;

                shakeDuracao = 600.0f;
                shakeForca = 8.0f * escalaJogo;
                shakeInicio = SDL_GetTicks();

                boss.rect.w = tamanhoBoss;
                boss.rect.h = tamanhoBoss;

                boss.rect.x = larguraTela;
                
                float maxYBoss = alturaChao - boss.rect.h;
                if (maxYBoss < 0) maxYBoss = 0;
                boss.rect.y = (float)(rand() % (int)(maxYBoss + 1));

                boss.speed = 1.5f + (bossAtivados * 0.5f) * escalaJogo;

                boss.vidaMax = 60 + (bossAtivados * 5);
                boss.vida = boss.vidaMax;

                boss.frame = 0;
                boss.ultimoFrame = SDL_GetTicks();
                spawnBoss += 1000;
            }
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

        // Calcula offset do shake
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

        // Limpa com cor de fundo
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
            frameIndex * larguraFrameBase,
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
            framePlayer * 64,
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
                projeteis[i].frame * larguraFrameTiro,
                0,
                larguraFrameTiro,
                alturaFrameTiro
            };

            SDL_RenderTexture(renderer, projetil, &srcTiro, &projeteis[i].rect);
        }

        // Inimigos
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (!inimigos[i].ativo && !inimigos[i].morrendo)
                continue;

            if (inimigos[i].morrendo)
                SDL_SetTextureColorMod(texturaInimigo, 255, 80, 80);

            SDL_FRect src = {
                inimigos[i].frame * larguraFrameInimigo,
                0,
                larguraFrameInimigo,
                alturaFrameInimigo
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
                obstaculos[i].frame * 64,
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
                    obstaculos[i].frame * 64,
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
                boss.frame * 128,
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

            SDL_FRect src = {
                vidasPowerup[i].frame * larguraFramePowerUp,
                0,
                larguraFramePowerUp,
                alturaFramePowerUp
            };

            SDL_FRect dest = vidasPowerup[i].rect;
            dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

            SDL_RenderTexture(renderer, powerUpVida, &src, &dest);
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

            SDL_RenderTexture(renderer, powerUpAtq, &src, &dest);
        }

        // -------------------------------------------------------
        // Reseta viewport — HUD fixo
        // -------------------------------------------------------
        SDL_SetRenderViewport(renderer, NULL);

        // Barra de vida do boss
        if (boss.ativo)
        {
            SDL_FRect fundoVida = {
                larguraTela / 2 - 200,
                20,
                400,
                25
            };

            SDL_FRect vidaBoss = {
                larguraTela / 2 - 200,
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

        // Buff
        if (ataqueBuff)
        {
            SDL_FRect buffRect;
            SDL_GetTextureSize(texturaBuff, &buffRect.w, &buffRect.h);
            buffRect.x = 10;
            buffRect.y = 50;
            SDL_RenderTexture(renderer, texturaBuff, NULL, &buffRect);
        }

        // Upgrade
        if (estado == UPGRADE)
        {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
            SDL_FRect telaInteira = { 0, 0, (float)larguraTela, (float)alturaTela };
            SDL_RenderFillRect(renderer, &telaInteira);

            struct SlotUpgrade {
                const char* label;
                const char* desc;
                SDL_Texture* img;
            };

            SlotUpgrade slots[5];
            int totalSlots = 0;

            slots[totalSlots++] = { "MAIS DANO",      "+1 de dano por tiro",      upgradeImgDano };
            slots[totalSlots++] = { "MAIS VIDA",      "+1 de vida maxima",         upgradeImgVida };
            slots[totalSlots++] = { "MAIS BALAS",     "+1 projetil simultaneo",    upgradeImgBalas };

            if (upgradeTiroDuploDisponivel)
                slots[totalSlots++] = { "TIRO DUPLO",      "Atira 2 projeteis por vez", upgradeImgDuplo };

            if (upgradeCacadordeBossDisponivel)
                slots[totalSlots++] = { "CACADOR DE BOSS", "+5 de dano nos bosses",     upgradeImgCacador };

            const float CARTA_W = larguraTela * 0.14f;
            const float CARTA_H = alturaTela * 0.62f;
            const float CARTA_GAP = larguraTela * 0.025f;
            const float IMG_H = CARTA_H * 0.42f;
            const float BORDA = 4.0f;

            float blocoW = totalSlots * CARTA_W + (totalSlots - 1) * CARTA_GAP;
            float blocoX = larguraTela / 2.0f - blocoW / 2.0f;
            float blocoY = alturaTela * 0.18f;

            {
                float titleY = blocoY - alturaTela * 0.09f;
                if (opcaoUpgrade >= 0 && opcaoUpgrade < totalSlots)
                    titleY -= alturaTela * 0.04f;

                SDL_Surface* s = TTF_RenderText_Blended(font, "ESCOLHA UMA MELHORIA", 0, branco);
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                SDL_FRect r;
                SDL_GetTextureSize(t, &r.w, &r.h);
                r.x = larguraTela / 2.0f - r.w / 2.0f;
                r.y = titleY;
                SDL_RenderTexture(renderer, t, NULL, &r);
                SDL_DestroySurface(s);
                SDL_DestroyTexture(t);
            }

            for (int i = 0; i < totalSlots; i++)
            {
                bool selecionada = (i == opcaoUpgrade);

                float cx = blocoX + i * (CARTA_W + CARTA_GAP);
                float cy = selecionada ? blocoY - alturaTela * 0.04f : blocoY;

                SDL_FRect cartaRect = { cx, cy, CARTA_W, CARTA_H };

                SDL_FRect sombraRect = { cx + 6, cy + 6, CARTA_W, CARTA_H };
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
                SDL_RenderFillRect(renderer, &sombraRect);

                SDL_Color corFundo = selecionada
                    ? SDL_Color{ 255, 230, 160, 255 }
                : SDL_Color{ 40,  35,  50, 255 };

                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                SDL_SetRenderDrawColor(renderer, corFundo.r, corFundo.g, corFundo.b, 255);
                SDL_RenderFillRect(renderer, &cartaRect);

                SDL_Color corBorda = selecionada
                    ? SDL_Color{ 220, 160,  20, 255 }
                : SDL_Color{ 100,  80, 130, 255 };

                SDL_SetRenderDrawColor(renderer, corBorda.r, corBorda.g, corBorda.b, 255);
                SDL_RenderRect(renderer, &cartaRect);

                SDL_FRect bordaInterna = {
                    cartaRect.x + BORDA, cartaRect.y + BORDA,
                    cartaRect.w - BORDA * 2, cartaRect.h - BORDA * 2
                };
                SDL_RenderRect(renderer, &bordaInterna);

                if (slots[i].img)
                {
                    float imgPad = CARTA_W * 0.08f;
                    SDL_FRect imgRect = {
                        cx + imgPad,
                        cy + CARTA_H * 0.10f,
                        CARTA_W - imgPad * 2,
                        IMG_H
                    };
                    SDL_RenderTexture(renderer, slots[i].img, NULL, &imgRect);
                }

                {
                    SDL_Color corTxt = selecionada
                        ? SDL_Color{ 20,  20,  20, 255 }
                    : SDL_Color{ 255, 255, 255, 255 };

                    SDL_Surface* s = TTF_RenderText_Blended(fontHud, slots[i].label, 0, corTxt);
                    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                    SDL_FRect r;
                    SDL_GetTextureSize(t, &r.w, &r.h);
                    float maxW = CARTA_W - 8.0f;
                    if (r.w > maxW) { float f = maxW / r.w; r.w *= f; r.h *= f; }
                    r.x = cx + CARTA_W / 2.0f - r.w / 2.0f;
                    r.y = cy + CARTA_H * 0.10f + IMG_H + CARTA_H * 0.04f;
                    SDL_RenderTexture(renderer, t, NULL, &r);
                    SDL_DestroySurface(s);
                    SDL_DestroyTexture(t);
                }

                {
                    SDL_Color corDesc = selecionada
                        ? SDL_Color{ 60,  40,  10, 255 }
                    : SDL_Color{ 180, 160, 200, 255 };

                    SDL_Surface* s = TTF_RenderText_Blended(fontHud, slots[i].desc, 0, corDesc);
                    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                    SDL_FRect r;
                    SDL_GetTextureSize(t, &r.w, &r.h);
                    float maxW = CARTA_W - 8.0f;
                    if (r.w > maxW) { float f = maxW / r.w; r.w *= f; r.h *= f; }
                    r.x = cx + CARTA_W / 2.0f - r.w / 2.0f;
                    r.y = cy + CARTA_H * 0.10f + IMG_H + CARTA_H * 0.14f;
                    SDL_RenderTexture(renderer, t, NULL, &r);
                    SDL_DestroySurface(s);
                    SDL_DestroyTexture(t);
                }

                if (selecionada)
                {
                    SDL_SetRenderDrawColor(renderer, 220, 160, 20, 255);
                    float ax = cx + CARTA_W / 2.0f;
                    float ay = cy - 18.0f;
                    SDL_RenderLine(renderer, ax - 10, ay, ax + 10, ay);
                    SDL_RenderLine(renderer, ax - 6, ay + 6, ax, ay + 12);
                    SDL_RenderLine(renderer, ax + 6, ay + 6, ax, ay + 12);
                }
            }

            {
                SDL_Surface* s = TTF_RenderText_Blended(fontHud,
                    "A / D  —  Navegar          ENTER  —  Confirmar", 0, branco);
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                SDL_FRect r;
                SDL_GetTextureSize(t, &r.w, &r.h);
                r.x = larguraTela / 2.0f - r.w / 2.0f;
                r.y = (float)alturaTela - r.h - 18.0f;
                SDL_RenderTexture(renderer, t, NULL, &r);
                SDL_DestroySurface(s);
                SDL_DestroyTexture(t);
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }

        // Pause
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

            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_FRect rect;

            surface = TTF_RenderText_Blended(font, "PAUSADO", 0, branco);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_GetTextureSize(texture, &rect.w, &rect.h);
            rect.x = painel.x + painel.w / 2 - rect.w / 2;
            rect.y = painel.y + 30;
            SDL_RenderTexture(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
            SDL_DestroySurface(surface);

            for (int i = 0; i < 2; i++)
            {
                SDL_Color cor = (i == opcaoPause) ? preto : branco;
                surface = TTF_RenderText_Blended(font, opcoesPause[i], 0, cor);
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_GetTextureSize(texture, &rect.w, &rect.h);
                rect.x = painel.x + painel.w / 2 - rect.w / 2;
                rect.y = painel.y + 120 + i * 70;
                SDL_RenderTexture(renderer, texture, NULL, &rect);
                SDL_DestroyTexture(texture);
                SDL_DestroySurface(surface);
            }
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
    SDL_DestroyTexture(baseTexture);
    SDL_DestroyTexture(backgroundNome);
    SDL_DestroyTexture(texturaPlayerSujo);
    SDL_DestroyTexture(powerUpAtq);
    SDL_DestroyTexture(powerUpVida);
    SDL_DestroyTexture(texturaBoss);
    SDL_DestroyTexture(texturaObstaculo);
    SDL_DestroyTexture(texturaSombra);
    SDL_DestroyTexture(upgradeImgDano);
    SDL_DestroyTexture(upgradeImgVida);
    SDL_DestroyTexture(upgradeImgBalas);
    SDL_DestroyTexture(upgradeImgDuplo);
    SDL_DestroyTexture(upgradeImgCacador);
    SDL_DestroyCursor(cursor);

    LiberarEfeito(somPulo);
    LiberarEfeito(somTiro);
    LiberarEfeito(somHoverBotao);
    LiberarEfeito(somHoverAbas);
    LiberarEfeito(somConfirm);
    LiberarEfeito(somPowerup);
    LiberarEfeito(somQuebra);
    LiberarEfeito(somAterrissagem);

    TTF_CloseFont(font);
    TTF_CloseFont(fontHud);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
