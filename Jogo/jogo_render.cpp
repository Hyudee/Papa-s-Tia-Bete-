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

void Jogo::CalcularShake() {
    shakeX = 0.0f;
    shakeY = 0.0f;

    if (shakeDuracao > 0.0f)
    {
        Uint64 tempoShake = SDL_GetTicks() - shakeInicio;

        if (tempoShake < (Uint64)shakeDuracao)
        {
            float progresso = 1.0f - (tempoShake / shakeDuracao);
            shakeX = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * shakeForca * progresso;
            shakeY = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * shakeForca * progresso;
        }
        else
        {
            shakeDuracao = 0.0f;
            shakeX = 0.0f;
            shakeY = 0.0f;
        }
    }
}

void Jogo::RenderizarMundo() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    // Aplica viewport
    SDL_Rect viewport = {
        (int)shakeX,
        (int)shakeY,
        larguraTela,
        alturaTela
    };
    SDL_SetRenderViewport(renderer, &viewport);

    // Fundo
    SDL_FRect fundo = {
        0,
        0,
        (float)larguraTela,
        (float)alturaTela
    };

    SDL_RenderTexture(renderer, background, NULL, &fundo);

    // Base
    SDL_Texture* texturaBase = baseTexture;
    int frameIndex;

    if (vidas >= 4)
        frameIndex = 0;
    else
        frameIndex = 4 - vidas;

    SDL_FRect srcBase = {
        (float)(frameIndex * larguraFrameBase),
        0.0f,
        (float)larguraFrameBase,
        (float)alturaFrameBase
    };

    if (tempoFlashBase > 0.0f)
    {
        if (flashBase == FLASH_DANO)
            SDL_SetTextureColorMod(texturaBase, 255, 80, 80);
        else if (flashBase == FLASH_CURA)
            SDL_SetTextureColorMod(texturaBase, 80, 255, 80);
    }
    else
    {
        SDL_SetTextureColorMod(texturaBase, 255, 255, 255);
    }

    SDL_RenderTexture(renderer, texturaBase, &srcBase, &base);
    SDL_SetTextureColorMod(texturaBase, 255, 255, 255);

    // Animação do player
    static Uint64 ultimoFrameAndar = 0;

    bool andando = key[SDL_SCANCODE_A] || key[SDL_SCANCODE_D];
    if (estado == JOGAR)
    {
        if (!dano && !atirando && !pulando && !aterrissando)
        {
            if (andando)
            {
                if (SDL_GetTicks() - ultimoFrameAndar > 120)
                {
                    if (olhandoDireita)
                    {
                        framePlayer++;
                        if (framePlayer < 16 || framePlayer > 19)
                            framePlayer = 16;
                    }
                    else
                    {
                        framePlayer++;
                        if (framePlayer < 20 || framePlayer > 23)
                            framePlayer = 20;
                    }
                    ultimoFrameAndar = SDL_GetTicks();
                }
            }
            else
            {
                framePlayer = olhandoDireita ? 0 : 1;
            }
        }
    }

    // Jogador
    SDL_FRect srcPlayer = {
        (float)(framePlayer * 64),
        0,
        64,
        64
    };

    SDL_Texture* texturaAtualPlayer = sujo ? texturaPlayerSujo : texturaPlayer;

    SDL_RenderTexture(renderer, texturaAtualPlayer, &srcPlayer, &player);

    // Balas
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        if (!projeteis[i].ativo)
            continue;

        SDL_FRect srcTiro = {
            (float)(projeteis[i].frame * larguraFrameTiro),
            0,
            (float)larguraFrameTiro,
            (float)alturaFrameTiro
        };

        // Gira o sprite para a direção do tiro (a explosão não gira)
        double anguloTiro = projeteis[i].explodindo ? 0.0 : (double)projeteis[i].angulo;

        SDL_RenderTextureRotated(renderer, projetil, &srcTiro, &projeteis[i].rect,
            anguloTiro, NULL, SDL_FLIP_NONE);
    }

    // Inimigos
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        if (!inimigos[i].ativo && !inimigos[i].morrendo)
            continue;

        const DadosInimigo& dados = DadosDoInimigo(inimigos[i].tipo);

        // Arte do tipo; se o arquivo nao existir, usa o cubo de acucar com a cor provisoria da tabela
        SDL_Texture* textura = texturasInimigo[inimigos[i].tipo];
        bool arteProvisoria = (textura == NULL);

        if (inimigos[i].temCor)
            SDL_SetTextureColorMod(textura, inimigos[i].corR, inimigos[i].corG, inimigos[i].corB);
        else if (arteProvisoria)
            SDL_SetTextureColorMod(textura, dados.corR, dados.corG, dados.corB);

        // A geometria da folha acompanha a textura realmente usada
        const DadosInimigo& folha = arteProvisoria ? DadosDoInimigo(INIMIGO_COMUM) : dados;

        int coluna = inimigos[i].frame % folha.framesAndar;

        // Pirulito disparando: quadros 4 e 5 da folha. A arte provisoria (cubo) so tem os quadros de andar.
        if (inimigos[i].tipo == INIMIGO_PIRULITO && inimigos[i].estadoPirulito == PIRULITO_DISPARANDO &&
            !arteProvisoria)
            coluna = inimigos[i].frame;

        int linha = EstadoDeVida(folha, inimigos[i].vida, inimigos[i].vidaMax);

        SDL_FRect src = {            // bloco dos inimigos: colunas = frames, linhas = estados de vida
            (float)(coluna * folha.frameLargura),
            (float)(linha * folha.frameAltura),
            (float)folha.frameLargura,
            (float)folha.frameAltura
        };

        // Divisor morrendo: toca a animacao de divisao (3 quadros, uma vez), sem o tom vermelho
        bool dividindo = inimigos[i].morrendo && inimigos[i].tipo == INIMIGO_DIVISOR &&
            texturaDivisaoMarshmallow != NULL;

        if (dividindo)
        {
            textura = texturaDivisaoMarshmallow;
            arteProvisoria = false;

            int frameDivisao = FrameDivisao(SDL_GetTicks() - inimigos[i].flashInicio, dados.msMorte);

            src = { (float)(frameDivisao * 64), 0.0f, 64.0f, 64.0f };
        }

        if (inimigos[i].temCor)
            SDL_SetTextureColorMod(textura, inimigos[i].corR, inimigos[i].corG, inimigos[i].corB);
        else if (arteProvisoria)
            SDL_SetTextureColorMod(textura, dados.corR, dados.corG, dados.corB);

        if (inimigos[i].morrendo && !dividindo)
            SDL_SetTextureColorMod(textura, 255, 80, 80);

        // angulo = 0 para todos os tipos, menos o Caramelo (inclina junto com o zigue-zague)
        SDL_RenderTextureRotated(renderer, textura, &src, &inimigos[i].rect,
            inimigos[i].angulo, NULL, SDL_FLIP_NONE);

        // Flash branco de acerto (camada aditiva): so quando o tiro acertou e o minion sobreviveu
        if (!inimigos[i].morrendo && inimigos[i].hitInicio != 0 &&
            SDL_GetTicks() - inimigos[i].hitInicio < (Uint64)MS_FLASH_ACERTO)
        {
            SDL_BlendMode modoAntigo = SDL_BLENDMODE_BLEND;
            SDL_GetTextureBlendMode(textura, &modoAntigo);

            SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_ADD);
            SDL_SetTextureColorMod(textura, 255, 255, 255);
            SDL_RenderTextureRotated(renderer, textura, &src, &inimigos[i].rect,
                inimigos[i].angulo, NULL, SDL_FLIP_NONE);
            SDL_SetTextureBlendMode(textura, modoAntigo);
        }

        // A textura e compartilhada: devolve a cor ao normal depois de cada minion
        SDL_SetTextureColorMod(textura, 255, 255, 255);
    }

    // Tiros dos Pirulitos: tiro_calda.png (4 quadros de 24x24) e, ao acertar, tiro_calda_impacto.png
    // (3 quadros de 32x32). Sem a arte, cai no tiro.png em vermelho (a textura e compartilhada: a cor volta ao normal).
    bool tiroProvisorio = (texturaTiroCalda == NULL);

    if (tiroProvisorio)
        SDL_SetTextureColorMod(projetil, 255, 70, 70);

    for (int i = 0; i < MAX_TIROS_INIMIGOS; i++)
    {
        if (!tirosInimigos[i].ativo)
            continue;

        if (tirosInimigos[i].tipo == TIRO_PEDACO_CHOCOLATE)
        {
            if (tirosInimigos[i].impacto)
            {
                SDL_FRect srcImp = {
                    (float)(tirosInimigos[i].frame * (int)CHOCOLATE_IMPACTO_TAMANHO),
                    0,
                    CHOCOLATE_IMPACTO_TAMANHO,
                    CHOCOLATE_IMPACTO_TAMANHO
                };

                SDL_RenderTexture(renderer, texturaPedacoImpacto, &srcImp, &tirosInimigos[i].rect);
            }
            else if (texturaPedacoChocolate != NULL)
            {
                SDL_FRect srcPedaco = {
                    (float)(tirosInimigos[i].frame * (int)CHOCOLATE_PEDACO_TAMANHO),
                    0,
                    CHOCOLATE_PEDACO_TAMANHO,
                    CHOCOLATE_PEDACO_TAMANHO
                };

                SDL_RenderTexture(renderer, texturaPedacoChocolate, &srcPedaco, &tirosInimigos[i].rect);   // sem rotacao: gira pelos quadros
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0x88, 0x50, 0x38, 255);
                SDL_RenderFillRect(renderer, &tirosInimigos[i].rect);
            }

            continue;
        }

        if (tirosInimigos[i].impacto)
        {
            SDL_FRect srcImpacto = {
                (float)(tirosInimigos[i].frame * (int)TIRO_IMPACTO_TAMANHO),
                0,
                TIRO_IMPACTO_TAMANHO,
                TIRO_IMPACTO_TAMANHO
            };

            SDL_RenderTexture(renderer, texturaTiroCaldaImpacto, &srcImpacto, &tirosInimigos[i].rect);
            continue;
        }

        if (tiroProvisorio)
        {
            SDL_FRect srcTiroInimigo = {
                (float)((tirosInimigos[i].frame % 3) * larguraFrameTiro),
                0,
                (float)larguraFrameTiro,
                (float)alturaFrameTiro
            };

            SDL_RenderTextureRotated(renderer, projetil, &srcTiroInimigo, &tirosInimigos[i].rect,
                (double)tirosInimigos[i].angulo, NULL, SDL_FLIP_NONE);
        }
        else
        {
            SDL_FRect srcTiroInimigo = {
                (float)(tirosInimigos[i].frame * (int)TIRO_INIMIGO_TAMANHO),
                0,
                TIRO_INIMIGO_TAMANHO,
                TIRO_INIMIGO_TAMANHO
            };

            SDL_RenderTextureRotated(renderer, texturaTiroCalda, &srcTiroInimigo, &tirosInimigos[i].rect,
                (double)tirosInimigos[i].angulo, NULL, SDL_FLIP_NONE);
        }
    }

    if (tiroProvisorio)
        SDL_SetTextureColorMod(projetil, 255, 255, 255);

    // Obstáculos
    for (int i = 0; i < MAX_OBSTACULOS; i++)
    {
        if (!obstaculos[i].ativo)
            continue;

        SDL_FRect srcSombra = {
            (float)(obstaculos[i].frame * 64),
            0,
            64,
            64
        };

        SDL_FRect sombraRect = {
            obstaculos[i].rect.x,
            (float)(alturaTela - tamanhoObstaculo),
            tamanhoObstaculo,
            tamanhoObstaculo
        };

        if (obstaculos[i].estado == OBSTACULO_SOMBRA)
        {
            SDL_RenderTexture(renderer, texturaSombra, &srcSombra, &sombraRect);
        }
        else if (obstaculos[i].estado == OBSTACULO_CAINDO)
        {
            SDL_RenderTexture(renderer, texturaSombra, &srcSombra, &sombraRect);

            SDL_FRect srcObstaculo = {
                (float)(obstaculos[i].frame * 64),
                0,
                64,
                64
            };

            SDL_RenderTexture(renderer, texturaObstaculo, &srcObstaculo, &obstaculos[i].rect);
        }
        else
        {
            SDL_FRect srcObstaculo = {
                obstaculos[i].frame * 64,
                0,
                64,
                64
            };

            SDL_RenderTexture(renderer, texturaObstaculo, &srcObstaculo, &obstaculos[i].rect);
        }
    }

    // Boss
    if (boss.ativo)
    {
        AnimBossId idAnim = AnimacaoDoBoss(boss.tipo, boss.estado, boss.estagioArte);
        const AnimBoss& anim = TABELA_ANIM_BOSS[idAnim];

        SDL_Texture* texBoss = texturasBoss[idAnim];
        bool provisorio = false;

        if (!texBoss)   // arte do tipo ausente: usa o chocolate com cor provisoria
        {
            texBoss = texturaBoss;
            provisorio = true;
        }

        int frameBoss = FrameExibidoBoss(boss.estado, boss.frame, anim.frames);   // saida do escudo toca ao contrario

        SDL_FRect srcBoss = {
            (float)(frameBoss * LARGURA_FRAME_BOSS),
            0,
            (float)LARGURA_FRAME_BOSS,
            (float)LARGURA_FRAME_BOSS
        };

        if (provisorio)
            SDL_SetTextureColorMod(texBoss, anim.corR, anim.corG, anim.corB);

        // Aviso do Touro: treme so no desenho (o rect, e a colisao, ficam parados)
        SDL_FRect destBoss = boss.rect;

        if (boss.estado == BOSS_AVISANDO)
        {
            float t = (float)SDL_GetTicks();
            destBoss.x += SDL_sinf(t * 0.08f) * 4.0f * escalaJogo;
            destBoss.y += SDL_cosf(t * 0.11f) * 2.0f * escalaJogo;
        }

        SDL_RenderTexture(renderer, texBoss, &srcBoss, &destBoss);

        if (provisorio)
            SDL_SetTextureColorMod(texBoss, 255, 255, 255);   // textura compartilhada: sempre restaurar
    }

    // Faiscas de bloqueio do escudo (desenhadas por cima do boss)
    if (texturaFaiscaEscudo != NULL)
    {
        for (int i = 0; i < MAX_FAISCAS_ESCUDO; i++)
        {
            if (!faiscas[i].ativa)
                continue;

            int f = (int)((SDL_GetTicks() - faiscas[i].inicio) / FAISCA_MS_FRAME);
            if (f >= FAISCA_FRAMES) f = FAISCA_FRAMES - 1;

            float t = FAISCA_TAMANHO * escalaJogo;

            SDL_FRect srcFaisca = { (float)(f * (int)FAISCA_TAMANHO), 0, FAISCA_TAMANHO, FAISCA_TAMANHO };
            SDL_FRect dstFaisca = { faiscas[i].x - t / 2.0f, faiscas[i].y - t / 2.0f, t, t };

            SDL_RenderTexture(renderer, texturaFaiscaEscudo, &srcFaisca, &dstFaisca);
        }
    }

    // Powerups
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (!vidasPowerup[i].ativo)
            continue;

        SDL_FRect src = {            // powerup de vida
            (float)(vidasPowerup[i].frame * larguraFramePowerUp),
            0,
            (float)larguraFramePowerUp,
            (float)alturaFramePowerUp
        };

        SDL_FRect dest = vidasPowerup[i].rect;
        dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

        SDL_RenderTexture(renderer, powerUpVida, &src, &dest);
    }

    for (int i = 0; i < MAX_POWERUPSATQ; i++)
    {
        if (!ataquePowerup[i].ativo)
            continue;

        SDL_FRect src = {            // powerup de ataque
            (float)(ataquePowerup[i].frame * larguraFramePowerUp),
            0,
            (float)larguraFramePowerUp,
            (float)alturaFramePowerUp
        };

        SDL_FRect dest = ataquePowerup[i].rect;
        dest.y += SDL_sinf(SDL_GetTicks() * 0.005f) * 4;

        SDL_RenderTexture(renderer, powerUpAtq, &src, &dest);
    }

    // Explosão Açucarada: disco translúcido que cresce e some
    SDL_BlendMode modoAnterior;
    SDL_GetRenderDrawBlendMode(renderer, &modoAnterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < MAX_EXPLOSOES; i++)
    {
        if (!explosoes[i].ativa)
            continue;

        float t = (float)(SDL_GetTicks() - explosoes[i].inicio) / (float)DURACAO_EXPLOSAO;

        if (t > 1.0f)
            t = 1.0f;

        float raio = explosoes[i].raio * (0.3f + 0.7f * t);

        const int SEGMENTOS = 24;
        SDL_Vertex vertices[SEGMENTOS + 1];
        int indices[SEGMENTOS * 3];
        SDL_FColor cor = { 1.0f, 0.65f, 0.15f, 0.6f * (1.0f - t) };

        vertices[0].position.x = explosoes[i].x;
        vertices[0].position.y = explosoes[i].y;
        vertices[0].color = cor;
        vertices[0].tex_coord.x = 0.0f;
        vertices[0].tex_coord.y = 0.0f;

        for (int k = 0; k < SEGMENTOS; k++)
        {
            float angulo = 6.2831853f * k / SEGMENTOS;

            vertices[k + 1].position.x = explosoes[i].x + SDL_cosf(angulo) * raio;
            vertices[k + 1].position.y = explosoes[i].y + SDL_sinf(angulo) * raio;
            vertices[k + 1].color = cor;
            vertices[k + 1].tex_coord.x = 0.0f;
            vertices[k + 1].tex_coord.y = 0.0f;

            indices[k * 3] = 0;
            indices[k * 3 + 1] = k + 1;
            indices[k * 3 + 2] = (k + 1) % SEGMENTOS + 1;
        }

        SDL_RenderGeometry(renderer, NULL, vertices, SEGMENTOS + 1, indices, SEGMENTOS * 3);
    }

    SDL_SetRenderDrawBlendMode(renderer, modoAnterior);
}    

void Jogo::RenderizarHud() {
    SDL_SetRenderViewport(renderer, NULL);

    // Barra de vida do boss
    if (boss.ativo)
    {
        SDL_FRect fundoVida = {
            (float)(larguraTela / 2 - 200),
            20,
            400,
            25
        };

        SDL_FRect vidaBoss = {
            (float)(larguraTela / 2 - 200),
            20,
            400.0f * ((float)boss.vida / boss.vidaMax),
            25
        };

        SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
        SDL_RenderFillRect(renderer, &fundoVida);

        if (BossInvulneravel(boss.tipo, boss.estado))
            SDL_SetRenderDrawColor(renderer, 120, 180, 255, 255);   // escudo ativo: barra azul
        else
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        SDL_RenderFillRect(renderer, &vidaBoss);
    }

    // Vidas
    for (int i = 0; i < vidas; i++)
    {
        SDL_FRect vida = {
            (float)(10 + (i * 40)),
            10,
            32,
            32
        };

        SDL_RenderTexture(renderer, coracao, NULL, &vida);
    }

    // Pontuação
    SDL_GetTextureSize(texturaPontos, &textoW, &textoH);
    textoRect.x = larguraTela - textoW - 10;
    textoRect.y = 10;
    textoRect.w = textoW;
    textoRect.h = textoH;
    SDL_RenderTexture(renderer, texturaPontos, NULL, &textoRect);

    // Ondas
    SDL_GetTextureSize(TexturaOndas, &textoW, &textoH);
    textoRect.x = larguraTela - textoW - 10;
    textoRect.y = 40;
    textoRect.w = textoW;
    textoRect.h = textoH;
    SDL_RenderTexture(renderer, TexturaOndas, NULL, &textoRect);

    // Buff
    if (ataqueBuff)
    {
        SDL_FRect buffRect;
        SDL_GetTextureSize(texturaBuff, &buffRect.w, &buffRect.h);
        buffRect.x = 10;
        buffRect.y = 50;
        SDL_RenderTexture(renderer, texturaBuff, NULL, &buffRect);
    }


    // Em Pause/Upgrade o relógio dos indicadores fica parado em inicioPausa
    Uint64 agora = (estado == JOGAR) ? SDL_GetTicks() : inicioPausa;

    // Barreira da Base: uma caixa por carga (azul = carregada) e barra de recarga
    if (barreirasMax > 0)
    {
        for (int i = 0; i < barreirasMax; i++)
        {
            SDL_FRect caixa = { 10.0f + i * 34.0f, 90.0f, 28.0f, 14.0f };

            if (i < barreiraCargas)
                SDL_SetRenderDrawColor(renderer, 90, 170, 255, 255);
            else
                SDL_SetRenderDrawColor(renderer, 50, 60, 80, 255);

            SDL_RenderFillRect(renderer, &caixa);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderRect(renderer, &caixa);
        }

        if (barreiraCargas < barreirasMax)
        {
            float decorrido = (agora > inicioRecargaBarreira) ? (float)(agora - inicioRecargaBarreira) : 0.0f;
            float progresso = SDL_min(1.0f, decorrido / (float)RECARGA_BARREIRA);
            float larguraTotal = barreirasMax * 34.0f - 6.0f;

            SDL_FRect fundo = { 10.0f, 108.0f, larguraTotal, 4.0f };
            SDL_FRect cheio = { 10.0f, 108.0f, larguraTotal * progresso, 4.0f };

            SDL_SetRenderDrawColor(renderer, 50, 60, 80, 255);
            SDL_RenderFillRect(renderer, &fundo);
            SDL_SetRenderDrawColor(renderer, 90, 170, 255, 255);
            SDL_RenderFillRect(renderer, &cheio);
        }
    }


    // Fúria de Combo: amarela enche com os abates; laranja esvazia enquanto está ativa
    if (furiaDanoPercent > 0)
    {
        SDL_FRect fundo = { 10.0f, 120.0f, 100.0f, 10.0f };
        SDL_Color corBarra;
        float fracao;

        if (furiaAtiva)
        {
            float decorrido = (agora > furiaInicio) ? (float)(agora - furiaInicio) : 0.0f;
            fracao = SDL_max(0.0f, 1.0f - decorrido / (float)DURACAO_FURIA);
            corBarra = { 255, 140, 0, 255 };
        }
        else
        {
            fracao = (float)sequenciaAbates / (float)ABATES_FURIA;
            corBarra = { 230, 200, 60, 255 };
        }

        SDL_FRect cheio = { fundo.x, fundo.y, fundo.w * fracao, fundo.h };

        SDL_SetRenderDrawColor(renderer, 60, 40, 20, 255);
        SDL_RenderFillRect(renderer, &fundo);
        SDL_SetRenderDrawColor(renderer, corBarra.r, corBarra.g, corBarra.b, 255);
        SDL_RenderFillRect(renderer, &cheio);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &fundo);
    }
}

void Jogo::RenderizarPause() {
    if (estado == PAUSE)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);

        SDL_FRect fundoPause = {
            0,
            0,
            (float)larguraTela,
            (float)alturaTela
        };

        SDL_RenderFillRect(renderer, &fundoPause);

        SDL_FRect painel = {
            larguraTela * 0.30f,
            alturaTela * 0.18f,
            larguraTela * 0.40f,
            alturaTela * 0.60f
        };

        SDL_SetRenderDrawColor(renderer, 240, 220, 180, 160);
        SDL_RenderFillRect(renderer, &painel);

        SDL_SetRenderDrawColor(renderer, 90, 60, 30, 255);
        SDL_RenderRect(renderer, &painel);

        DesenharTextoCentrado(renderer, font, "PAUSADO", branco,
            painel.x + painel.w / 2, painel.y + 30);

        for (int i = 0; i < 3; i++)
        {
            SDL_FRect rect = RetanguloOpcaoPause(i);

            if (i == opcaoPause)
                DesenharDestaque(rect, margemDestaqueX, margemDestaqueY, corDestaqueMarrom);

            SDL_Color cor = (i == opcaoPause) ? preto : branco;
            DesenharTextoCentrado(renderer, font, opcoesPause[i], cor,
                rect.x + rect.w / 2, rect.y);
        }
    }
}

// Imagem de cada carta (as texturas pertencem ao Assets)
SDL_Texture* Jogo::ImagemDaCarta(IdCarta id) const
{
    switch (id)
    {
    case CARTA_MAIS_DANO:    return upgradeImgDano;
    case CARTA_MAIS_VIDA:    return upgradeImgVida;
    case CARTA_MAIS_BALAS:   return upgradeImgBalas;
    case CARTA_TIRO_DUPLO:   return upgradeImgDuplo;
    case CARTA_CACADOR_BOSS: return upgradeImgCacador;
	case CARTA_CADENCIA:     return upgradeImgCadencia;
    case CARTA_BALA_PESADA:  return upgradeImgBalaPesada;
    case CARTA_PERNAS_ACUCAR:return upgradeImgPernas;
    case CARTA_DOCE_SORTE:   return upgradeImgDoceSorte;
    case CARTA_GLACE:        return upgradeImgGlace;
    case CARTA_PERFURANTE:   return upgradeImgPerfurante;
    case CARTA_BARREIRA:     return upgradeImgBarreira;
    case CARTA_EXPLOSAO:     return upgradeImgExplosao;
	case CARTA_FURIA:        return upgradeImgFuria;
    case CARTA_PACTO_AMARGO: return upgradeImgPacto;

    default:                 return nullptr;
    }
}