#include "ranking.h"

#include <stdio.h>
#include <string.h>

void InicializarRanking(Ranking ranking[])
{
    for (int i = 0; i < 3; i++)
    {
        strcpy_s(ranking[i].nome,
            sizeof(ranking[i].nome),
            "???");
        ranking[i].pontos = 0;
    }
}

void CarregarRanking(Ranking ranking[])
{
    FILE* arq;
    fopen_s(&arq,
        "ranking.dat",
        "rb");

    if (arq == NULL)
    {
        InicializarRanking(ranking);
        return;
    }

    Ranking lido[3];

    size_t quantidade = fread(lido, sizeof(Ranking), 3, arq);
    fclose(arq);

    // Arquivo incompleto ou corrompido: começa um ranking novo
    if (quantidade != 3)
    {
        InicializarRanking(ranking);
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        // Garante o terminador do nome
        lido[i].nome[sizeof(lido[i].nome) - 1] = '\0';

        ranking[i] = lido[i];
    }
}

void SalvarRanking(Ranking ranking[])
{
    FILE* arq;
    fopen_s(&arq,
        "ranking.dat",
        "wb");

    if (arq == NULL)
        return;

    fwrite(ranking, sizeof(Ranking), 3, arq);
    fclose(arq);
}

int AdicionarPontuacao(Ranking ranking[],
    char nome[],
    int pontos)
{
    if (pontos <= ranking[2].pontos)
        return 0;

    strcpy_s(ranking[2].nome,
        sizeof(ranking[2].nome),
        nome);
    ranking[2].pontos = pontos;

    for (int i = 2; i > 0; i--)
    {
        if (ranking[i].pontos >
            ranking[i - 1].pontos)
        {
            Ranking temp = ranking[i];
            ranking[i] = ranking[i - 1];
            ranking[i - 1] = temp;
        }
    }

    return 1;
}