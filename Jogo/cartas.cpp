#include <stdlib.h>
#include <algorithm>
#include "cartas.h"

static const int TIROS_BASE = 3;
static const int MAX_VIDA_EXTRA = 5;     // MAIS VIDA: vida máxima de 3 até 8
static const int MAX_DANO_EXTRA = 7;     // MAIS DANO: dano de 1 até 8
static const int MAX_BALAS_EXTRAS = 5;   // MAIS BALAS: de 3 até 8 tiros simultâneos
static const int MAX_CADENCIA = 3;       // CADENCIA: -20% do tempo entre tiros por carta
static const int MAX_BALA_PESADA = 2;    // BALA PESADA: +2 de dano por carta
static const int MAX_PERNAS = 3;         // PERNAS DE ACUCAR: +10% de velocidade por carta
static const int MAX_DOCE_SORTE = 2;     // DOCE SORTE: coração 35% mais lento e 25% mais cedo por carta    
static const int MAX_GLACE = 2;          // GLACE PEGAJOSO: boss 15% mais lento por carta, -1 de dano por carta
static const int MAX_PERFURANTE = 2;     // PERFURANTE: a bala atravessa +1 minion por carta, tiro 15% mais lento
static const int MAX_BARREIRA = 2;       // BARREIRA DA BASE: +1 carga por carta, jogador 10% mais lento por carta
static const int MAX_EXPLOSAO = 2;       // EXPLOSAO ACUCARADA: +15% de chance por carta
static const int MAX_FURIA = 2;          // FURIA DE COMBO: +25% de dano por carta

static const CartaDef TABELA_CARTAS[NUM_CARTAS] =
{
    { CARTA_MAIS_DANO,    "MAIS DANO",       "+1 de dano por tiro",       RARIDADE_COMUM, MAX_DANO_EXTRA,             100, 0 },
    { CARTA_MAIS_VIDA,    "MAIS VIDA",       "+1 vida max. e +1 vida",   RARIDADE_COMUM, MAX_VIDA_EXTRA,             100, 0 },
    { CARTA_MAIS_BALAS,   "MAIS BALAS",      "+1 projetil simultaneo",    RARIDADE_COMUM, MAX_BALAS_EXTRAS,           100, 0 },
    { CARTA_TIRO_DUPLO,   "TIRO DUPLO",      "+2 balas em leque, 60% do dano", RARIDADE_EPICA,  1,                          5,  0 },
    { CARTA_CACADOR_BOSS, "CACADOR DE BOSS", "+5 de dano nos bosses",     RARIDADE_EPICA,  1,                          20,  0 },
    { CARTA_CADENCIA,     "CADENCIA",        "-20% tempo entre tiros",    RARIDADE_COMUM, MAX_CADENCIA,               100,  0 },
    { CARTA_BALA_PESADA,  "BALA PESADA",     "+2 dano, tiro mais lento",  RARIDADE_COMUM, MAX_BALA_PESADA,            100,  0 },
    { CARTA_PERNAS_ACUCAR,"PERNAS DE ACUCAR","+10% velocidade, mais recuo", RARIDADE_COMUM, MAX_PERNAS,                100,  0 },
    { CARTA_DOCE_SORTE,   "DOCE SORTE",      "Coracao lento e frequente", RARIDADE_COMUM, MAX_DOCE_SORTE,             60,  0 },
    { CARTA_GLACE,        "GLACE PEGAJOSO",  "Boss 15% mais lento, -1 dano", RARIDADE_RARA, MAX_GLACE,                 40,  0 },
    { CARTA_PERFURANTE,   "PERFURANTE",      "Atravessa +1 inimigo, tiro lento", RARIDADE_RARA, MAX_PERFURANTE,        40,  0 },
    { CARTA_BARREIRA,     "BARREIRA DA BASE", "Bloqueia 1 minion, recarga 25s", RARIDADE_RARA, MAX_BARREIRA,         35,  0 },
    { CARTA_EXPLOSAO,     "EXPLOSAO ACUCARADA", "15% de explodir ao abater", RARIDADE_RARA, MAX_EXPLOSAO,        30,  0 },
    { CARTA_FURIA,        "FURIA DE COMBO", "10 abates seguidos: +25% dano", RARIDADE_EPICA, MAX_FURIA,           30,  0 },
};

const CartaDef& DadosDaCarta(IdCarta id)
{
    return TABELA_CARTAS[id];
}

void ColecaoCartas::Limpar()
{
    for (int i = 0; i < NUM_CARTAS; i++)
        quantidade[i] = 0;
}

int ColecaoCartas::Quantidade(IdCarta id) const
{
    return quantidade[id];
}

bool ColecaoCartas::PodeReceber(IdCarta id) const
{
    int maximo = TABELA_CARTAS[id].maximo;

    return maximo == 0 || quantidade[id] < maximo;
}

bool ColecaoCartas::Adicionar(IdCarta id)
{
    if (!PodeReceber(id))
        return false;

    quantidade[id]++;
    return true;
}

StatsJogador CalcularStats(const ColecaoCartas& cartas)
{
    StatsJogador s;

    s.danoBase = 1
        + cartas.Quantidade(CARTA_MAIS_DANO)
        + 5 * cartas.Quantidade(CARTA_CACADOR_BOSS)
        + 2 * cartas.Quantidade(CARTA_BALA_PESADA)
        - cartas.Quantidade(CARTA_GLACE);

    s.vidaMaxima = MAX_VIDAS + cartas.Quantidade(CARTA_MAIS_VIDA);

    s.tirosExtras = std::min(cartas.Quantidade(CARTA_MAIS_BALAS), MAX_BALAS_EXTRAS);
    s.quantidadeTiros = TIROS_BASE + s.tirosExtras;

    s.tiroDuplo = cartas.Quantidade(CARTA_TIRO_DUPLO) > 0;
    s.cacadorBoss = cartas.Quantidade(CARTA_CACADOR_BOSS) > 0;


    // Tempo entre tiros: CADENCIA tira 20% da base por carta, BALA PESADA põe 20% (soma de percentuais)
    int percentualCooldown = 100
        - 20 * cartas.Quantidade(CARTA_CADENCIA)
        + 20 * cartas.Quantidade(CARTA_BALA_PESADA);

    // Custa 1 de dano por carta, mas nunca cai pra 0
    s.danoBase = std::max(1, s.danoBase);

    s.cooldownTiro = COOLDOWN_TIRO_BASE * std::max(20, percentualCooldown) / 100;

    // Bala 20% mais lenta por bala pesada e 15% por perfurante
    s.velocidadeTiroPercent = std::max(20, 100
        - 20 * cartas.Quantidade(CARTA_BALA_PESADA)
        - 15 * cartas.Quantidade(CARTA_PERFURANTE));

    // Jogador 10% mais rápido por PERNAS DE ACUCAR, com recuo 30% maior por carta
    // BARREIRA DA BASE custa 10% de velocidade por carta (soma de percentuais)
    s.velocidadeJogadorPercent = std::max(20, 100
        + 10 * cartas.Quantidade(CARTA_PERNAS_ACUCAR)
        - 10 * cartas.Quantidade(CARTA_BARREIRA));

    s.knockbackPercent = 100 + 30 * cartas.Quantidade(CARTA_PERNAS_ACUCAR);


    // Coração 35% mais lento e nascendo 25% mais cedo por DOCE SORTE (soma de percentuais)
    s.velocidadeCoracaoPercent = std::max(20, 100 - 35 * cartas.Quantidade(CARTA_DOCE_SORTE));
    s.intervaloCoracaoPercent = std::max(20, 100 - 25 * cartas.Quantidade(CARTA_DOCE_SORTE));


    // Cada acerto no boss o desacelera 15% por carta de GLACE PEGAJOSO
    s.lentidaoBossPercent = 15 * cartas.Quantidade(CARTA_GLACE);


    // PERFURANTE: a bala atravessa 1 minion por carta (o boss sempre para a bala)
    s.perfuracoes = cartas.Quantidade(CARTA_PERFURANTE);


    // BARREIRA DA BASE: cada carta guarda 1 carga
    s.barreiras = cartas.Quantidade(CARTA_BARREIRA);


    // EXPLOSAO ACUCARADA: 15% de chance por carta de explodir ao abater um minion
    s.explosaoChancePercent = 15 * cartas.Quantidade(CARTA_EXPLOSAO);


    // FURIA DE COMBO: +25% de dano por carta enquanto a Furia estiver ativa
    s.furiaDanoPercent = 25 * cartas.Quantidade(CARTA_FURIA);

    return s;
}

int DanoPorBalaTiroDuplo(int dano)
{
    // 0,6 * dano arredondado, só com inteiros: (6 * dano + 5) / 10
    return std::max(1, (6 * dano + 5) / 10);
}

ChancesRaridade ChancesPorBosses(int bossesDerrotados)
{
    ChancesRaridade c;

    c.comum = 0;
    c.rara = 80;
    c.epica = 20;

    return c;
}

static int Sortear(int limite, int (*aleatorio)(int))
{
    return aleatorio ? aleatorio(limite) : rand() % limite;
}

// Sorteia uma carta da raridade pedida, pelo peso, entre as que podem aparecer agora
static bool SortearCartaDaRaridade(const ColecaoCartas& cartas, int bossesDerrotados, Raridade raridade,
    const IdCarta* oferta, int total,
    int (*aleatorio)(int), IdCarta& escolhida)
{
    bool elegivel[NUM_CARTAS];
    int somaPesos = 0;

    for (int i = 0; i < NUM_CARTAS; i++)
    {
        const CartaDef& carta = TABELA_CARTAS[i];

        elegivel[i] = carta.raridade == raridade
            && carta.peso > 0
            && cartas.PodeReceber(carta.id)               // fora as no máximo e as únicas já pegas
            && bossesDerrotados >= carta.bossesMinimos;   // pré-requisito

        // Fora as que já estão nesta oferta
        for (int k = 0; k < total; k++)
        {
            if (oferta[k] == carta.id)
                elegivel[i] = false;
        }

        if (elegivel[i])
            somaPesos += carta.peso;
    }

    if (somaPesos == 0)
        return false;

    int sorteio = Sortear(somaPesos, aleatorio);

    for (int i = 0; i < NUM_CARTAS; i++)
    {
        if (!elegivel[i])
            continue;

        if (sorteio < TABELA_CARTAS[i].peso)
        {
            escolhida = TABELA_CARTAS[i].id;
            return true;
        }

        sorteio -= TABELA_CARTAS[i].peso;
    }

    return false;   // não chega aqui: sorteio < somaPesos
}

int MontarOfertaCartas(const ColecaoCartas& cartas, int bossesDerrotados,
    IdCarta* oferta, int capacidade, int (*aleatorio)(int))
{
    ChancesRaridade chances = ChancesPorBosses(bossesDerrotados);
    int total = 0;

    while (total < capacidade)
    {
        // Raridade deste espaço
        int sorteio = Sortear(100, aleatorio);
        int sorteada = RARIDADE_COMUM;

        if (sorteio >= chances.comum + chances.rara)
            sorteada = RARIDADE_EPICA;
        else if (sorteio >= chances.comum)
            sorteada = RARIDADE_RARA;

        // Sem carta nessa raridade: cai para a de baixo; por último tenta as de cima
        int ordem[3];
        int n = 0;

        for (int r = sorteada; r >= RARIDADE_COMUM; r--)
            ordem[n++] = r;

        for (int r = sorteada + 1; r <= RARIDADE_EPICA; r++)
            ordem[n++] = r;

        bool achou = false;
        IdCarta escolhida = CARTA_MAIS_DANO;

        for (int k = 0; k < n && !achou; k++)
            achou = SortearCartaDaRaridade(cartas, bossesDerrotados, (Raridade)ordem[k],
                oferta, total, aleatorio, escolhida);

        if (!achou)
            break;   // acabaram as cartas elegíveis

        oferta[total++] = escolhida;
    }

    return total;
}