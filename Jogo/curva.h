#pragma once

#include <algorithm>
#include "config.h"

// Curva de dificuldade: tudo em função da onda (a partida começa na onda 1
// e a cada 30 s sobe uma). Para ajustar o balanceamento, mexa só aqui.

// Inimigos que nascem de uma vez: +1 a cada 5 ondas
inline int QuantidadePorSpawn(int onda)
{
    return 1 + onda / 5;
}

// Máximo de inimigos vivos: 5, 10, 15... (+5 a cada 5 ondas), até MAX_INIMIGOS
inline int MaximoInimigosVivos(int onda)
{
    return std::min(MAX_INIMIGOS, 5 + 5 * (onda / 5));
}

// Intervalo entre spawns, em ms
inline int IntervaloSpawn(int onda)
{
    return std::max(1000, 2000 - 100 * onda);
}

// Velocidade base de um inimigo (antes de multiplicar por escalaJogo)
inline float VelocidadeInimigo(int onda)
{
    return std::min(5.0f, 2.0f + 0.15f * onda);
}

// Boss: vida e aumento de velocidade (somado à velocidade base de 1,5)
inline int BossVidaMax(int bossesDerrotados)
{
    return 55 + 10 * bossesDerrotados;
}

inline float BossVelocidadeExtra(int bossesDerrotados)
{
    return std::min(1.5f, 0.15f * bossesDerrotados);
}

// ---- XP e nivel do jogador ----
inline constexpr int XP_BASE = 150;   // XP para sair do nivel 1
inline constexpr int XP_INCREMENTO = 60;    // quanto a barra cresce a cada nivel
inline constexpr int XP_BOSS_PERCENT_DA_BARRA = 25;    // % da barra do nivel atual que o boss concede

inline int XpParaProximoNivel(int nivel)
{
    return XP_BASE + XP_INCREMENTO * (nivel - 1);
}

inline int XpDoBoss(int nivel)
{
    return (XpParaProximoNivel(nivel) * XP_BOSS_PERCENT_DA_BARRA + 50) / 100;   // arredondado
}