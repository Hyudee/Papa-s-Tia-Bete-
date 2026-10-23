#pragma once

#include "tipos.h"

// Ranking local

void InicializarRanking(Ranking ranking[]);
void CarregarRanking(Ranking ranking[]);
void SalvarRanking(Ranking ranking[]);
int AdicionarPontuacao(Ranking ranking[], char nome[], int pontos);