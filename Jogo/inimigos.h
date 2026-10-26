#pragma once

// Tabela de tipos de minion (so dados, como o curva.h).
// Para ajustar o balanceamento dos minions, mexa so aqui.
//
// REGRA: o valor 0 PRECISA ser o minion comum, porque ReiniciarPartida
// faz memset(inimigos, 0, ...) e todo minion zerado vira comum.

enum TipoInimigo
{
    INIMIGO_COMUM = 0,
    TOTAL_TIPOS_INIMIGO     // sempre o ultimo; os tipos novos entram antes dele
};

struct DadosInimigo
{
    int   vida;             // golpes (dano) para matar
    float fatorVelocidade;  // multiplica VelocidadeInimigo(onda)
    float fatorTamanho;     // multiplica tamanhoInimigo
    int   pontos;           // pontos por abate
    int   framesAndar;      // frames da animacao de andar
    int   msPorFrame;       // tempo de cada frame, em ms
    int   msMorte;          // duracao da janela de morte, em ms
};

// Ordem das linhas = ordem do enum TipoInimigo
inline constexpr DadosInimigo TABELA_INIMIGOS[TOTAL_TIPOS_INIMIGO] =
{
    // vida  veloc  tamanho  pontos  frames  msFrame  msMorte
    { 1,   1.0f,  1.0f,    50,     3,      150,     120 },   // INIMIGO_COMUM
};

inline const DadosInimigo& DadosDoInimigo(TipoInimigo tipo)
{
    return TABELA_INIMIGOS[tipo];
}

// Tipo do proximo minion a nascer. Por enquanto so existe o comum;
// a partir do Lote 13c ela passa a sortear entre os tipos ativos da onda.
inline TipoInimigo SortearTipoInimigo(int onda)
{
    (void)onda;
    return INIMIGO_COMUM;
}