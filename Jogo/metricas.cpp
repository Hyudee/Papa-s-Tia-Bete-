#include <stdio.h>
#include "metricas.h"

void Metricas::AtualizarFrame(bool emPartida)
{
    Uint64 agora = SDL_GetTicksNS();

    if (ultimoFrameNs == 0)
    {
        ultimoFrameNs = agora;
        janelaInicioNs = agora;
        return;
    }

    float frameMs = (agora - ultimoFrameNs) / 1000000.0f;
    ultimoFrameNs = agora;

    // FPS médio a cada 5 s (aparece na janela Saída do Visual Studio)
    janelaFrames++;

    if (frameMs > piorFrameMs)
        piorFrameMs = frameMs;

    if (agora - janelaInicioNs >= 5000000000ULL)
    {
        float segundos = (agora - janelaInicioNs) / 1000000000.0f;

        SDL_Log("FPS: %.1f | frame medio: %.2f ms | pior frame: %.1f ms",
            janelaFrames / segundos,
            segundos * 1000.0f / janelaFrames,
            piorFrameMs);

        janelaInicioNs = agora;
        janelaFrames = 0;
        piorFrameMs = 0.0f;
    }

    // Tempo e frames da partida (travamentos grandes, como arrastar a janela, não entram)
    if (emPartida && partidaAberta && frameMs < 250.0f)
    {
        tempoPartidaMs += frameMs;
        framesPartida++;
    }
}

void Metricas::IniciarPartida()
{
    tempoPartidaMs = 0.0f;
    framesPartida = 0;
    partidaAberta = true;
}

void Metricas::FinalizarPartida(const ResumoPartida& resumo)
{
    if (!partidaAberta)
        return;

    partidaAberta = false;

    // Cabeçalho só quando o arquivo ainda não existe
    bool escreverCabecalho = true;
    FILE* teste = nullptr;

    fopen_s(&teste, "metricas.csv", "rb");

    if (teste)
    {
        escreverCabecalho = false;
        fclose(teste);
    }

    FILE* arquivo = nullptr;

    fopen_s(&arquivo, "metricas.csv", "ab");

    if (!arquivo)
    {
        SDL_Log("Nao foi possivel abrir metricas.csv para escrita");
        return;
    }

    if (escreverCabecalho)
    {
        fprintf(arquivo,
            "data_hora,tempo_s,onda,pontos,bosses_derrotados,causa,"
            "dano_base,tiros_extras,vida_maxima,tiro_duplo,cacador_boss,"
            "cadencia,bala_pesada,pernas_acucar,doce_sorte,glace,perfurante,"
            "barreira,explosao,furia,pacto,"
            "fps_medio,largura,altura\n");
    }

    // Data e hora locais
    char dataHora[32] = "";
    SDL_Time agora = 0;
    SDL_DateTime dt;

    if (SDL_GetCurrentTime(&agora) && SDL_TimeToDateTime(agora, &dt, true))
    {
        snprintf(dataHora, sizeof(dataHora), "%04d-%02d-%02d %02d:%02d:%02d",
            dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    }

    float tempoS = tempoPartidaMs / 1000.0f;
    float fpsMedio = (tempoS > 0.0f) ? framesPartida / tempoS : 0.0f;

    fprintf(arquivo, "%s,%.1f,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.1f,%d,%d\n",
        dataHora,
        tempoS,
        resumo.onda,
        resumo.pontos,
        resumo.bossesDerrotados,
        resumo.causa,
        resumo.danoBase,
        resumo.tirosExtras,
        resumo.vidaMaxima,
        resumo.tiroDuplo ? 1 : 0,
        resumo.cacadorBoss ? 1 : 0,
        resumo.cadencia,
        resumo.balaPesada,
        resumo.pernasAcucar,
        resumo.doceSorte,
        resumo.glace,
        resumo.perfurante,
        resumo.barreira,
        resumo.explosao,
        resumo.furia,
        resumo.pacto,
        fpsMedio,
        resumo.larguraTela,
        resumo.alturaTela);

    fclose(arquivo);
}