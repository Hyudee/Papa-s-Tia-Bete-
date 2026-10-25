#pragma once

#include "config.h"

// As cartas são dados: para criar uma carta nova, acrescente uma linha em
// cartas.cpp (e o efeito em CalcularStats). Nada de índices escritos à mão.

enum IdCarta
{
    CARTA_MAIS_DANO,
    CARTA_MAIS_VIDA,
    CARTA_MAIS_BALAS,
    CARTA_TIRO_DUPLO,
    CARTA_CACADOR_BOSS,

    NUM_CARTAS
};

enum Raridade
{
    RARIDADE_COMUM,
    RARIDADE_RARA,
    RARIDADE_EPICA
};

struct CartaDef
{
    IdCarta id;
    const char* nome;
    const char* descricao;
    Raridade raridade;
    int maximo;         // quantas vezes pode ser pega (0 = sem limite)
    int chanceOferta;   // % de aparecer na escolha (100 = sempre); regra provisória
};

// Cartas que o jogador já pegou nesta partida
struct ColecaoCartas
{
    int quantidade[NUM_CARTAS] = { 0 };

    void Limpar();
    int Quantidade(IdCarta id) const;
    bool PodeReceber(IdCarta id) const;   // false se já está no máximo
    bool Adicionar(IdCarta id);           // false (e nada muda) se já está no máximo
};

// Tudo que as cartas mudam no jogador, recalculado a partir da coleção
struct StatsJogador
{
    int danoBase;
    int vidaMaxima;
    int tirosExtras;
    int quantidadeTiros;
    bool tiroDuplo;
    bool cacadorBoss;
};

const CartaDef& DadosDaCarta(IdCarta id);

StatsJogador CalcularStats(const ColecaoCartas& cartas);

// Sorteia as cartas oferecidas após um boss (na ordem da tabela) e devolve quantas são.
int MontarOfertaCartas(const ColecaoCartas& cartas, IdCarta* oferta, int capacidade);