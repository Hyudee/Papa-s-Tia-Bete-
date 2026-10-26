#pragma once

// Tabela de tipos de minion (so dados, como o curva.h).
// Para ajustar o balanceamento dos minions, mexa so aqui.
//
// REGRA: o valor 0 PRECISA ser o minion comum, porque ReiniciarPartida
// faz memset(inimigos, 0, ...) e todo minion zerado vira comum.

enum TipoInimigo
{
    INIMIGO_COMUM = 0,
    INIMIGO_RAPADURA,       // Cubo Blindado
    INIMIGO_GRANULADO,
    INIMIGO_CARAMELO,       // zigue-zague
    INIMIGO_DIVISOR,        // marshmallow
    INIMIGO_FILHOTE,        // filhote do marshmallow
    INIMIGO_PIRULITO,       // atirador voador
    TOTAL_TIPOS_INIMIGO     // sempre o ultimo; os tipos novos entram antes dele
};

struct DadosInimigo
{
    // Jogabilidade (os tipos novos ficam com os valores do comum ate o Lote 13c)
    int   vida;             // golpes (dano) para matar
    float fatorVelocidade;  // multiplica VelocidadeInimigo(onda)
    float fatorTamanho;     // multiplica tamanhoInimigo (tambem e a hitbox)
    int   pontos;           // pontos por abate

    // Animacao
    int   framesAndar;      // frames da animacao de andar (colunas da folha)
    int   msPorFrame;       // tempo de cada frame, em ms
    int   msMorte;          // duracao da janela de morte, em ms

    // Arte: frames em colunas, estados de vida em linhas
    int   frameLargura;     // largura de um frame na folha, em pixels
    int   frameAltura;      // altura de um frame na folha, em pixels
    int   estadosVida;      // linhas da folha: 0 = inteiro, 1 = machucado, ...

    // Cor provisoria, usada so se o arquivo de arte do tipo nao existir
    unsigned char corR, corG, corB;
};

// Duracao do flash branco quando o tiro acerta mas nao mata, em ms
inline constexpr int MS_FLASH_ACERTO = 90;

// Ordem das linhas = ordem do enum TipoInimigo
inline constexpr DadosInimigo TABELA_INIMIGOS[TOTAL_TIPOS_INIMIGO] =
{
    // vida veloc tam    pts  frames msFrame msMorte  larg alt estados  cor provisoria
    { 1,  1.0f, 1.0f,   50,  3,     150,    120,     64,  64, 1,       255, 255, 255 },   // COMUM
    { 1,  1.0f, 1.0f,   50,  3,     150,    120,     64,  64, 3,       190, 120,  40 },   // RAPADURA
    { 1,  1.0f, 0.625f, 50,  4,     100,    120,     40,  40, 1,       200, 200, 255 },   // GRANULADO (40/64: tamanho nativo da arte)
    { 1,  1.0f, 1.0f,   50,  4,     150,    120,     64,  64, 1,       230, 160,  40 },   // CARAMELO
    { 1,  1.0f, 1.0f,   50,  3,     150,    120,     64,  64, 2,       255, 150, 190 },   // DIVISOR
    { 1,  1.0f, 0.5f,   50,  3,     150,    120,     32,  32, 1,       255, 150, 190 },   // FILHOTE
    { 1,  1.0f, 1.0f,   50,  4,     150,    120,     64,  64, 1,       190, 150, 255 },   // PIRULITO (4 de voo; os 2 ultimos quadros sao do disparo, Lote 13e)
};

inline const DadosInimigo& DadosDoInimigo(TipoInimigo tipo)
{
    return TABELA_INIMIGOS[tipo];
}

// Linha da folha de arte que representa a vida atual (0 = inteiro).
// Nunca passa da ultima linha, mesmo com vida <= 0 (janela de morte).
inline constexpr int EstadoDeVida(const DadosInimigo& dados, int vida, int vidaMax)
{
    int estado = vidaMax - vida;

    if (estado < 0)
        estado = 0;

    if (estado > dados.estadosVida - 1)
        estado = dados.estadosVida - 1;

    return estado;
}

// Tipo do proximo minion a nascer. Por enquanto so existe o comum em jogo;
// a partir do Lote 13c ela passa a sortear entre os tipos ativos da onda.
inline TipoInimigo SortearTipoInimigo(int onda)
{
    (void)onda;
    return INIMIGO_COMUM;
}