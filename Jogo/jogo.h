#pragma once

#include <optional>

#include "config.h"
#include "sdl_raii.h"
#include "assets.h"
#include "tipos.h"
#include "audio.h"
#include "metricas.h"
#include "curva.h"
#include "inimigos.h"
#include "cartas.h"
#include "anim_cartas.h"

// Estado e lógica do jogo.
// As variáveis que eram locais de main() agora são membros desta classe, na mesma ordem.
// O código do loop principal continua o mesmo (agora em Jogo::Executar).

class Jogo
{
public:
    bool Iniciar();   // inicialização (tudo que vinha antes do loop principal)
    void Executar();  // loop principal + finalização

private:

    // Estado da partida
    void ReiniciarPartida();
    void DeslocarTimers(Uint64 delta);   // empurra os temporizadores pelo tempo em que o jogo ficou parado

    // ---- SDL / janela / fontes (na ordem de declaracao original; define a ordem de destruicao) ----
    std::optional<SdlGuard> sdlGuard;
    SdlPtr<SDL_Window> windowOwner;
    SDL_Window* window = nullptr;
    SdlPtr<SDL_Renderer> rendererOwner;
    SDL_Renderer* renderer = nullptr;
    std::optional<TtfGuard> ttfGuard;

    SDL_AudioStream* streamMenu = NULL;

    SdlPtr<TTF_Font> fontOwner;
    TTF_Font* font = nullptr;
    SdlPtr<TTF_Font> fontHudOwner;
    TTF_Font* fontHud = nullptr;

    // Textos da tela

    SDL_Color preto = { 0, 0, 0, 255 };
    SDL_Color branco = { 255, 255, 255, 255 };
    SDL_Color corDestaqueOuro = { 255, 215, 0, 255 };
    SDL_Color corDestaqueMarrom = { 200, 110, 20, 255 };
    const float margemDestaqueX = 24.0f;
    const float margemDestaqueY = 6.0f;

    // ADAPTAR A TELA

    int larguraTela;
    int alturaTela;
    float tileW;
    float tileH;

    // ESTADO DO SISTEMA / MENU

    Sistema estado = MENU;
    SDL_FRect botaoJogar;
    SDL_FRect botaoCreditos;
    SDL_FRect botaoSair;
    SDL_FRect logoRect;
    SDL_FRect abaConfig;
    SDL_FRect abaControles;
    SDL_FRect abaRanking;
    SDL_FRect abaAjuda;
    float textoW;
    float textoH;
    SDL_FRect textoRect;
    float larguraAba;
    float alturaAba;
    float padding;
    Ranking ranking[3];
    char nomeJogador[20] = "";
    int tamanhoNome = 0;

    // CARREGAMENTO DOS SPRITES

    Assets assets;
    SDL_Texture* background;
    SDL_Texture* backgroundNome;
    SDL_Texture* menu;
    SDL_Texture* logo;
    SDL_Texture* jogar;
    SDL_Texture* creditos;
    SDL_Texture* sair;
    SDL_Texture* texturaInimigo;
    SDL_Texture* texturasInimigo[TOTAL_TIPOS_INIMIGO] = { 0 };   // por tipo; [0] = texturaInimigo
    SDL_Texture* texturaDivisaoMarshmallow = NULL;   // animacao de divisao do Divisor
    SDL_Texture* texturaPlayer;
    SDL_Texture* texturaPlayerSujo;
    SDL_Texture* projetil;
    SDL_Texture* gameover;
    SDL_Texture* coracao;
    SDL_Texture* powerUpAtq;
    SDL_Texture* powerUpVida;
    SDL_Texture* texturaTrofeu;
    SDL_Texture* texturaEngrenagem;
    SDL_Texture* texturaControles;
    SDL_Texture* texturaAjuda;
    SDL_Texture* texturaBoss;
    SDL_Texture* texturaObstaculo;
    SDL_Texture* texturaSombra;
    SDL_Texture* baseTexture;

    // Texturas das cartas de upgrade
    SDL_Texture* 
        Dano;
    SDL_Texture* upgradeImgDano;
    SDL_Texture* upgradeImgVida;
    SDL_Texture* upgradeImgBalas;
    SDL_Texture* upgradeImgDuplo;
    SDL_Texture* upgradeImgCacador;
    SDL_Texture* upgradeImgCadencia;
    SDL_Texture* upgradeImgBalaPesada;
    SDL_Texture* upgradeImgPernas;
    SDL_Texture* upgradeImgDoceSorte;
    SDL_Texture* upgradeImgGlace;
    SDL_Texture* upgradeImgPerfurante;
    SDL_Texture* upgradeImgBarreira;
    SDL_Texture* upgradeImgExplosao;
    SDL_Texture* upgradeImgFuria;
    SDL_Texture* upgradeImgPacto;
    SdlPtr<SDL_Cursor> cursor;

    // VARIÁVEIS DO JOGADOR

    SDL_FRect player = {
        320.0f,
        240.0f,
        64,
        64
    };
    float velocidadeY;
    float alturaChao;
    int framePowerUp = 0;
    int framePlayer = 0;
    int framePulo = 0;
    int danoTiro = 1;
    int danoTiroBase = 1;
    int vidaMaxima = MAX_VIDAS;
    int tirosExtras = 0;
    int quantidadeTiros;
    const int totalFramesPowerUp = 5;
    const int larguraFramePowerUp = 64;
    const int alturaFramePowerUp = 64;
    bool noChao = true;
    bool tiroDuplo = false;
    ColecaoCartas cartas;       // cartas escolhidas nesta partida
    bool cacadorBoss = false;
    int cooldownTiro = COOLDOWN_TIRO_BASE;   // ms mínimos entre disparos (CADENCIA e BALA PESADA)
    int velocidadeTiroPercent = 100;         // BALA PESADA
    int velocidadeJogadorPercent = 100;      // PERNAS DE ACUCAR
    int knockbackPercent = 100;              // PERNAS DE ACUCAR
    int velocidadeCoracaoPercent = 100;      // DOCE SORTE
    int intervaloCoracaoPercent = 100;       // DOCE SORTE
    int lentidaoBossPercent = 0; // GLACÊ
    int perfuracoesTiro = 0;                 // PERFURANTE4
    int barreirasMax = 0;                    // BARREIRA DA BASE (cargas máximas)
    int explosaoChancePercent = 0;           // EXPLOSAO ACUCARADA
    int furiaDanoPercent = 0;                // FURIA DE COMBO
    bool atirando = false;
    bool pulando = false;
    bool aterrissando = false;
    bool olhandoDireita = true;
    bool dano = false;
    bool stun = false;
    bool ataqueBuff = false;
    bool sujo = false;
    bool naBordaDir = false;

    // Eventos
    void ProcessarEventos();

    // Tiro
    void DispararTiro(float mouseX, float mouseY);
    int DisparosAtivos() const;

    // Menu principal (Enter e clique esquerdo usam a mesma confirmação)
    int ItemMenuSobMouse(float x, float y) const;
    const SDL_FRect* RetanguloItemMenu(int item) const;
    void MoverItemMenu(int vertical, int horizontal);
    void ConfirmarItemMenu(int item);

    // Limpa a janela (tarjas pretas) e pinta a área do jogo com a cor de desenho atual
    void LimparTela();

    // Destaque arredondado
    void DesenharDestaque(const SDL_FRect& alvo, float margemX, float margemY, SDL_Color cor) const;

    bool MouseNaOpcao(float x, float y, const SDL_FRect& textoOpcao) const;
    SDL_FRect RetanguloOpcaoPause(int i) const;
    SDL_FRect RetanguloOpcaoConfig(int i) const;
    void TextoOpcaoConfig(int i, char* destino, size_t tamanho) const;
    void ConfirmarOpcaoPause();
    void AlternarOpcaoConfig();
    SDL_FRect RetanguloBotaoResultado() const;
    void ConfirmarResultado();

    // Atualização do jogo
    void AtualizarHoverMenu();
    void GerenciarMusica();
    void AtualizarAnimacoes();
    void AtualizarOndasEBuffs();

    void AtualizarSpawn();
    void AtualizarProjeteis();
    void AtualizarInimigos();
    bool SpawnarInimigo(TipoInimigo tipo, float x, float y);   // false se nao houver vaga
    void DividirDivisor(SDL_FRect pai);                        // cria os filhotes do Divisor que acabou de morrer
    void AtualizarPowerups();
    void AtualizarBoss();
    void AtualizarObstaculos();

    // Colisões
    void ColisaoPlayerInimigos();
    void ColisaoPlayerBase();
    void ColisaoTiroPowerups();
    void ColisaoTiroBoss();
    void ColisaoBaseInimigos();
    void ColisaoBossBase();
    void ColisaoProjeteisObstaculos();
    void ColisaoProjeteisInimigos();

    // Fim de partida
    void RegistrarFimDePartida(const char* causa);
    void EsperarProximoFrame();

    // Cartas
    void AplicarCartas();
    void EscolherCarta(IdCarta id);
    SDL_Texture* ImagemDaCarta(IdCarta id) const;
    int RerollsDisponiveis() const;
    void RefazerOferta();
    void AtualizarEfeitosDasCartas();   // recarga da Barreira, fim da Furia e das explosoes
    void ExplodirAcucar(float centroX, float centroY);
    void RegistrarAbate();              // conta um abate para a Furia
    void QuebrarSequenciaAbates();      // o jogador ou a base foi atingido
    int DanoAtualDoTiro() const;        // dano do tiro agora (com a Furia, se ativa)

    // Teclado, movimento e física do jogador
    void LerTeclado();
    void MoverJogador();
    void AnimarJogador();
    void AnimarDano();
    void ColisoesTela();
    void FisicaJogador();
    void ColisaoJogadorObstaculos();
    void AnimarPulo();
    void AtualizarTexturaPontos();
    void SpawnarBoss();
    void AtualizarTexturaOnda();

    // Renderização
    void CalcularShake();
    void RenderizarMundo();
    void RenderizarHud();
    bool RenderizarUpgrade();   // true = tela de upgrade tratada neste frame
    void RenderizarPause();


    // Tela de escolha de cartas (jogo_upgrade.cpp)
    void IniciarTelaCartas();
    void AtualizarUpgrade();
    void ConstruirTexturasCartas();
    void DesenharCartaNaTextura(SDL_Texture* alvo, IdCarta id, bool destacada);
    void EncerrarTelaCartas();
    void ConfirmarCarta(int indice);
    void SelecionarCarta(int indice);
    void DesenharBrilhoCarta(const SDL_FRect& destino, Raridade raridade, float alfa, Uint64 agora);
    void DesenharBurstCarta(const SDL_FRect& destino, Raridade raridade, int quadro);

    // Telas de estado (true = tela tratada neste frame)
    bool TelaMenu();
    bool TelaCreditos();
    bool TelaRanking();
    bool TelaDigitarNome();
    bool TelaConfiguracoes();
    bool TelaControles();
    bool TelaAjuda();
    bool TelaGameOver();
    bool TelaResultado();

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
    const Uint64 DURACAO_GLACE = 1500;   // ms de lentidão do boss depois de cada acerto com Glacê
    const Uint64 RECARGA_BARREIRA = 25000;   // ms para recuperar 1 carga da Barreira
    const Uint64 DURACAO_EXPLOSAO = 250;     // ms da animação da explosão
    const Uint64 DURACAO_FURIA = 8000;       // ms de dano extra depois de completar a sequência
    const int ABATES_FURIA = 10;             // abates seguidos para ativar a Fúria
    const float RAIO_EXPLOSAO = 48.0f;       // raio da explosão, antes de multiplicar por escalaJogo
    static const int MAX_EXPLOSOES = 8;      // explosões visíveis ao mesmo tempo

    // Variavéis da base

    SDL_FRect base;
    float tempoFlashBase = 0.0f;
    FlashBase flashBase = FLASH_NENHUM;

    //Inimigos
    SDL_FRect srcInimigo = {
        0,
        0,
        32,
        32
    };

    // VARIÁVEIS DE JOGO

    Projetil projeteis[MAX_PROJETEIS] = { 0 };
    Inimigo inimigos[MAX_INIMIGOS] = { 0 };
    TipoInimigo tiposAtivos[TOTAL_TIPOS_INIMIGO] = { INIMIGO_COMUM };   // tipos que nascem nesta onda (Lote 13c)
    int totalTiposAtivos = 1;    
    Boss boss = { 0 };
    PowerupVida vidasPowerup[MAX_POWERUPS] = { 0 };
    PowerupAtq ataquePowerup[MAX_POWERUPSATQ] = { 0 };
    Obstaculo obstaculos[MAX_OBSTACULOS] = { 0 };
    int mapa[LINHAS][COLUNAS] = { 0 };
    int pontos = 0;
    int pontosAntigos = -1;
    int ondas = 1;
    int ondaAntiga = -1;
    int vidas = MAX_VIDAS;
    int inimigosDerrotados = 0;
    int inimigosAtivos = 5;
    int spawnBoss = 1000;
    Uint64 fimUltimoBoss = 0;   // quando o último boss morreu ou chegou à base
    int bossAtivados = 0;
    int frameBase = 0;
    int opcaoConfig = 0;  // 0 = Tela cheia, 1 = Música, 2 = Efeitos
    int opcaoPause = 0;  // 0 = Continuar, 2 = Menu
    int opcaoUpgrade = 0;
    IdCarta ofertaCartas[NUM_CARTAS];   // cartas mostradas na tela de escolha
    int totalOfertaCartas = 0;
    const char* opcoesPause[2] = { "Continuar", "Menu Principal" };
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
    // Item destacado no menu principal (teclado ou mouse):
    // 0 Jogar, 1 Créditos, 2 Sair, 3 Engrenagem, 4 Controles, 5 Troféu, 6 Ajuda
    int itemMenu = 0;

    Metricas metricas;            // FPS e registro das partidas (metricas.csv)
    Uint64 proximoFrameNs = 0;    // limitador de quadros
    Uint64 inicioResultado = 0; // quando a tela de resultado abriu

    // TEMPORIZAÇÃO DE SPAWN E ONDAS

    int intervaloSpawn;
    Uint64 ultimoSpawn;
    Uint64 ultimoSpawnPowerUp;
    Uint64 ultimoSpawnAtq;
    Uint64 intervaloSpawnPowerUp;
    int sorteioIntervaloCoracao = 100;   // 70 a 130: sorteio do tempo até o próximo coração (não é instante)
    Uint64 intervaloSpawnAtq = 40000;
    Uint64 ultimoSpawnObstaculo;
    Uint64 intervaloObstaculo = 15000;
    Uint64 tempoAtual = 0;
    Uint64 inicioOnda;
    Uint64 inicioSpam;
    Uint64 ultimoFrameBase = 0;
    SDL_Event event;

    Uint64 inicioPausa = 0;   // quando o jogo entrou em Pause ou Upgrade

    // Teclado e posição anterior do jogador (usados por mais de uma etapa do frame)
    const bool* key = nullptr;
    float playerXAnterior = 0.0f;
    float playerYAnterior = 0.0f;

    int rerollsUsados = 0;      // trocas de cartas já gastas nesta partida

    // Estado das cartas Barreira, Fúria e Explosão (os instantes entram em DeslocarTimers)
    int barreiraCargas = 0;
    Uint64 inicioRecargaBarreira = 0;
    Explosao explosoes[MAX_EXPLOSOES] = { 0 };
    int sequenciaAbates = 0;
    bool furiaAtiva = false;
    Uint64 furiaInicio = 0;

    Uint64 ultimoTiro = 0;      // instante do último disparo (cooldown)
    int proximoIdDisparo = 1;   // id do próximo disparo (não é tempo, não precisa de DeslocarTimers)


    // Tela de escolha de cartas. Fica por último de propósito: as texturas precisam ser
    // destruídas antes do renderer. Os instantes de animCartas NÃO entram em DeslocarTimers.
    AnimCartas animCartas;
    LayoutCartas layoutCartas = {};
    bool cartasProntas = false;
    int larguraCartasConstruida = 0;
    int alturaCartasConstruida = 0;
    SdlPtr<SDL_Texture> cartaNormal[CARTAS_POR_OFERTA];
    SdlPtr<SDL_Texture> cartaDestacada[CARTAS_POR_OFERTA];
    SdlPtr<SDL_Texture> textoTituloCartas;
    SdlPtr<SDL_Texture> textoDicaCartas;
};