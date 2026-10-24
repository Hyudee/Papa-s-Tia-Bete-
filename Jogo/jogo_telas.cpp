#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include "config.h"
#include "tipos.h"
#include "ranking.h"
#include "audio.h"
#include "escala.h"
#include "sdl_raii.h"
#include "assets.h"
#include "texto.h"
#include "botoes.h"
#include "jogo.h"

bool Jogo::TelaMenu()
{
    if (estado == MENU)
    {

        SDL_RenderClear(renderer);

        SDL_FRect menuRect = {
            0,
            0,
            (float)larguraTela,
            (float)alturaTela
        };

        SDL_RenderTexture(renderer, menu, NULL, &menuRect);
        SDL_RenderTexture(renderer, texturaEngrenagem, NULL, &abaConfig);
        SDL_RenderTexture(renderer, texturaControles, NULL, &abaControles);
        SDL_RenderTexture(renderer, texturaTrofeu, NULL, &abaRanking);
        SDL_RenderTexture(renderer, texturaAjuda, NULL, &abaAjuda);

        //Desenhar logo e botões

        SDL_RenderTexture(renderer, logo, NULL, &logoRect);
        SDL_RenderTexture(renderer, jogar, NULL, &botaoJogar);
        SDL_RenderTexture(renderer, creditos, NULL, &botaoCreditos);
        SDL_RenderTexture(renderer, sair, NULL, &botaoSair);

        // Destaque arredondado do item (teclado ou mouse)
        const SDL_FRect* destaque = RetanguloItemMenu(itemMenu);

        if (destaque)
        {
            if (itemMenu < 3)
                DesenharDestaque(*destaque, 8.0f, 6.0f, corDestaqueOuro);   // botões
            else
                DesenharDestaque(*destaque, 4.0f, 4.0f, corDestaqueOuro);   // abas
        }

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaCreditos()
{
    if (estado == CREDITOS) {
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

        // Rolagem rápida: segurar Espaço ou seta para baixo
        const bool* teclasCreditos = SDL_GetKeyboardState(NULL);
        float velocidadeCreditos = 1.0f;

        if (teclasCreditos[SDL_SCANCODE_SPACE] ||
            teclasCreditos[SDL_SCANCODE_DOWN])
        {
            velocidadeCreditos = 4.0f;
        }

        yCreditos -= velocidadeCreditos;

        // Textos

        float espacamento = alturaTela * 0.07f;

        const char* linhascreditos[] =
        {
            "Desenvolvido por:",
            "",


            "Direção",
            "Davi Hyude Nunes de Almeida",
            "",

            "Programação",
            "Davi Hyude Nunes de Almeida",
            "",

            "Game Design",
            "Davi Hyude Nunes de Almeida,",
            "Danah Bezerra de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Direção de Arte",
            "Danah Bezerra de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Arte 2D",
            "Danah Bezerra de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Animação",
            "Danah Bezerra de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Design de Interface (UI)",
            "Davi Hyude Nunes de Almeida,",
            "Danah Bezerra de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Efeitos Sonoros",
            "Davi Hyude Nunes de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Balanceamento",
            "Davi Hyude Nunes de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Testes",
            "Davi Hyude Nunes de Almeida e",
            "João Pedro Souza de Oliveira",
            "Danah Bezerra de Almeida e",
            "Ian Victor de Miranda e Carvalho",
            "",


            "Suporte Moral",
            "Lucas Natanael de Oliveira Costa",
            "",

            "Ferramentas Utilizadas",
            "C++",
            "SDL3",
            "SDL_image",
            "SDL_ttf",
            "Microsoft Visual Studio 2026",
            "",

            "© 2026",
            "Davi Hyude Nunes de Almeida,",
            "Danah Bezerra de Almeida e",
            "João Pedro Souza de Oliveira",
            "",

            "Obrigado por jogar!"
        };

        int quantidadeLinhas = sizeof(linhascreditos) / sizeof(linhascreditos[0]);

        for (int i = 0; i < quantidadeLinhas; i++)
        {
            if (strlen(linhascreditos[i]) == 0)
                continue;

            DesenharTextoCentrado(renderer, fontHud, linhascreditos[i], preto,
                (float)(larguraTela / 2), yCreditos + i * espacamento);
        }

        DesenharTextoEsquerda(renderer, fontHud, "ESC - Voltar", preto,
            20, (float)(alturaTela - 50));

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaRanking()
{
    if (estado == RANKING)
    {
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

        char texto[50];

        // Titulo
        DesenharTextoCentrado(renderer, font, "RANKING", preto,
            (float)(larguraTela / 2), 50);

        for (int i = 0; i < 3; i++)
        {
            sprintf_s(
                texto,
                sizeof(texto),
                "%d. %s - %d",
                i + 1,
                ranking[i].nome,
                ranking[i].pontos
            );

            DesenharTextoCentrado(renderer, font, texto, preto,
                (float)(larguraTela / 2), (float)(150 + i * 80));
        }

        // Voltar
        DesenharTextoEsquerda(renderer, fontHud, "ESC - Voltar", preto,
            20, (float)(alturaTela - 50));

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaDigitarNome()
{
    if (estado == DIGITAR_NOME)
    {
        SDL_FRect fundo = {
        0,
        0,
        (float)larguraTela,
        (float)alturaTela
        };

        SDL_RenderTexture(renderer, backgroundNome, NULL, &fundo);

        //desenha os textos aqui

        DesenharTextoCentrado(renderer, font, "NOVO RECORDE!", preto,
            (float)(larguraTela / 2), 120);

        DesenharTextoCentrado(renderer, fontHud, "Digite seu nome:", preto,
            (float)(larguraTela / 2), 200);

        DesenharTextoCentrado(renderer, font, nomeJogador, preto,
            (float)(larguraTela / 2), 260);

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

// Limpa a janela inteira de preto (inclusive as tarjas do letterbox) e depois pinta
// a área do jogo com a cor de desenho atual, como o SDL_RenderClear fazia antes.
void Jogo::LimparTela()
{
    Uint8 corR, corG, corB, corA;
    SDL_GetRenderDrawColor(renderer, &corR, &corG, &corB, &corA);

    SDL_BlendMode modoBlend;
    SDL_GetRenderDrawBlendMode(renderer, &modoBlend);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_FRect areaJogo = { 0.0f, 0.0f, (float)larguraTela, (float)alturaTela };

    SDL_SetRenderDrawColor(renderer, corR, corG, corB, corA);
    SDL_RenderFillRect(renderer, &areaJogo);

    SDL_SetRenderDrawBlendMode(renderer, modoBlend);
}

// Retângulo arredondado translúcido atrás de uma opção (textura gerada em código)
void Jogo::DesenharDestaque(const SDL_FRect& alvo, float margemX, float margemY, SDL_Color cor) const
{
    SDL_Texture* textura = assets.pilulaDestaque.get();

    if (!textura)
        return;

    SDL_FRect destino = {
        alvo.x - margemX,
        alvo.y - margemY,
        alvo.w + 2.0f * margemX,
        alvo.h + 2.0f * margemY
    };

    // O raio dos cantos diminui em alvos finos (ex.: abas) para não se sobrepor
    float canto = SDL_min(24.0f, SDL_min(destino.w, destino.h) / 2.0f);

    // Pulso suave de opacidade
    float pulso = 0.5f + 0.5f * SDL_sinf((float)SDL_GetTicks() * 0.006f);

    SDL_SetTextureColorMod(textura, cor.r, cor.g, cor.b);
    SDL_SetTextureAlphaMod(textura, (Uint8)(115.0f + 40.0f * pulso));

    SDL_RenderTexture9Grid(renderer, textura, NULL,
        24.0f, 24.0f, 24.0f, 24.0f, canto / 24.0f, &destino);
}

// O clique vale na área do destaque (texto + margens)
bool Jogo::MouseNaOpcao(float x, float y, const SDL_FRect& textoOpcao) const
{
    SDL_FRect area = {
        textoOpcao.x - margemDestaqueX,
        textoOpcao.y - margemDestaqueY,
        textoOpcao.w + 2.0f * margemDestaqueX,
        textoOpcao.h + 2.0f * margemDestaqueY
    };

    return MouseNoBotao(x, y, area);
}

SDL_FRect Jogo::RetanguloOpcaoPause(int i) const
{
    // Mesma posição usada para desenhar o texto em RenderizarPause
    return MedirTextoCentrado(font, opcoesPause[i],
        larguraTela * 0.5f,
        alturaTela * 0.18f + 120.0f + i * 70.0f);
}

void Jogo::TextoOpcaoConfig(int i, char* destino, size_t tamanho) const
{
    switch (i)
    {
    case 0:
        sprintf_s(destino, tamanho, "Tela cheia: %s", telaCheia ? "ON" : "OFF");
        break;
    case 1:
        sprintf_s(destino, tamanho, "Musica: %s", musicaLigada ? "ON" : "OFF");
        break;
    default:
        sprintf_s(destino, tamanho, "Efeitos sonoros: %s", efeitosLigados ? "ON" : "OFF");
        break;
    }
}

SDL_FRect Jogo::RetanguloOpcaoConfig(int i) const
{
    char texto[50];
    TextoOpcaoConfig(i, texto, sizeof(texto));

    return MedirTextoCentrado(fontHud, texto,
        larguraTela / 2.0f,
        alturaTela * (0.30f + 0.10f * i));
}

SDL_FRect Jogo::RetanguloBotaoResultado() const
{
    return MedirTextoCentrado(fontHud, "ENTER - Continuar",
        larguraTela / 2.0f,
        alturaTela * 0.82f);
}

bool Jogo::TelaConfiguracoes()
{
    if (estado == CONFIGURACOES)
    {
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

        DesenharTextoCentrado(renderer, font, "CONFIGURAÇÕES", preto,
            (float)(larguraTela / 2), 50);

        // Tela cheia
        
        // Opções (o destaque arredondado marca a selecionada)
        for (int i = 0; i < 3; i++)
        {
            char textoOpcao[50];
            TextoOpcaoConfig(i, textoOpcao, sizeof(textoOpcao));

            SDL_FRect rect = RetanguloOpcaoConfig(i);

            if (i == opcaoConfig)
                DesenharDestaque(rect, margemDestaqueX, margemDestaqueY, corDestaqueMarrom);

            DesenharTextoCentrado(renderer, fontHud, textoOpcao, preto,
                rect.x + rect.w / 2, rect.y);
        }

        // Voltar
        DesenharTextoEsquerda(renderer, fontHud, "ESC - Voltar", preto,
            20, (float)(alturaTela - 50));

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaControles()
{
    if (estado == CONTROLES)
    {
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

        // textos
        DesenharTextoCentrado(renderer, font, "CONTROLES", preto,
            (float)(larguraTela / 2), 50);

        float yInicial = alturaTela * 0.25f;
        float espacamento = alturaTela * 0.07f;

        DesenharTextoCentrado(renderer, fontHud, "W A S D - Movimentar", preto,
            (float)(larguraTela / 2), yInicial);

        //Controles

        DesenharTextoCentrado(renderer, fontHud, "ESPACO - Pular", preto,
            (float)(larguraTela / 2), yInicial + espacamento);

        DesenharTextoCentrado(renderer, fontHud, "BOTAO ESQUERDO DO MOUSE - Atirar", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 2);

        DesenharTextoCentrado(renderer, fontHud, "ESC - Pausar / Voltar", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 3);

        // Voltar
        DesenharTextoEsquerda(renderer, fontHud, "ESC - Voltar", preto,
            20, (float)(alturaTela - 50));

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaAjuda()
{
    if (estado == AJUDA)
    {
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 239, 194, 153, 1);

        // textos
        DesenharTextoCentrado(renderer, font, "AJUDA", preto,
            (float)(larguraTela / 2), 50);

        float yInicial = alturaTela * 0.20f;
        float espacamento = alturaTela * 0.055f;

        // Explicação
        DesenharTextoCentrado(renderer, fontHud, "OBJETIVO:", preto,
            (float)(larguraTela / 2), yInicial);

        DesenharTextoCentrado(renderer, fontHud, "Proteja a base dos inimigos.", preto,
            (float)(larguraTela / 2), yInicial + espacamento);

        DesenharTextoCentrado(renderer, fontHud, "Elimine inimigos para ganhar pontos.", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 2);

        DesenharTextoCentrado(renderer, fontHud, "BOSS:", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 4);

        DesenharTextoCentrado(renderer, fontHud, "Surge a cada 1000 pontos.", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 5);

        DesenharTextoCentrado(renderer, fontHud, "Enquanto estiver vivo, nenhum inimigo aparece.", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 6);

        DesenharTextoCentrado(renderer, fontHud, "POWER-UPS:", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 8);

        DesenharTextoCentrado(renderer, fontHud, "Colete-os para facilitar a sobrevivencia.", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 9);

        DesenharTextoCentrado(renderer, fontHud, "Se a base perder todas as vidas, o jogo termina.", preto,
            (float)(larguraTela / 2), yInicial + espacamento * 11);

        // Voltar
        DesenharTextoEsquerda(renderer, fontHud, "ESC - Voltar", preto,
            20, (float)(alturaTela - 50));

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaGameOver()
{
    if (estado == GAMEOVER)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_FRect gameoverRect = {
            0,
            0,
            (float)larguraTela,
            (float)alturaTela
        };

        SDL_RenderTexture(renderer, gameover, NULL, &gameoverRect);
        SDL_RenderPresent(renderer);

        ultimoSpawnAtq = SDL_GetTicks();

        //Limpar powerUps

        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            ataquePowerup[i].ativo = false;
        }

        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            vidasPowerup[i].ativo = false;
        }

        if (SDL_GetTicks() - tempoAtual >= 3000)
        {
            estado = RESULTADO;
            inicioResultado = SDL_GetTicks();
        }

        EsperarProximoFrame();
        return true;
    }
    return false;
}

bool Jogo::TelaResultado()
{
    if (estado == RESULTADO)
    {
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

        // Evento — Enter avança

        SDL_Color amarelo = { 255, 220,  50, 255 };
        SDL_Color cinza = { 180, 180, 180, 255 };

        float cy = alturaTela * 0.15f;
        float esp = alturaTela * 0.10f;

        DesenharTextoCentrado(renderer, font, "RESULTADO", branco, larguraTela / 2.0f, cy);

        char buf[64];

        sprintf_s(buf, sizeof(buf), "Pontos:              %d", pontos);
        DesenharTextoCentrado(renderer, fontHud, buf, amarelo, larguraTela / 2.0f, cy + esp * 1.5f);

        sprintf_s(buf, sizeof(buf), "Onda alcancada:      %d", ondas);
        DesenharTextoCentrado(renderer, fontHud, buf, cinza, larguraTela / 2.0f, cy + esp * 2.2f);

        sprintf_s(buf, sizeof(buf), "Inimigos derrotados: %d", inimigosDerrotados);
        DesenharTextoCentrado(renderer, fontHud, buf, cinza, larguraTela / 2.0f, cy + esp * 2.9f);

        sprintf_s(buf, sizeof(buf), "Bosses derrotados:   %d", bossAtivados);
        DesenharTextoCentrado(renderer, fontHud, buf, cinza, larguraTela / 2.0f, cy + esp * 3.6f);

        // Botão "Continuar": Enter ou clique esquerdo
        DesenharDestaque(RetanguloBotaoResultado(), margemDestaqueX, margemDestaqueY, corDestaqueOuro);
        DesenharTextoCentrado(renderer, fontHud, "ENTER - Continuar", branco, larguraTela / 2.0f, alturaTela * 0.82f);

        SDL_RenderPresent(renderer);
        EsperarProximoFrame();
        return true;
    }
    return false;
}