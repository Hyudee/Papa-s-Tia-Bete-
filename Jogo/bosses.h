#pragma once

// Tabela de bosses (so dados, como o curva.h e o inimigos.h).
// Para ajustar o balanceamento dos bosses, mexa so aqui.
//
// REGRA: o valor 0 de TipoBoss e de EstadoBoss precisa ser o boss chocolate
// e o estado "andando", porque ReiniciarPartida faz boss = { 0 }.

#include <stdlib.h>
#include "inimigos.h"

enum TipoBoss
{
    BOSS_CHOCOLATE = 0,
    BOSS_TOURO,         // Touro de Goma: investida
    BOSS_CHURROS,       // Churros Mago: invoca minions
    BOSS_BROWNIE,       // Brownie: escudo
    TOTAL_TIPOS_BOSS    // sempre o ultimo
};

enum EstadoBoss
{
    BOSS_ANDANDO = 0,       // estado normal (todos os tipos)
    BOSS_AVISANDO,          // Touro: para e treme antes de investir (13g)
    BOSS_INVESTINDO,        // Touro: corrida em linha reta (13g)
    BOSS_INVOCANDO,         // Churros: para e invoca (13h)
    BOSS_ESCUDO_ENTRANDO,   // Brownie: transicao vulneravel -> escudo (13i)
    BOSS_ESCUDO,            // Brownie: invulneravel e lento (13i)
    BOSS_ESCUDO_SAINDO      // Brownie: transicao escudo -> vulneravel (13i)
};

// Uma folha de arte por animacao (linha unica, frames de 128x128)
enum AnimBossId
{
    ANIM_BOSS_NENHUMA = 0,          // [0] nao e usado
    ANIM_CHOCOLATE_ANDAR,           // assets/boss.png
    ANIM_TOURO_ANDAR,               // assets/boss_touro_andar.png
    ANIM_TOURO_CARREGAR,            // assets/boss_touro_carregar.png
    ANIM_TOURO_INVESTIDA,           // assets/boss_touro_investida.png
    ANIM_CHURROS_ANDAR,             // assets/boss_churros_andar.png
    ANIM_CHURROS_INVOCAR,           // assets/boss_churros_invocar.png
    ANIM_BROWNIE_ANDAR,             // assets/boss_brownie_andar.png
    ANIM_BROWNIE_TRANSICAO,         // assets/boss_brownie_transicao.png
    ANIM_BROWNIE_ESCUDO,            // assets/boss_brownie_escudo.png
    TOTAL_ANIM_BOSS
};

struct AnimBoss
{
    int frames;     // frames na folha
    int msFrame;    // duracao de cada frame, em ms
    unsigned char corR, corG, corB;   // colormod provisorio, usado so se o arquivo de arte nao existir
};

inline constexpr int LARGURA_FRAME_BOSS = 128;   // largura e altura de um frame na folha

// Ordem das linhas = ordem do enum AnimBossId
inline constexpr AnimBoss TABELA_ANIM_BOSS[TOTAL_ANIM_BOSS] =
{
    // frames msFrame  cor provisoria
    { 1, 120,  255, 255, 255 },   // NENHUMA (nao usada)
    { 6, 120,  255, 255, 255 },   // CHOCOLATE andar (velocidade original: 120 ms)
    { 6, 120,  230, 190, 120 },   // TOURO andar
    { 4, 120,  230, 120,  80 },   // TOURO carregar
    { 4, 120,  230, 120,  80 },   // TOURO investida
    { 6, 120,  240, 200, 120 },   // CHURROS andar
    { 6, 120,  240, 200, 120 },   // CHURROS invocar (6 x 120 = 0,72 s)
    { 6, 120,  140,  90,  60 },   // BROWNIE andar
    { 4, 120,  140,  90,  60 },   // BROWNIE transicao
    { 6, 200,  140,  90,  60 },   // BROWNIE escudo (animacao mais lenta que o andar)
};

// Quantos tipos entram no sorteio (os primeiros do enum). Sobe a cada boss
// que ganha comportamento: 13g -> 2 (Touro, pronto), 13h -> 3 (Churros, pronto), 13i -> 4 (Brownie, pronto).
inline constexpr int BOSSES_LIBERADOS = 4;

// Sorteio a cada spawn de boss: qualquer tipo liberado, sem restricao por onda
// e sem rotacao (P7). 'aleatorio' existe so para o teste (nullptr = rand()).
inline TipoBoss SortearTipoBoss(int liberados = BOSSES_LIBERADOS, int (*aleatorio)(int limite) = nullptr)
{
    if (liberados < 1) liberados = 1;
    if (liberados > TOTAL_TIPOS_BOSS) liberados = TOTAL_TIPOS_BOSS;

    int n = aleatorio ? aleatorio(liberados) : rand() % liberados;
    if (n < 0) n = 0;
    if (n >= liberados) n = liberados - 1;

    return (TipoBoss)n;
}

// Qual folha de arte desenhar para este tipo e estado
inline AnimBossId AnimacaoDoBoss(TipoBoss tipo, EstadoBoss estado)
{
    switch (tipo)
    {
    case BOSS_TOURO:
        if (estado == BOSS_AVISANDO)   return ANIM_TOURO_CARREGAR;
        if (estado == BOSS_INVESTINDO) return ANIM_TOURO_INVESTIDA;
        return ANIM_TOURO_ANDAR;

    case BOSS_CHURROS:
        if (estado == BOSS_INVOCANDO)  return ANIM_CHURROS_INVOCAR;
        return ANIM_CHURROS_ANDAR;

    case BOSS_BROWNIE:
        if (estado == BOSS_ESCUDO_ENTRANDO || estado == BOSS_ESCUDO_SAINDO) return ANIM_BROWNIE_TRANSICAO;
        if (estado == BOSS_ESCUDO)     return ANIM_BROWNIE_ESCUDO;
        return ANIM_BROWNIE_ANDAR;

    default:
        return ANIM_CHOCOLATE_ANDAR;
    }
}


// ---- Touro de Goma: investida (Lote 13g) ----
inline constexpr float TOURO_GATILHO_PROGRESSO = 0.35f;   // investe ao passar de 35% do caminho ate a base
inline constexpr unsigned long long TOURO_MS_AVISO = 800;    // parado e tremendo
inline constexpr unsigned long long TOURO_MS_INVESTIDA = 1000;   // duracao da corrida; depois anda normal
inline constexpr float TOURO_FATOR_INVESTIDA = 2.5f;     // multiplica a velocidade do boss

// Quanto do caminho (da borda direita da tela ate a borda direita da base) o boss ja andou: 0 a 1
inline float ProgressoBoss(float bossX, float larguraTela, float baseDireita)
{
    float total = larguraTela - baseDireita;
    if (total <= 0.0f) return 1.0f;

    float p = (larguraTela - bossX) / total;
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    return p;
}

// Maquina de estados do Touro: devolve o proximo estado (igual ao atual = sem mudanca).
// ANDANDO -> AVISANDO (so uma vez, ao passar do gatilho) -> INVESTINDO -> ANDANDO
inline EstadoBoss TransicaoTouro(EstadoBoss estado, bool jaInvestiu, float progresso,
    unsigned long long agora, unsigned long long estadoInicio)
{
    switch (estado)
    {
    case BOSS_ANDANDO:
        if (!jaInvestiu && progresso >= TOURO_GATILHO_PROGRESSO) return BOSS_AVISANDO;
        return BOSS_ANDANDO;

    case BOSS_AVISANDO:
        if (agora - estadoInicio >= TOURO_MS_AVISO) return BOSS_INVESTINDO;
        return BOSS_AVISANDO;

    case BOSS_INVESTINDO:
        if (agora - estadoInicio >= TOURO_MS_INVESTIDA) return BOSS_ANDANDO;
        return BOSS_INVESTINDO;

    default:
        return estado;
    }
}

// ---- Brownie: escudo (Lote 13i) ----
inline constexpr unsigned long long BROWNIE_MS_VULNERAVEL = 6000;   // andando e vulneravel
inline constexpr unsigned long long BROWNIE_MS_ESCUDO = 4000;   // escudo ativo (sem contar as transicoes)
inline constexpr float BROWNIE_FATOR_ESCUDO = 0.3f;                 // velocidade com escudo e nas transicoes (30%)

inline constexpr int   FAISCA_FRAMES = 3;      // assets/fx_faisca_escudo.png: 96x32 = 3 frames de 32x32
inline constexpr float FAISCA_TAMANHO = 32.0f;  // lado do frame, em pixels (multiplicado por escalaJogo)
inline constexpr unsigned long long FAISCA_MS_FRAME = 60;

// Escudo valendo? Inclui as duas transicoes (P9: invulneravel nelas tambem)
inline bool BossComEscudo(EstadoBoss estado)
{
    return estado == BOSS_ESCUDO_ENTRANDO || estado == BOSS_ESCUDO || estado == BOSS_ESCUDO_SAINDO;
}

// O boss ignora dano? (so o Brownie chega nesses estados)
inline bool BossInvulneravel(TipoBoss tipo, EstadoBoss estado)
{
    return tipo == BOSS_BROWNIE && BossComEscudo(estado);
}

// Animacao em loop? As transicoes tocam uma vez e seguram o ultimo frame
inline bool AnimacaoBossEmLoop(EstadoBoss estado)
{
    return estado != BOSS_ESCUDO_ENTRANDO && estado != BOSS_ESCUDO_SAINDO;
}

// Frame a desenhar: a saida do escudo toca a transicao de tras para frente
inline int FrameExibidoBoss(EstadoBoss estado, int frame, int frames)
{
    if (frame < 0 || frame >= frames) frame = 0;
    return (estado == BOSS_ESCUDO_SAINDO) ? (frames - 1 - frame) : frame;
}

inline unsigned long long BrownieMsTransicao()
{
    return (unsigned long long)TABELA_ANIM_BOSS[ANIM_BROWNIE_TRANSICAO].frames
        * (unsigned long long)TABELA_ANIM_BOSS[ANIM_BROWNIE_TRANSICAO].msFrame;
}

// VULNERAVEL -> ENTRANDO -> ESCUDO -> SAINDO -> VULNERAVEL ... por tempo
inline EstadoBoss TransicaoBrownie(EstadoBoss estado, unsigned long long agora, unsigned long long estadoInicio)
{
    unsigned long long dec = agora - estadoInicio;

    switch (estado)
    {
    case BOSS_ANDANDO:         return (dec >= BROWNIE_MS_VULNERAVEL) ? BOSS_ESCUDO_ENTRANDO : BOSS_ANDANDO;
    case BOSS_ESCUDO_ENTRANDO: return (dec >= BrownieMsTransicao()) ? BOSS_ESCUDO : BOSS_ESCUDO_ENTRANDO;
    case BOSS_ESCUDO:          return (dec >= BROWNIE_MS_ESCUDO) ? BOSS_ESCUDO_SAINDO : BOSS_ESCUDO;
    case BOSS_ESCUDO_SAINDO:   return (dec >= BrownieMsTransicao()) ? BOSS_ANDANDO : BOSS_ESCUDO_SAINDO;
    default:                   return estado;
    }
}

// Fator de velocidade por tipo e estado (antes do Glace)
inline float FatorVelocidadeBoss(TipoBoss tipo, EstadoBoss estado)
{
    if (tipo == BOSS_TOURO)
    {
        if (estado == BOSS_AVISANDO)   return 0.0f;
        if (estado == BOSS_INVESTINDO) return TOURO_FATOR_INVESTIDA;
    }

    if (tipo == BOSS_CHURROS && estado == BOSS_INVOCANDO)
        return 0.0f;    // para enquanto invoca

    if (tipo == BOSS_BROWNIE && BossComEscudo(estado))
        return BROWNIE_FATOR_ESCUDO;

    return 1.0f;
}


// ---- Churros Mago: invocacao (Lote 13h) ----
inline constexpr unsigned long long CHURROS_MS_PRIMEIRA = 2000;   // 1a invocacao, depois do nascimento
inline constexpr unsigned long long CHURROS_MS_INTERVALO = 6000;   // espera entre o fim de uma invocacao e a proxima
inline constexpr int CHURROS_QTD_INVOCACAO = 2;    // minions por invocacao
inline constexpr int CHURROS_FRAME_SPAWN = 3;    // indice (0 a 5) do frame do pico do brilho = "frame 4" da arte; os minions nascem nele

// Tipos que o Churros pode invocar: so os que andam (P10: sem Pirulito; Filhote so nasce do Divisor)
inline constexpr TipoInimigo TIPOS_INVOCAVEIS[] =
{
    INIMIGO_COMUM, INIMIGO_RAPADURA, INIMIGO_GRANULADO, INIMIGO_CARAMELO, INIMIGO_DIVISOR
};
inline constexpr int TOTAL_TIPOS_INVOCAVEIS = (int)(sizeof(TIPOS_INVOCAVEIS) / sizeof(TIPOS_INVOCAVEIS[0]));

inline TipoInimigo SortearTipoInvocado(int (*sorteio)(int) = SorteioPadrao)
{
    return TIPOS_INVOCAVEIS[sorteio(TOTAL_TIPOS_INVOCAVEIS)];
}

// Duracao da animacao de invocar
inline unsigned long long ChurrosMsInvocacao()
{
    return (unsigned long long)TABELA_ANIM_BOSS[ANIM_CHURROS_INVOCAR].frames
        * (unsigned long long)TABELA_ANIM_BOSS[ANIM_CHURROS_INVOCAR].msFrame;
}

// ANDANDO -> INVOCANDO (quando chega a hora) -> ANDANDO (quando a animacao termina)
inline EstadoBoss TransicaoChurros(EstadoBoss estado, unsigned long long agora,
    unsigned long long estadoInicio, unsigned long long proximaInvocacao)
{
    switch (estado)
    {
    case BOSS_ANDANDO:
        if (agora >= proximaInvocacao) return BOSS_INVOCANDO;
        return BOSS_ANDANDO;

    case BOSS_INVOCANDO:
        if (agora - estadoInicio >= ChurrosMsInvocacao()) return BOSS_ANDANDO;
        return BOSS_INVOCANDO;

    default:
        return estado;
    }
}

// Ja chegou o frame do spawn? (por tempo, para nao depender do ritmo dos quadros)
inline bool InvocacaoPronta(unsigned long long agora, unsigned long long estadoInicio)
{
    return agora - estadoInicio >= (unsigned long long)CHURROS_FRAME_SPAWN
        * (unsigned long long)TABELA_ANIM_BOSS[ANIM_CHURROS_INVOCAR].msFrame;
}

struct PosicaoInvocado { float x; float y; };

// Onde nasce o minion 'indice' (0 = acima do boss, 1 = abaixo): a frente do boss (lado da base),
// sempre dentro da tela e nunca atras da borda direita da base.
inline PosicaoInvocado PosInvocado(int indice, float bossX, float bossY, float bossW, float bossH,
    float w, float h, float baseDireita,
    float larguraTela, float alturaTela, float escala)
{
    float x = bossX - w - 10.0f * escala;

    float minX = baseDireita + 10.0f * escala;
    float maxX = larguraTela - w;
    if (maxX < minX) maxX = minX;

    if (x < minX) x = minX;
    if (x > maxX) x = maxX;

    float sinal = (indice % 2 == 0) ? -1.0f : 1.0f;
    float y = bossY + bossH * 0.5f - h * 0.5f + sinal * bossH * 0.6f;

    float maxY = alturaTela - h;
    if (maxY < 0.0f) maxY = 0.0f;

    if (y < 0.0f) y = 0.0f;
    if (y > maxY) y = maxY;

    return { x, y };
}