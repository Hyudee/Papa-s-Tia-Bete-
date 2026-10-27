
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

// ============================================================
// MAIN
// ============================================================

bool Jogo::Iniciar()
{
    // ----------------------------------------------------------
    // INICIALIZAÇÃO DO SDL
    // ----------------------------------------------------------

    //Icon
    SdlPtr<SDL_Surface> icon(IMG_Load("assets/icon.ico"));

    if (!icon)
        SDL_Log("Falha ao carregar o icone assets/icon.ico: %s", SDL_GetError());

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("Falha ao iniciar o SDL: %s", SDL_GetError());
        return false;
    }
    sdlGuard.emplace();

    windowOwner.reset(SDL_CreateWindow(
        "Papa's Tia Bete",
        840,
        480,
        SDL_WINDOW_FULLSCREEN
    ));
    window = windowOwner.get();

    if (!window)
    {
        SDL_Log("Falha ao criar a janela: %s", SDL_GetError());
        return false;
    }

    SDL_SetWindowMouseGrab(window, true);

    if (icon)
    {
        SDL_SetWindowIcon(window, icon.get());
        icon.reset();
    }

    rendererOwner.reset(SDL_CreateRenderer(window, NULL));
    renderer = rendererOwner.get();

    if (!renderer)
    {
        SDL_Log("Falha ao criar o renderer: %s", SDL_GetError());
        return false;
    }   

    if (!TTF_Init()) {
        SDL_Log("Falha ao iniciar o SDL_ttf: %s", SDL_GetError());
        return false;
    }
    ttfGuard.emplace();

    //Musicas
    streamMenu = NULL;

    // Fonte do jogo

    fontOwner.reset(TTF_OpenFont("assets/PixelifySans.ttf", 32));
    font = fontOwner.get();
    fontHudOwner.reset(TTF_OpenFont("assets/PixelifySans.ttf", 18));
    fontHud = fontHudOwner.get();

    if (!font || !fontHud)
        SDL_Log("Falha ao carregar a fonte assets/PixelifySans.ttf: %s", SDL_GetError());

    SDL_GetWindowSize(window, &larguraTela, &alturaTela);

    AtualizarEscala(larguraTela, alturaTela);
    tileW = (float)larguraTela / COLUNAS;
    tileH = (float)alturaTela / LINHAS;

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

    srand((unsigned)time(NULL));
    CarregarRanking(ranking);

    assets.Carregar(renderer);
    background = assets.background.get();
    backgroundNome = assets.backgroundNome.get();
    menu = assets.menu.get();
    logo = assets.logo.get();
    jogar = assets.jogar.get();
    creditos = assets.creditos.get();
    sair = assets.sair.get();
    texturaInimigo = assets.texturaInimigo.get();
    texturasInimigo[INIMIGO_COMUM] = texturaInimigo;
    for (int t = INIMIGO_COMUM + 1; t < TOTAL_TIPOS_INIMIGO; t++)
        texturasInimigo[t] = assets.texturasInimigo[t].get();
    texturaDivisaoMarshmallow = assets.texturaDivisaoMarshmallow.get();
    texturaTiroCalda = assets.texturaTiroCalda.get();
    texturaTiroCaldaImpacto = assets.texturaTiroCaldaImpacto.get();
    texturaPedacoChocolate = assets.texturaPedacoChocolate.get();
    texturaPedacoImpacto = assets.texturaPedacoImpacto.get();
    texturaFaiscaEscudo = assets.texturaFaiscaEscudo.get();
    texturaPlayer = assets.texturaPlayer.get();    
    texturaPlayerSujo = assets.texturaPlayerSujo.get();
    projetil = assets.projetil.get();
    gameover = assets.gameover.get();
    coracao = assets.coracao.get();
    powerUpAtq = assets.powerUpAtq.get();
    powerUpVida = assets.powerUpVida.get();
    texturaTrofeu = assets.texturaTrofeu.get();
    texturaEngrenagem = assets.texturaEngrenagem.get();
    texturaControles = assets.texturaControles.get();
    texturaAjuda = assets.texturaAjuda.get();
    texturaBoss = assets.texturaBoss.get();
    for (int a = ANIM_BOSS_NENHUMA + 1; a < TOTAL_ANIM_BOSS; a++)
        texturasBoss[a] = assets.texturasBoss[a].get();
    texturaObstaculo = assets.texturaObstaculo.get();
    texturaSombra = assets.texturaSombra.get();
    baseTexture = assets.baseTexture.get();
    upgradeImgDano = assets.upgradeImgDano.get();
    upgradeImgVida = assets.upgradeImgVida.get();
    upgradeImgBalas = assets.upgradeImgBalas.get();
    upgradeImgDuplo = assets.upgradeImgDuplo.get();
    upgradeImgCacador = assets.upgradeImgCacador.get();
    upgradeImgCadencia = assets.upgradeImgCadencia.get();
    upgradeImgBalaPesada = assets.upgradeImgBalaPesada.get();
    upgradeImgPernas = assets.upgradeImgPernas.get();
    upgradeImgDoceSorte = assets.upgradeImgDoceSorte.get();
    upgradeImgGlace = assets.upgradeImgGlace.get();
    upgradeImgPerfurante = assets.upgradeImgPerfurante.get();
    upgradeImgBarreira = assets.upgradeImgBarreira.get();
    upgradeImgExplosao = assets.upgradeImgExplosao.get();
    upgradeImgFuria = assets.upgradeImgFuria.get();
    upgradeImgPacto = assets.upgradeImgPacto.get();

    // Cursor

    SDL_Surface* cursorSurface = IMG_Load("assets/cursor.png");

    if (!cursorSurface)
        SDL_Log("Falha ao carregar o cursor assets/cursor.png: %s", SDL_GetError());

    cursor.reset(SDL_CreateColorCursor(cursorSurface, 0, 0));

    SDL_SetCursor(cursor.get());

    SDL_DestroySurface(cursorSurface);

    velocidadeY = forcaPulo;
    alturaChao = alturaTela - player.h;
    quantidadeTiros = 3 + tirosExtras;
    base = {
    0.0f,
    0.0f,
    180.0f * escalaJogo,
    (float)alturaTela
    };

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

    intervaloSpawn = IntervaloSpawn(ondas);
    ultimoSpawn = SDL_GetTicks();
    ultimoSpawnPowerUp = SDL_GetTicks();
    ultimoSpawnAtq = SDL_GetTicks();
    intervaloSpawnPowerUp = 20000 + (ondas * 150);
    ultimoSpawnObstaculo = SDL_GetTicks();
    inicioOnda = SDL_GetTicks();
    inicioSpam = SDL_GetTicks();

    return true;
}

// Segura o laço em FPS_ALVO quadros por segundo, qualquer que seja o tempo gasto no quadro.
// Substitui o SDL_Delay(16), que somava 16 ms ao tempo do quadro (FPS dependia da máquina).
void Jogo::EsperarProximoFrame()
{
    const Uint64 duracaoFrameNs = 1000000000ULL / FPS_ALVO;
    Uint64 agora = SDL_GetTicksNS();

    if (proximoFrameNs == 0)
        proximoFrameNs = agora;

    proximoFrameNs += duracaoFrameNs;

    if (agora < proximoFrameNs)
    {
        SDL_DelayPrecise(proximoFrameNs - agora);
    }
    else if (agora - proximoFrameNs > 5 * duracaoFrameNs)
    {
        // Atrasou muito (janela arrastada, carregamento): não tenta recuperar os quadros perdidos
        proximoFrameNs = agora;
    }
}

void Jogo::Executar()
{
    // ==========================================================
    // LOOP PRINCIPAL
    // ==========================================================

    while (running)
    {
        // ------------------------------------------------------
        // PROCESSAMENTO DE EVENTOS
        // ------------------------------------------------------

        ProcessarEventos();
        AtualizarHoverMenu();
        GerenciarMusica();
        AtualizarAnimacoes();
        metricas.AtualizarFrame(estado == JOGAR);

        // ------------------------------------------------------
        // SISTEMA DE ONDAS
        // ------------------------------------------------------

        AtualizarOndasEBuffs();

        if (estado == JOGAR && spam)
        {
            if (SDL_GetTicks() - inicioSpam >= 10000)
            {
                spam = false;

                intervaloSpawn = IntervaloSpawn(ondas);
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

        if (TelaMenu()) continue;
        if (TelaCreditos()) continue;
        if (TelaRanking()) continue;
        if (TelaDigitarNome()) continue;
        if (TelaConfiguracoes()) continue;
        if (TelaControles()) continue;
        if (TelaAjuda()) continue;
        if (TelaGameOver()) continue;
        if (TelaResultado()) continue;
        
        AtualizarSpawn();
        AtualizarProjeteis();
        AtualizarInimigos();
        AtualizarTirosInimigos();
        AtualizarPowerups();
        AtualizarBoss();
        AtualizarObstaculos();
        ColisaoPlayerInimigos();
        ColisaoTirosInimigosPlayer();
        ColisaoPlayerBase();
        ColisaoTiroPowerups();
        ColisaoTiroBoss();
        ColisaoBaseInimigos();
        ColisaoBossBase();
        ColisaoProjeteisObstaculos();
        ColisaoProjeteisInimigos();
        
        LerTeclado();
        MoverJogador();
        AnimarJogador();
        AnimarDano();
        ColisoesTela();

        
        //FÍSICA: GRAVIDADE E CHÃO

        FisicaJogador();
        ColisaoJogadorObstaculos();
        AnimarPulo();
        AtualizarTexturaPontos();
        SpawnarBoss();
        ProcessarLevelUps();
        AtualizarTexturaOnda();
        AtualizarBarraXp();

        // ------------------------------------------------------
        // RENDERIZAÇÃO
        // ------------------------------------------------------

        AtualizarUpgrade();
        CalcularShake();
        RenderizarMundo();
        RenderizarHud();
        if (RenderizarUpgrade()) continue;
        RenderizarPause();

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();

} // fim do loop principal

    // ==========================================================
    // LIMPEZA E ENCERRAMENTO
    // ==========================================================

    // Destruir texturas


    SDL_DestroyTexture(texturaPontos);
    SDL_DestroyTexture(TexturaOndas);
    SDL_DestroyTexture(texturaBuff);

    PararMusica(musicaMenu);
    PararMusica(musicaGame);
    PararMusica(musicaBoss);
    PararMusica(musicaCreditos);
    PararMusica(musicaGameOver);

    LiberarEfeito(somPulo);
    LiberarEfeito(somTiro);
    LiberarEfeito(somHoverBotao);
    LiberarEfeito(somHoverAbas);
    LiberarEfeito(somConfirm);
    LiberarEfeito(somPowerup);
    LiberarEfeito(somQuebra);
    LiberarEfeito(somAterrissagem);

}
