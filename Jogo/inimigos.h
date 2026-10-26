#pragma once

// Tabela de tipos de minion (so dados, como o curva.h).
// Para ajustar o balanceamento dos minions, mexa so aqui.
//
// REGRA: o valor 0 PRECISA ser o minion comum, porque ReiniciarPartida
// faz memset(inimigos, 0, ...) e todo minion zerado vira comum.

#include <stdlib.h>
#include <math.h>

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
    // Jogabilidade
    int   vida;             // golpes (dano) para matar
    bool  golpesFixos;      // true = cada DISPARO tira 1 de vida, seja qual for o dano (Rapadura: sempre 3 disparos)
    float fatorVelocidade;  // multiplica VelocidadeInimigo(onda)
    float fatorTamanho;     // multiplica tamanhoInimigo (tambem e a hitbox)
    int   pontos;           // pontos por abate
    int   ondaMinima;       // primeira onda em que o tipo entra no sorteio (ONDA_NAO_LIBERADA = nunca sorteado)

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

// ---- Sorteio de tipos (Lote 13c) ----
inline constexpr int ONDA_NAO_LIBERADA = 999;   // tipo ainda sem comportamento pronto: nunca sorteado

// Bando do Granulado: quantos nascem juntos (cada um conta no teto de vivos)
inline constexpr int BANDO_GRANULADO_MIN = 3;
inline constexpr int BANDO_GRANULADO_MAX = 4;
inline constexpr int MAX_GRANULADOS_POR_SPAWN = 5;   // teto de granulados num mesmo spawn (somando todos os bandos)

// Ordem das linhas = ordem do enum TipoInimigo
// ondaMinima: ONDA_NAO_LIBERADA ate o lote que implementa o comportamento do tipo
//   (Pirulito: Lote 13e -> trocar para 2).
//   O Filhote NUNCA entra no sorteio: so nasce quando o Divisor morre.
inline constexpr DadosInimigo TABELA_INIMIGOS[TOTAL_TIPOS_INIMIGO] =
{
    // vida fixo veloc tam    pts  onda  frames msFrame msMorte  larg alt estados  cor provisoria
    { 1, false, 1.0f, 1.0f,   50,  1,    3,     150,    120,     64,  64, 1,       255, 255, 255 },   // COMUM
    { 3, true,  0.7f, 1.0f,   60,  2,    3,     150,    120,     64,  64, 3,       190, 120,  40 },   // RAPADURA (192x192: 3 frames x 3 estados de vida)
    { 1, false, 1.6f, 0.625f, 50,  2,    4,     100,    120,     40,  40, 1,       200, 200, 255 },   // GRANULADO (40/64: tamanho nativo da arte; cor aleatoria por minion)
    { 1, false, 1.0f, 1.0f,   60,  2,    4,     150,    120,     64,  64, 1,       230, 160,  40 },   // CARAMELO (zigue-zague senoidal)
    { 2, false, 1.0f, 1.0f,   50,  2,    3,     150,    300,     64,  64, 2,       255, 150, 190 },   // DIVISOR (msMorte 300 = animacao de divisao de 3 frames)
    { 1, false, 1.0f, 0.5f,   25,  ONDA_NAO_LIBERADA, 3, 150, 120, 32,  32, 1,       255, 150, 190 },   // FILHOTE (nunca sorteado)
    { 1, false, 1.0f, 1.0f,   50,  ONDA_NAO_LIBERADA, 4, 150, 120, 64,  64, 1,       190, 150, 255 },   // PIRULITO (tiro: 13e)
};

// Pirulito: folha de 6 quadros (384x64). Quadros 0 a 3 = voo (framesAndar); 4 e 5 = disparo (Lote 13e).
inline constexpr int PIRULITO_FRAME_DISPARO_INICIO = 4;
inline constexpr int PIRULITO_FRAMES_DISPARO = 2;

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

// ---------------------------------------------------------------------------
// Sorteio (Lote 13c). Todas as funcoes aceitam um "sorteio(n)" que devolve 0..n-1,
// para poder ser testado com numeros previsiveis. O padrao usa rand().
// ---------------------------------------------------------------------------
inline int SorteioPadrao(int n)
{
    return rand() % n;
}

// Tipos que podem nascer na onda: TODOS os ja liberados (ondaMinima <= onda).
// Na onda 1 so o comum esta liberado. Chamar quando a onda muda. Devolve quantos gravou em 'saida'
// (o vetor precisa ter TOTAL_TIPOS_INIMIGO posicoes). Cada spawn sorteia dentro dessa lista,
// entao qualquer tipo liberado pode aparecer a qualquer momento, misturado com os outros.
inline int ListarTiposLiberados(int onda, TipoInimigo* saida)
{
    int total = 0;

    for (int t = 0; t < TOTAL_TIPOS_INIMIGO; t++)
    {
        if (TABELA_INIMIGOS[t].ondaMinima <= onda)
            saida[total++] = (TipoInimigo)t;
    }

    // Garantia: a lista nunca fica vazia (o comum sempre pode nascer)
    if (total == 0)
        saida[total++] = INIMIGO_COMUM;

    return total;
}

// Tipo do proximo minion a nascer, sorteado entre os tipos liberados (eles se misturam).
inline TipoInimigo SortearTipoInimigo(const TipoInimigo* ativos, int total, int (*sorteio)(int) = SorteioPadrao)
{
    if (total <= 0)
        return INIMIGO_COMUM;

    return ativos[sorteio(total)];
}

// Quantos granulados tem o bando (3 ou 4)
inline int TamanhoBandoGranulado(int (*sorteio)(int) = SorteioPadrao)
{
    return BANDO_GRANULADO_MIN + sorteio(BANDO_GRANULADO_MAX - BANDO_GRANULADO_MIN + 1);
}

// Quantos granulados o bando realmente cria: respeita as vagas do teto de vivos
// e o limite de granulados por spawn (0 = nao cabe nenhum)
inline int MembrosDoBando(int sorteado, int vagasNoTeto, int granuladosJaNoSpawn)
{
    int membros = sorteado;
    int restantes = MAX_GRANULADOS_POR_SPAWN - granuladosJaNoSpawn;

    if (membros > vagasNoTeto)
        membros = vagasNoTeto;

    if (membros > restantes)
        membros = restantes;

    return (membros > 0) ? membros : 0;
}

// Dano que um disparo causa em um minion. Tipos com golpesFixos tiram sempre 1 por DISPARO
// (as duas balas do Tiro Duplo contam como um disparo so: a segunda nao tira nada).
inline int DanoNoMinion(const DadosInimigo& dados, int danoDaBala, int idDisparoDaBala, int ultimoIdDisparoDoMinion)
{
    if (!dados.golpesFixos)
        return danoDaBala;

    return (idDisparoDaBala == ultimoIdDisparoDoMinion) ? 0 : 1;
}

// Quantos minions ainda cabem no teto de vivos, contando os que ja nasceram neste spawn
inline int VagasNoTeto(int teto, int vivos, int jaCriados)
{
    int vagas = teto - vivos - jaCriados;
    return (vagas > 0) ? vagas : 0;
}

// Cor do Granulado: a arte e branca, o colormod da a cor
struct CorRGB
{
    unsigned char r, g, b;
};

inline constexpr CorRGB CORES_GRANULADO[] =
{
    { 255,  70,  70 },   // vermelho
    { 255, 150,  50 },   // laranja
    { 255, 230,  60 },   // amarelo
    { 90, 220,  90 },   // verde
    { 80, 220, 230 },   // ciano
    { 90, 130, 255 },   // azul
    { 180, 100, 255 },   // roxo
    { 255, 110, 200 },   // rosa
};

inline constexpr int TOTAL_CORES_GRANULADO = (int)(sizeof(CORES_GRANULADO) / sizeof(CORES_GRANULADO[0]));

inline CorRGB CorAleatoriaGranulado(int (*sorteio)(int) = SorteioPadrao)
{
    return CORES_GRANULADO[sorteio(TOTAL_CORES_GRANULADO)];
}

// ---------------------------------------------------------------------------
// Caramelo (Lote 13d): zigue-zague senoidal. A fase avanca com a distancia andada
// (nada de timestamp: pausa e upgrade nao precisam de DeslocarTimers).
// ---------------------------------------------------------------------------
inline constexpr float CARAMELO_AMPLITUDE = 60.0f;    // px (multiplicada por escalaJogo)
inline constexpr float CARAMELO_COMPRIMENTO_ONDA = 320.0f;   // px andados na horizontal por ciclo completo (x escalaJogo)
inline constexpr float CARAMELO_INCLINACAO_MAX = 15.0f;    // graus de inclinacao do sprite no ponto mais inclinado
inline constexpr float PI_INIMIGOS = 3.14159265f;

// Quanto a fase avanca em um quadro, dada a velocidade (ja multiplicada por escalaJogo)
inline float AvancoFaseCaramelo(float velocidade, float escalaJogo)
{
    return 2.0f * PI_INIMIGOS * velocidade / (CARAMELO_COMPRIMENTO_ONDA * escalaJogo);
}

// Linha central da senoide, escolhida de modo que a onda inteira caiba na tela
inline float YBaseCaramelo(float yDesejado, float amplitude, float alturaTela, float alturaMinion)
{
    float minimo = amplitude;
    float maximo = alturaTela - alturaMinion - amplitude;

    if (maximo < minimo)
        return (alturaTela - alturaMinion) / 2.0f;   // tela muito baixa: fica no meio

    if (yDesejado < minimo) return minimo;
    if (yDesejado > maximo) return maximo;
    return yDesejado;
}

// Posicao y do Caramelo; nunca sai da tela, qualquer que seja a amplitude
inline float CarameloY(float yBase, float amplitude, float fase, float alturaTela, float alturaMinion)
{
    float y = yBase + amplitude * sinf(fase);

    if (y < 0.0f) y = 0.0f;
    if (y > alturaTela - alturaMinion) y = alturaTela - alturaMinion;
    return y;
}

// Inclinacao do sprite em graus, proporcional a velocidade vertical (positivo = bico para cima, sentido horario).
// Subindo = positivo. A velocidade vertical e proporcional a cos(fase).
inline float CarameloAngulo(float fase)
{
    return -cosf(fase) * CARAMELO_INCLINACAO_MAX;
}

// ---------------------------------------------------------------------------
// Divisor (Lote 13d)
// ---------------------------------------------------------------------------
inline constexpr int DIVISOR_FRAMES_DIVISAO = 3;   // minion_marshmallow_divisao.png: 3 quadros de 64x64
inline constexpr int FILHOTES_POR_DIVISAO = 2;

// Quadro da animacao de divisao (tocada uma vez durante a janela de morte). Nunca passa do ultimo.
inline int FrameDivisao(unsigned long long decorrido, int msMorte)
{
    if (msMorte <= 0)
        return DIVISOR_FRAMES_DIVISAO - 1;

    int frame = (int)((decorrido * DIVISOR_FRAMES_DIVISAO) / (unsigned long long)msMorte);

    if (frame > DIVISOR_FRAMES_DIVISAO - 1)
        frame = DIVISOR_FRAMES_DIVISAO - 1;

    return frame;
}

// Posicao de um filhote (k = 0 acima, k = 1 abaixo do centro do pai), sempre dentro da tela
inline void PosicaoFilhote(float paiX, float paiY, float paiW, float paiH, float tamFilhote,
    int k, float alturaTela, float* saidaX, float* saidaY)
{
    float centroX = paiX + paiW / 2.0f;
    float centroY = paiY + paiH / 2.0f;
    float desvio = tamFilhote * 0.75f;

    float x = centroX - tamFilhote / 2.0f;
    float y = centroY + ((k == 0) ? -desvio : desvio) - tamFilhote / 2.0f;

    if (y < 0.0f) y = 0.0f;
    if (y > alturaTela - tamFilhote) y = alturaTela - tamFilhote;

    *saidaX = x;
    *saidaY = y;
}