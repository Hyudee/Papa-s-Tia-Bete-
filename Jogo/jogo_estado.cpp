#include <string.h>
#include "escala.h"
#include "jogo.h"

// Reinício de partida: tudo que precisa voltar ao valor inicial quando uma nova partida começa.
// (Une os dois resets que existiam: o do botão "Jogar" e o da tela de resultado.)
void Jogo::ReiniciarPartida()
{
    // Jogador
    player.x = 200.0f;
    player.y = 240.0f;
    player.w = tamanhoPlayer;
    player.h = tamanhoPlayer;
    speed = SPEED_NORMAL;
    velocidadeY = 0.0f;
    noChao = true;

    // Placar e progressão
    pontos = 0;
    ondas = 1;
    ondaAntiga = -1;
    pontosAntigos = -1;
    vidas = MAX_VIDAS;
    vidaMaxima = MAX_VIDAS;
    inimigosAtivos = MaximoInimigosVivos(ondas);
    inimigosDerrotados = 0;
    spam = false;
    intervaloSpawn = IntervaloSpawn(ondas);

    // Upgrades
    cartas.Limpar();
    ataqueBuff = false;
    AplicarCartas();
    totalOfertaCartas = 0;
    rerollsUsados = 0;
    barreiraCargas = 0;
    inicioRecargaBarreira = SDL_GetTicks();

    // Tiros
    ultimoTiro = SDL_GetTicks();

    // Boss
    boss = { 0 };
    spawnBoss = 1000;
    bossAtivados = 0;
    fimUltimoBoss = SDL_GetTicks();

    // Temporizadores
    inicioOnda = SDL_GetTicks();
    inicioSpam = SDL_GetTicks();
    ultimoSpawn = SDL_GetTicks();
    ultimoSpawnObstaculo = SDL_GetTicks();
    ultimoSpawnPowerUp = SDL_GetTicks();
    ultimoSpawnAtq = SDL_GetTicks();
    intervaloSpawnAtq = 40000;

    // Entidades
    memset(inimigos, 0, sizeof(inimigos));
    memset(projeteis, 0, sizeof(projeteis));
    memset(vidasPowerup, 0, sizeof(vidasPowerup));
    memset(ataquePowerup, 0, sizeof(ataquePowerup));
    memset(obstaculos, 0, sizeof(obstaculos));

    metricas.IniciarPartida();
}

// Recalcula tudo que as cartas mudam a partir da coleção de cartas
void Jogo::AplicarCartas()
{
    StatsJogador stats = CalcularStats(cartas);

    danoTiroBase = stats.danoBase;
    vidaMaxima = stats.vidaMaxima;
    tirosExtras = stats.tirosExtras;
    quantidadeTiros = stats.quantidadeTiros;
    tiroDuplo = stats.tiroDuplo;
    cacadorBoss = stats.cacadorBoss;
    cooldownTiro = stats.cooldownTiro;
    velocidadeTiroPercent = stats.velocidadeTiroPercent;
    velocidadeJogadorPercent = stats.velocidadeJogadorPercent;
    knockbackPercent = stats.knockbackPercent;
    velocidadeCoracaoPercent = stats.velocidadeCoracaoPercent;
    intervaloCoracaoPercent = stats.intervaloCoracaoPercent;
    lentidaoBossPercent = stats.lentidaoBossPercent;
    perfuracoesTiro = stats.perfuracoes;
    barreirasMax = stats.barreiras;

    danoTiro = ataqueBuff ? danoTiroBase * 2 : danoTiroBase;
}

// Aplica a carta escolhida na tela de upgrade
void Jogo::EscolherCarta(IdCarta id)
{
    if (!cartas.Adicionar(id))
        return;

    AplicarCartas();

    // MAIS VIDA sobe a vida máxima em 1 e a vida atual em 1 (não recupera tudo)
    if (id == CARTA_MAIS_VIDA && vidas < vidaMaxima)
        vidas++;


    // BARREIRA DA BASE: a carta nova ja vem carregada
    if (id == CARTA_BARREIRA && barreiraCargas < barreirasMax)
        barreiraCargas++;
}

// Trocas de cartas: 1 por partida, mais 1 a cada 5 bosses derrotados
int Jogo::RerollsDisponiveis() const
{
    return 1 + bossAtivados / 5 - rerollsUsados;
}

// Sorteia uma oferta nova (gasta uma troca)
void Jogo::RefazerOferta()
{
    if (RerollsDisponiveis() <= 0)
        return;

    rerollsUsados++;

    totalOfertaCartas = MontarOfertaCartas(cartas, bossAtivados, ofertaCartas, CARTAS_POR_OFERTA);
    opcaoUpgrade = 0;
    IniciarTelaCartas();
}

// Recarga da Barreira, fim da Fúria e fim das explosões (chamado a cada quadro)
void Jogo::AtualizarEfeitosDasCartas()
{
    if (estado != JOGAR)
        return;

    Uint64 agora = SDL_GetTicks();

    // Barreira: recupera 1 carga a cada RECARGA_BARREIRA enquanto faltar alguma
    if (barreiraCargas < barreirasMax && agora - inicioRecargaBarreira >= RECARGA_BARREIRA)
    {
        barreiraCargas++;
        inicioRecargaBarreira = agora;
    }
}

// Grava a partida que acabou em metricas.csv (a própria Metricas ignora chamadas repetidas)
void Jogo::RegistrarFimDePartida(const char* causa)
{
    ResumoPartida resumo;

    resumo.onda = ondas;
    resumo.pontos = pontos;
    resumo.bossesDerrotados = bossAtivados;
    resumo.causa = causa;
    resumo.danoBase = danoTiroBase;
    resumo.tirosExtras = tirosExtras;
    resumo.vidaMaxima = vidaMaxima;
    resumo.tiroDuplo = tiroDuplo;
    resumo.cacadorBoss = cacadorBoss;
    resumo.cadencia = cartas.Quantidade(CARTA_CADENCIA);
    resumo.balaPesada = cartas.Quantidade(CARTA_BALA_PESADA);
    resumo.pernasAcucar = cartas.Quantidade(CARTA_PERNAS_ACUCAR);
    resumo.doceSorte = cartas.Quantidade(CARTA_DOCE_SORTE);
    resumo.glace = cartas.Quantidade(CARTA_GLACE);
    resumo.perfurante = cartas.Quantidade(CARTA_PERFURANTE);
    resumo.barreira = cartas.Quantidade(CARTA_BARREIRA);
    resumo.larguraTela = larguraTela;
    resumo.alturaTela = alturaTela;

    metricas.FinalizarPartida(resumo);
}

// Pause e Upgrade param o jogo, mas SDL_GetTicks() continua correndo.
// Ao voltar ao jogo, todos os instantes guardados são empurrados pelo tempo parado,
// para que ondas, buffs, spawns, stun, obstáculos e animações continuem de onde pararam.
void Jogo::DeslocarTimers(Uint64 delta)
{
    // Partida
    inicioOnda += delta;
    inicioSpam += delta;
    ultimoSpawn += delta;
    ultimoSpawnPowerUp += delta;
    ultimoSpawnAtq += delta;
    ultimoSpawnObstaculo += delta;
    fimUltimoBoss += delta;

    // Jogador
    ataqueBuffInicio += delta;
    sujoInicio += delta;
    stunStart += delta;
    danoStart += delta;
    tempoBordaDir += delta;
    inicioAterrissagem += delta;
    shakeInicio += delta;
    ultimoTiro += delta;
    inicioRecargaBarreira += delta;

    // Animações
    ultimoFramePlayer += delta;
    ultimoFramePowerUp += delta;
    ultimoFrameBase += delta;

    // Entidades
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        inimigos[i].ultimoFrame += delta;
        inimigos[i].flashInicio += delta;
    }

    for (int i = 0; i < MAX_PROJETEIS; i++)
        projeteis[i].ultimoFrame += delta;

    for (int i = 0; i < MAX_POWERUPS; i++)
        vidasPowerup[i].ultimoFrame += delta;

    for (int i = 0; i < MAX_POWERUPSATQ; i++)
        ataquePowerup[i].ultimoFrame += delta;

    for (int i = 0; i < MAX_OBSTACULOS; i++)
    {
        obstaculos[i].ultimoFrame += delta;
        obstaculos[i].inicioEstado += delta;
    }

    boss.ultimoFrame += delta;
    boss.lentidaoFim += delta;
}