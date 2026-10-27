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
    sujo = false;   // não leva o obstáculo grudento para a partida nova
    velocidadeY = 0.0f;
    noChao = true;

    // Placar e progressão
    pontos = 0;
    nivel = 1;
    xpAtual = 0;
    levelUpsPendentes = 0;
    nivelExibido = 1;
    xpFracaoExibida = 0.0f;
    xpAgitacao = 0.0f;
    xpFlashMs = 0.0f;
    xpBarraY = 20.0f;
    nivelTextoXp = -1;
    ondas = 1;
    ondaAntiga = -1;
    pontosAntigos = -1;
    vidas = MAX_VIDAS;
    vidaMaxima = MAX_VIDAS;
    inimigosAtivos = MaximoInimigosVivos(ondas);
    inimigosDerrotados = 0;
    contadores = {};
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
    memset(explosoes, 0, sizeof(explosoes));
    sequenciaAbates = 0;
    furiaAtiva = false;
    furiaInicio = 0;

    // Tiros
    ultimoTiro = SDL_GetTicks();

    // Boss
    boss = { 0 };
    memset(faiscas, 0, sizeof(faiscas));
    spawnBoss = 1000;
    ultimoTipoBoss = -1;
    bossAtivados = 0;
    fimUltimoBoss = SDL_GetTicks();

    // Temporizadores
    inicioOnda = SDL_GetTicks();
    inicioSpam = SDL_GetTicks();
    ultimoSpawn = SDL_GetTicks();
    ultimoSpawnObstaculo = SDL_GetTicks();
    ultimoSpawnPowerUp = SDL_GetTicks();
    sorteioIntervaloCoracao = 100;
    ultimoSpawnAtq = SDL_GetTicks();
    intervaloSpawnAtq = 40000;

    // Entidades
    memset(inimigos, 0, sizeof(inimigos));
    memset(tirosInimigos, 0, sizeof(tirosInimigos));
    stunDuracaoAtual = STUN_DURACAO;
    danoDuracaoAtual = DURACAO_DANO;
    tiposAtivos[0] = INIMIGO_COMUM;     // onda 1: so minion comum
    totalTiposAtivos = 1;    
    memset(projeteis, 0, sizeof(projeteis));
    memset(vidasPowerup, 0, sizeof(vidasPowerup));
    memset(ataquePowerup, 0, sizeof(ataquePowerup));
    memset(obstaculos, 0, sizeof(obstaculos));

    metricas.IniciarPartida();
    PreAquecerCartas();
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
    explosaoChancePercent = stats.explosaoChancePercent;
    furiaDanoPercent = stats.furiaDanoPercent;

    danoTiro = ataqueBuff ? danoTiroBase * 2 : danoTiroBase;
}

// Aplica a carta escolhida na tela de upgrade
void Jogo::EscolherCarta(IdCarta id)
{
    if (!cartas.Adicionar(id))
        return;

    AplicarCartas();

    // PACTO AMARGO baixa a vida máxima: a vida atual não pode passar dela
    if (vidas > vidaMaxima)
        vidas = vidaMaxima;

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

    // Fúria: termina depois de DURACAO_FURIA
    if (furiaAtiva && agora - furiaInicio >= DURACAO_FURIA)
        furiaAtiva = false;

    for (int i = 0; i < MAX_EXPLOSOES; i++)
    {
        if (explosoes[i].ativa && agora - explosoes[i].inicio >= DURACAO_EXPLOSAO)
            explosoes[i].ativa = false;
    }
}

// Conta um abate com tiro; ao chegar em ABATES_FURIA seguidos, liga a Fúria
void Jogo::RegistrarAbate()
{
    if (furiaDanoPercent <= 0)
        return;

    sequenciaAbates++;

    if (sequenciaAbates >= ABATES_FURIA)
    {
        sequenciaAbates = 0;
        furiaAtiva = true;
        furiaInicio = SDL_GetTicks();   // completar outra sequência renova os 8 s
    }
}

// O jogador foi atingido ou a base perdeu vida: a sequência recomeça (a Fúria já ativa continua)
void Jogo::QuebrarSequenciaAbates()
{
    sequenciaAbates = 0;
}

// Dano do tiro neste instante: o dano normal (já com o buff de ataque) mais o bônus da Fúria
int Jogo::DanoAtualDoTiro() const
{
    if (!furiaAtiva || furiaDanoPercent <= 0)
        return danoTiro;

    // Arredondado só com inteiros: com dano 1 o bônus de 25% ainda dá 1
    return (danoTiro * (100 + furiaDanoPercent) + 50) / 100;
}

// Explosão Açucarada: mata os minions em volta do ponto, sem pontos e sem encadear outra explosão
void Jogo::ExplodirAcucar(float centroX, float centroY)
{
    float raio = RAIO_EXPLOSAO * escalaJogo;
    Uint64 agora = SDL_GetTicks();

    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        if (!inimigos[i].ativo || inimigos[i].morrendo)
            continue;

        float dx = inimigos[i].rect.x + inimigos[i].rect.w / 2.0f - centroX;
        float dy = inimigos[i].rect.y + inimigos[i].rect.h / 2.0f - centroY;
        float alcance = raio + inimigos[i].rect.w / 2.0f;

        if (dx * dx + dy * dy <= alcance * alcance)
        {
            inimigos[i].morrendo = true;
            inimigos[i].flashInicio = agora;
            inimigos[i].vida = 0;   // a explosao mata qualquer tipo
            contadores.abatesExplosao[inimigos[i].tipo]++;
            if (!inimigos[i].invocado)
                DarXp(DadosDoInimigo(inimigos[i].tipo).xp);
        }
    }

    // Efeito visual (usa a primeira vaga livre; se não houver, só não desenha)
    for (int i = 0; i < MAX_EXPLOSOES; i++)
    {
        if (explosoes[i].ativa)
            continue;

        explosoes[i].ativa = true;
        explosoes[i].x = centroX;
        explosoes[i].y = centroY;
        explosoes[i].raio = raio;
        explosoes[i].inicio = agora;
        break;
    }
}

// Soma XP; o excedente transborda para o proximo nivel. Um ganho pode render varios niveis.
void Jogo::DarXp(int quantidade)
{
    if (quantidade <= 0)
        return;

    xpAtual += quantidade;

    while (xpAtual >= XpParaProximoNivel(nivel))
    {
        xpAtual -= XpParaProximoNivel(nivel);
        nivel++;
        levelUpsPendentes++;
    }
}

// Anima a barra de XP. So avanca em JOGAR: durante a tela de cartas fica parada
// e o preenchimento/flash acontecem quando o jogo volta.
void Jogo::AtualizarBarraXp()
{
    Uint64 agora = SDL_GetTicks();
    float dt = (ultimoTickXp == 0) ? 0.0f : (float)(agora - ultimoTickXp);
    ultimoTickXp = agora;

    if (dt > 50.0f)
        dt = 50.0f;

    if (estado != JOGAR)
        return;

    // Desce para baixo da barra do boss (y 20 + altura 25 + folga) e volta quando ele morre
    float alvoY = boss.ativo ? 53.0f : 20.0f;
    xpBarraY += (alvoY - xpBarraY) * (1.0f - expf(-dt / 80.0f));

    if (nivelExibido > nivel)
    {
        nivelExibido = nivel;
        xpFracaoExibida = 0.0f;
    }

    // Subindo de nivel: enche ate o topo, esvazia e continua; senao vai ate o XP real
    bool subindo = nivelExibido < nivel;
    float alvo = subindo ? 1.0f : XpFracaoBarra();
    float diff = alvo - xpFracaoExibida;

    if (diff > 0.0f)
    {
        float passo = SDL_max(diff * 6.0f, 0.4f) * dt / 1000.0f;
        xpFracaoExibida = (passo >= diff) ? alvo : xpFracaoExibida + passo;
    }
    else if (diff < 0.0f)
    {
        xpFracaoExibida = alvo;
    }

    // Onda na ponta do liquido: forte enquanto enche, acalma ao chegar
    float alvoAgitacao = (diff > 0.0f) ? SDL_max(0.25f, SDL_min(1.0f, diff * 12.0f)) : 0.0f;
    xpAgitacao += (alvoAgitacao - xpAgitacao) * (1.0f - expf(-dt / 120.0f));

    if (subindo && xpFracaoExibida >= 1.0f)
    {
        nivelExibido++;
        xpFracaoExibida = 0.0f;
        xpFlashMs = DURACAO_FLASH_XP;
    }

    if (xpFlashMs > 0.0f)
        xpFlashMs = SDL_max(0.0f, xpFlashMs - dt);
}

// Roda uma vez por quadro. Consome 1 level up pendente: abre a tela de cartas
// (ou, sem cartas elegiveis, so consome o pendente).
void Jogo::ProcessarLevelUps()
{
    if (estado != JOGAR || levelUpsPendentes <= 0)
        return;

    levelUpsPendentes--;

    totalOfertaCartas = MontarOfertaCartas(cartas, bossAtivados, ofertaCartas, CARTAS_POR_OFERTA);
    opcaoUpgrade = 0;

    if (totalOfertaCartas > 0)
    {
        inicioPausa = SDL_GetTicks();
        estado = UPGRADE;
        IniciarTelaCartas();
    }
}

// Grava a partida que acabou em metricas.csv (a própria Metricas ignora chamadas repetidas)
void Jogo::RegistrarFimDePartida(const char* causa)
{
    ResumoPartida resumo;

    resumo.onda = ondas;
    resumo.pontos = pontos;
    resumo.nivel = nivel;
    levelUpsPendentes = 0;
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
    resumo.explosao = cartas.Quantidade(CARTA_EXPLOSAO);
    resumo.furia = cartas.Quantidade(CARTA_FURIA);
    resumo.pacto = cartas.Quantidade(CARTA_PACTO_AMARGO);
    resumo.contadores = contadores;

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
    furiaInicio += delta;

    // Animações
    ultimoFramePlayer += delta;
    ultimoFramePowerUp += delta;
    ultimoFrameBase += delta;

    // Entidades
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        inimigos[i].ultimoFrame += delta;
        inimigos[i].flashInicio += delta;
        inimigos[i].hitInicio += delta;
        inimigos[i].ultimoDisparo += delta;
        inimigos[i].disparoInicio += delta;
    }

    for (int i = 0; i < MAX_TIROS_INIMIGOS; i++)
        tirosInimigos[i].ultimoFrame += delta;

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
    boss.estadoInicio += delta;
    boss.proximaInvocacao += delta;
    boss.proximoArremesso += delta;

    for (int i = 0; i < MAX_EXPLOSOES; i++)
        explosoes[i].inicio += delta;

    for (int i = 0; i < MAX_FAISCAS_ESCUDO; i++)
        faiscas[i].inicio += delta;
}