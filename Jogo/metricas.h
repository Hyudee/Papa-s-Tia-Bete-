#pragma once

#include <SDL3/SDL.h>

// Dados de uma partida que vão para o CSV (preenchidos pelo Jogo ao fim da partida)
struct ResumoPartida
{
    int onda;
    int pontos;
    int bossesDerrotados;
    const char* causa;      // ex.: "inimigo_na_base", "boss_na_base"
    int danoBase;
    int tirosExtras;
    int vidaMaxima;
    bool tiroDuplo;
    bool cacadorBoss;
    int cadencia;           // quantas cartas de cada tipo (carta nova = coluna nova)
    int balaPesada;
    int pernasAcucar;
    int doceSorte;
    int glace;
    int perfurante;
    int barreira;
    int explosão;
    int furia;
    int larguraTela;
    int alturaTela;
};

// Medição de desempenho e registro de partidas em "metricas.csv" (na pasta de execução)
class Metricas
{
public:
    // Uma vez por volta do laço principal. emPartida = true só enquanto se está jogando
    // (pause, upgrade e menus não contam no tempo da partida).
    void AtualizarFrame(bool emPartida);

    void IniciarPartida();

    // Grava uma linha no CSV. Só grava uma vez por partida.
    void FinalizarPartida(const ResumoPartida& resumo);

private:
    // FPS (janela de 5 s)
    Uint64 ultimoFrameNs = 0;
    Uint64 janelaInicioNs = 0;
    int janelaFrames = 0;
    float piorFrameMs = 0.0f;

    // Partida atual
    float tempoPartidaMs = 0.0f;
    Uint64 framesPartida = 0;
    bool partidaAberta = false;
};