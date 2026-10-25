#include <stdlib.h>
#include <algorithm>
#include "cartas.h"

static const int TIROS_BASE = 3;

static const CartaDef TABELA_CARTAS[NUM_CARTAS] =
{
    { CARTA_MAIS_DANO,    "MAIS DANO",       "+1 de dano por tiro",       RARIDADE_COMUM, 0,                          100 },
    { CARTA_MAIS_VIDA,    "MAIS VIDA",       "+1 de vida maxima",         RARIDADE_COMUM, 0,                          100 },
    { CARTA_MAIS_BALAS,   "MAIS BALAS",      "+1 projetil simultaneo",    RARIDADE_COMUM, MAX_PROJETEIS - TIROS_BASE, 100 },
    { CARTA_TIRO_DUPLO,   "TIRO DUPLO",      "Atira 2 projeteis por vez", RARIDADE_RARA,  1,                          10 },
    { CARTA_CACADOR_BOSS, "CACADOR DE BOSS", "+5 de dano nos bosses",     RARIDADE_RARA,  1,                          5 },
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

    s.tirosExtras = std::min(cartas.Quantidade(CARTA_MAIS_BALAS), MAX_PROJETEIS - TIROS_BASE);
    s.quantidadeTiros = TIROS_BASE + s.tirosExtras;

    s.tiroDuplo = cartas.Quantidade(CARTA_TIRO_DUPLO) > 0;
    s.cacadorBoss = cartas.Quantidade(CARTA_CACADOR_BOSS) > 0;

    return s;
}

int MontarOfertaCartas(const ColecaoCartas& cartas, IdCarta* oferta, int capacidade)
{
    int total = 0;

    for (int i = 0; i < NUM_CARTAS && total < capacidade; i++)
    {
        const CartaDef& carta = TABELA_CARTAS[i];

        // Cartas únicas que o jogador já tem não aparecem de novo
        if (carta.maximo == 1 && !cartas.PodeReceber(carta.id))
            continue;

        // Só sorteia quando a chance é menor que 100% (mesma ordem de rand() de antes)
        if (carta.chanceOferta >= 100 || rand() % 100 < carta.chanceOferta)
            oferta[total++] = carta.id;
    }

    return total;
}