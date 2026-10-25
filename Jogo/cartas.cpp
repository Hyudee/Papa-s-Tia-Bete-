#include <stdlib.h>
#include <algorithm>
#include "cartas.h"

static const int TIROS_BASE = 3;
static const int MAX_VIDA_EXTRA = 5;     // MAIS VIDA: vida máxima de 3 até 8
static const int MAX_DANO_EXTRA = 7;     // MAIS DANO: dano de 1 até 8
static const int MAX_BALAS_EXTRAS = 5;   // MAIS BALAS: de 3 até 8 tiros simultâneos

static const CartaDef TABELA_CARTAS[NUM_CARTAS] =
{
    { CARTA_MAIS_DANO,    "MAIS DANO",       "+1 de dano por tiro",       RARIDADE_COMUM, MAX_DANO_EXTRA,             100, 0 },
    { CARTA_MAIS_VIDA,    "MAIS VIDA",       "+1 vida max. e +1 vida",   RARIDADE_COMUM, MAX_VIDA_EXTRA,             100, 0 },
    { CARTA_MAIS_BALAS,   "MAIS BALAS",      "+1 projetil simultaneo",    RARIDADE_COMUM, MAX_BALAS_EXTRAS,           100, 0 },
    { CARTA_TIRO_DUPLO,   "TIRO DUPLO",      "+2 balas em leque, 60% do dano", RARIDADE_EPICA,  1,                          5,  0 },
    { CARTA_CACADOR_BOSS, "CACADOR DE BOSS", "+5 de dano nos bosses",     RARIDADE_RARA,  1,                          20,  0 },
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
        + 5 * cartas.Quantidade(CARTA_CACADOR_BOSS);

    s.vidaMaxima = MAX_VIDAS + cartas.Quantidade(CARTA_MAIS_VIDA);

    s.tirosExtras = std::min(cartas.Quantidade(CARTA_MAIS_BALAS), MAX_BALAS_EXTRAS);
    s.quantidadeTiros = TIROS_BASE + s.tirosExtras;

    s.tiroDuplo = cartas.Quantidade(CARTA_TIRO_DUPLO) > 0;
    s.cacadorBoss = cartas.Quantidade(CARTA_CACADOR_BOSS) > 0;

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

    c.comum = std::max(40, 80 - 4 * bossesDerrotados);
    c.rara = std::min(40, 15 + 2 * bossesDerrotados);
    c.epica = std::min(20, 5 + 2 * bossesDerrotados);

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