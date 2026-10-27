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

void Jogo::AtualizarHoverMenu()
{
    if (estado == MENU)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int novoHoverBotao = -1;

        if (MouseNoBotao(mouseX, mouseY, botaoJogar))
            novoHoverBotao = 0;
        else if (MouseNoBotao(mouseX, mouseY, botaoCreditos))
            novoHoverBotao = 1;
        else if (MouseNoBotao(mouseX, mouseY, botaoSair))
            novoHoverBotao = 2;

        if (novoHoverBotao != hoverBotaoAtual &&
            novoHoverBotao != -1)
        {
            TocarEfeito(somHoverBotao);
            itemMenu = novoHoverBotao; // 0 a 2 coincidem com os botões
        }

        hoverBotaoAtual = novoHoverBotao;

        int novoHoverAba = -1;

        if (MouseNoBotao(mouseX, mouseY, abaRanking))
            novoHoverAba = 0;
        else if (MouseNoBotao(mouseX, mouseY, abaConfig))
            novoHoverAba = 1;
        else if (MouseNoBotao(mouseX, mouseY, abaControles))
            novoHoverAba = 2;
        else if (MouseNoBotao(mouseX, mouseY, abaAjuda))
            novoHoverAba = 3;

        if (novoHoverAba != hoverAbaAtual &&
            novoHoverAba != -1)
        {
            TocarEfeito(somHoverAbas);

            // Ranking, Engrenagem, Controles, Ajuda -> itens do menu
            const int itemPorAba[4] = { 5, 3, 4, 6 };
            itemMenu = itemPorAba[novoHoverAba];
        }

        hoverAbaAtual = novoHoverAba;
    }
}

void Jogo::GerenciarMusica()
{
    if (estado == MENU)
    {
        TocarMusica(musicaMenu,
            "assets/menu.wav");

        AtualizarMusica(musicaMenu);
    }
    else
    {
        PararMusica(musicaMenu);
    }

    // Musica da Gameplay
    if (estado == JOGAR && !boss.ativo) {
        TocarMusica(musicaGame, "assets/jogar.wav");
        AtualizarMusica(musicaGame);
    }
    else { PararMusica(musicaGame); }

    // Musica do Boss
    if (estado == JOGAR && boss.ativo) {
        TocarMusica(musicaBoss, "assets/boss.wav");
        AtualizarMusica(musicaBoss);
    }
    else { PararMusica(musicaBoss); }

    // Musica da tela de gameover
    if (estado == GAMEOVER)
    {
        TocarMusica(musicaGameOver,
            "assets/gameover.wav");

        AtualizarMusica(musicaGameOver);
    }
    else
    {
        PararMusica(musicaGameOver);
    }

    // Musica dos creditos
    if (estado == CREDITOS)
    {
        TocarMusica(musicaCreditos,
            "assets/creditos.wav");

        AtualizarMusica(musicaCreditos);
    }
    else
    {
        PararMusica(musicaCreditos);
    }
}

void Jogo::AtualizarAnimacoes()
{
    if (estado == JOGAR) {

        if (SDL_GetTicks() - ultimoFrameBase > 150)
        {
            frameBase++;

            if (frameBase >= totalFramesBase)
                frameBase = 0;

            ultimoFrameBase = SDL_GetTicks();
        }
    }

    // Atualização do flash da base
    if (estado == JOGAR) {

        if (tempoFlashBase > 0.0f)
        {
            tempoFlashBase -= 1.0f / 60.0f;

            if (tempoFlashBase <= 0.0f)
            {
                tempoFlashBase = 0.0f;
                flashBase = FLASH_NENHUM;
            }
        }
    }

    // Atualização da animação do PowerUp de Vida
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            if (!vidasPowerup[i].ativo)
                continue;

            if (SDL_GetTicks() - vidasPowerup[i].ultimoFrame > 120)
            {
                vidasPowerup[i].frame++;

                if (vidasPowerup[i].frame >= totalFramesPowerUp)
                    vidasPowerup[i].frame = 0;

                vidasPowerup[i].ultimoFrame = SDL_GetTicks();
            }
        }
    }
    // Atualização da animação do PowerUp de Ataque

    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (!ataquePowerup[i].ativo)
                continue;

            if (SDL_GetTicks() - ataquePowerup[i].ultimoFrame > 120)
            {
                ataquePowerup[i].frame++;

                if (ataquePowerup[i].frame >= totalFramesPowerUp)
                    ataquePowerup[i].frame = 0;

                ataquePowerup[i].ultimoFrame = SDL_GetTicks();
            }
        }
    }
}

void Jogo::AtualizarOndasEBuffs()
{
    AtualizarEfeitosDasCartas();

    if (estado == JOGAR && SDL_GetTicks() - inicioOnda >= 30000)
    {
        ondas++;

        inicioOnda = SDL_GetTicks();

        spam = true;
        inicioSpam = SDL_GetTicks();

        inimigosAtivos = MaximoInimigosVivos(ondas);

        intervaloSpawn = IntervaloSpawn(ondas);

        // Libera os tipos de minion que ja podem nascer nesta onda (cada spawn sorteia entre eles)
        totalTiposAtivos = ListarTiposLiberados(ondas, tiposAtivos);
    }
}

void Jogo::AtualizarSpawn() {
    if (estado == JOGAR) {
        int vivos = 0;

        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].ativo)
            {
                vivos++;
            }
        }

        // Spawn automático
        tempoAtual = SDL_GetTicks();

        if (!boss.ativo && tempoAtual - ultimoSpawn >= (Uint64)intervaloSpawn && vivos < inimigosAtivos)
        {
            ultimoSpawn = tempoAtual;

            int quantidadeSpawn = QuantidadePorSpawn(ondas);

            // Não passa do máximo de inimigos vivos
            if (quantidadeSpawn > inimigosAtivos - vivos)
                quantidadeSpawn = inimigosAtivos - vivos;

            int criados = 0;            // quantos minions ja nasceram neste spawn (o bando conta cada membro)
            int granuladosNoSpawn = 0;  // granulados deste spawn, somando todos os bandos (maximo MAX_GRANULADOS_POR_SPAWN)


            int pirulitosVivos = 0;     // atiradores na tela (limite MAX_PIRULITOS)

            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (inimigos[i].ativo && inimigos[i].tipo == INIMIGO_PIRULITO)
                    pirulitosVivos++;
            }

            for (int s = 0; s < quantidadeSpawn; s++)
            {
                int vagas = VagasNoTeto(inimigosAtivos, vivos, criados);

                if (vagas <= 0)
                    break;

                TipoInimigo tipo = SortearTipoInimigo(tiposAtivos, totalTiposAtivos);


                // Ja ha Pirulitos demais na tela: este vira minion comum
                tipo = LimitarPirulitos(tipo, pirulitosVivos);

                // Granulado nasce em bando; os outros tipos nascem um por vez
                int membros = 1;

                if (tipo == INIMIGO_GRANULADO)
                {
                    // O bando nunca passa do teto de vivos nem do limite de granulados por spawn
                    membros = MembrosDoBando(TamanhoBandoGranulado(), vagas, granuladosNoSpawn);

                    if (membros <= 0)
                        continue;   // ja nasceram granulados demais neste spawn
                }

                float altura = tamanhoInimigo * DadosDoInimigo(tipo).fatorTamanho;

                int faixaY = FaixaYSpawn(tipo, alturaTela, (int)altura);   // Pirulito: so a metade superior

                for (int m = 0; m < membros; m++)
                {
                    float x = larguraTela + (s * 70) + (m * 30) + (rand() % 20);
                    float y = (float)(rand() % faixaY);   // cada membro nasce numa altura propria

                    if (SpawnarInimigo(tipo, x, y))
                    {
                        criados++;

                        if (tipo == INIMIGO_GRANULADO)
                            granuladosNoSpawn++;

                        if (tipo == INIMIGO_PIRULITO)
                            pirulitosVivos++;
                    }
                }
            }
        }

        // Spawn do coração (powerup de vida)
        // O relógio roda sempre, com a vida cheia ou não. Quando o tempo sorteado passa, ele
        // recomeça com um novo sorteio (70% a 130% do intervalo) e o coração só nasce se a vida
        // estiver abaixo do máximo. Assim, tomar dano não faz o coração nascer na hora.
        Uint64 esperaCoracao = intervaloSpawnPowerUp * sorteioIntervaloCoracao / 100
            * intervaloCoracaoPercent / 100;

        if (tempoAtual - ultimoSpawnPowerUp >= esperaCoracao) {
            ultimoSpawnPowerUp = tempoAtual;
            sorteioIntervaloCoracao = 70 + rand() % 61;

            for (int i = 0; vidas < vidaMaxima && i < MAX_POWERUPS; i++) {
                if (!vidasPowerup[i].ativo) {
                    vidasPowerup[i].ativo = true;
                    vidasPowerup[i].frame = 0;
                    vidasPowerup[i].ultimoFrame = SDL_GetTicks();

                    vidasPowerup[i].rect.w = tamanhoPowerup;

                    vidasPowerup[i].rect.h = tamanhoPowerup;

                    vidasPowerup[i].rect.x = larguraTela;

                    vidasPowerup[i].rect.y =
                        rand() % (alturaTela - (int)vidasPowerup[i].rect.h);

                    vidasPowerup[i].speed = 7.0f * escalaJogo * velocidadeCoracaoPercent / 100.0f;

                    break;
                }
            }
        }

        // SPAWN DOS OBSTÁCULOS

        if (ondas >= 2 &&
            !boss.ativo &&
            SDL_GetTicks() - ultimoSpawnObstaculo >= intervaloObstaculo)
        {
            ultimoSpawnObstaculo = SDL_GetTicks();

            for (int i = 0; i < MAX_OBSTACULOS; i++)
            {
                if (!obstaculos[i].ativo)
                {
                    obstaculos[i].ativo = true;

                    obstaculos[i].estado = OBSTACULO_SOMBRA;

                    obstaculos[i].frame = 0;
                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                    obstaculos[i].inicioEstado = SDL_GetTicks();

                    obstaculos[i].velocidadeY = 4.0f * escalaJogo;

                    obstaculos[i].rect.w = tamanhoObstaculo;
                    obstaculos[i].rect.h = tamanhoObstaculo;

                    // posição aleatória (à direita da base, nunca dentro dela)
                    int faixaSpawn = larguraTela - (int)base.w - (int)obstaculos[i].rect.w;
                    if (faixaSpawn < 1)
                        faixaSpawn = 1;
                    obstaculos[i].rect.x = base.w + (float)(rand() % faixaSpawn);

                    // começa acima da tela
                    obstaculos[i].rect.y = -obstaculos[i].rect.h;

                    break;
                }
            }
        }

        int buffsAtivos = 0;

        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (ataquePowerup[i].ativo)
                buffsAtivos++;
        }

        if (tempoAtual - ultimoSpawnAtq >= intervaloSpawnAtq &&
            buffsAtivos == 0)
        {
            ultimoSpawnAtq = tempoAtual;

            intervaloSpawnAtq =
                20000 + rand() % 30000;

            ataquePowerup[0].ativo = true;
            ataquePowerup[0].frame = 0;
            ataquePowerup[0].ultimoFrame = SDL_GetTicks();

            ataquePowerup[0].rect.w = tamanhoPowerup;
            ataquePowerup[0].rect.h = tamanhoPowerup;

            ataquePowerup[0].rect.x = larguraTela;
            ataquePowerup[0].rect.y =
                rand() % (alturaTela - (int)ataquePowerup[0].rect.h);

            ataquePowerup[0].speed = 7.0f * escalaJogo;
        }
    }
}

void Jogo::AtualizarProjeteis() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_PROJETEIS; i++)
        {
            if (!projeteis[i].ativo)
                continue;

            // Se não estiver explodindo, movimenta
            if (!projeteis[i].explodindo)
            {
                projeteis[i].rect.x += projeteis[i].velX;
                projeteis[i].rect.y += projeteis[i].velY;

                // Frames de voo (0,1,2)
                if (SDL_GetTicks() - projeteis[i].ultimoFrame > 80)
                {
                    projeteis[i].frame++;

                    if (projeteis[i].frame > 2)
                        projeteis[i].frame = 0;

                    projeteis[i].ultimoFrame = SDL_GetTicks();
                }
            }
            else
            {
                // Frames de explosão (3,4)
                if (SDL_GetTicks() - projeteis[i].ultimoFrame > 100)
                {
                    projeteis[i].frame++;

                    if (projeteis[i].frame > 4)
                    {
                        projeteis[i].ativo = false;
                    }

                    projeteis[i].ultimoFrame = SDL_GetTicks();
                }
            }

            if (projeteis[i].rect.x < 0 ||
                projeteis[i].rect.x > larguraTela ||
                projeteis[i].rect.y < 0 ||
                projeteis[i].rect.y > alturaTela)
            {
                projeteis[i].ativo = false;
            }
        }
    }
}

bool Jogo::SpawnarInimigo(TipoInimigo tipo, float x, float y)
{
    const DadosInimigo& dados = DadosDoInimigo(tipo);

    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        if (inimigos[i].ativo)
            continue;

        inimigos[i].ativo = true;
        inimigos[i].morrendo = false;
        inimigos[i].tipo = tipo;
        inimigos[i].vida = dados.vida;
        inimigos[i].vidaMax = dados.vida;
        inimigos[i].hitInicio = 0;
        inimigos[i].ultimoIdDisparo = 0;

        // Cor propria so para o Granulado (a vaga pode ter sido de outro tipo: sempre reescrever)
        inimigos[i].temCor = false;

        if (tipo == INIMIGO_GRANULADO)
        {
            CorRGB cor = CorAleatoriaGranulado();

            inimigos[i].temCor = true;
            inimigos[i].corR = cor.r;
            inimigos[i].corG = cor.g;
            inimigos[i].corB = cor.b;
        }

        inimigos[i].frame = 0;
        inimigos[i].ultimoFrame = SDL_GetTicks();

        inimigos[i].rect.w = tamanhoInimigo * dados.fatorTamanho;
        inimigos[i].rect.h = tamanhoInimigo * dados.fatorTamanho;
        
        inimigos[i].rect.x = x;
        inimigos[i].rect.y = y;

        inimigos[i].yBase = y;
        inimigos[i].fase = 0.0f;
        inimigos[i].angulo = 0.0f;

        // Pirulito (a vaga pode ter sido de outro tipo: sempre reescrever)
        inimigos[i].estadoPirulito = PIRULITO_INDO;
        inimigos[i].alvoX = 0.0f;
        inimigos[i].ultimoDisparo = 0;
        inimigos[i].disparoInicio = 0;
        inimigos[i].atirou = false;

        if (tipo == INIMIGO_PIRULITO)
        {
            // Para num x aleatorio, nunca atras da base
            inimigos[i].alvoX = AlvoXPirulito(base.x + base.w, (float)larguraTela, inimigos[i].rect.w,
                PIRULITO_MARGEM_BASE * escalaJogo);
        }

        // Caramelo: zigue-zague em torno de uma linha central que deixa a onda inteira dentro da tela
        if (tipo == INIMIGO_CARAMELO)
        {
            float amplitude = CARAMELO_AMPLITUDE * escalaJogo;

            inimigos[i].yBase = YBaseCaramelo(y, amplitude, (float)alturaTela, inimigos[i].rect.h);
            inimigos[i].fase = (float)(rand() % 628) / 100.0f;   // fase inicial aleatoria (0 a 2 pi)
            inimigos[i].rect.y = CarameloY(inimigos[i].yBase, amplitude, inimigos[i].fase,
                (float)alturaTela, inimigos[i].rect.h);
            inimigos[i].angulo = CarameloAngulo(inimigos[i].fase);
        }

        inimigos[i].speed = VelocidadeInimigo(ondas) * dados.fatorVelocidade * escalaJogo;

        if (spam)
            inimigos[i].speed += 0.5f * escalaJogo;

        return true;
    }

    return false;
}

// O Divisor terminou a animacao de divisao: nascem os filhotes.
// Os filhotes NAO contam no teto de vivos (inimigosAtivos); so a vaga no array limita.
// Se faltar vaga, nasce so o que couber.
void Jogo::DividirDivisor(SDL_FRect pai)
{
    contadores.divisoes++;
    float tamFilhote = tamanhoInimigo * DadosDoInimigo(INIMIGO_FILHOTE).fatorTamanho;

    for (int k = 0; k < FILHOTES_POR_DIVISAO; k++)
    {
        float x, y;
        PosicaoFilhote(pai.x, pai.y, pai.w, pai.h, tamFilhote, k, (float)alturaTela, &x, &y);

        if (!SpawnarInimigo(INIMIGO_FILHOTE, x, y))
            break;
    }
}

// Pirulito, o atirador. Tres estados:
//   INDO       anda para a esquerda ate alvoX (que fica sempre a frente da base)
//   PAIRANDO   parado, oscila de leve; quando passa PIRULITO_INTERVALO_TIRO_MS, comeca a disparar
//   DISPARANDO toca os quadros 4 e 5 da folha; o tiro sai quando o quadro 5 aparece;
//              no fim volta a pairar e a espera do proximo tiro conta a partir de AGORA
void Jogo::AtualizarPirulito(Inimigo& p)
{
    Uint64 agora = SDL_GetTicks();
    const DadosInimigo& dados = DadosDoInimigo(INIMIGO_PIRULITO);

    if (p.estadoPirulito == PIRULITO_DISPARANDO)
    {
        Uint64 decorrido = agora - p.disparoInicio;

        p.frame = PIRULITO_FRAME_DISPARO_INICIO + FrameDisparoPirulito(decorrido);

        if (!p.atirou && TiroSaiAgora(decorrido))
        {
            DispararTiroInimigo(p);
            p.atirou = true;
        }

        if (DisparoTerminou(decorrido))
        {
            p.estadoPirulito = PIRULITO_PAIRANDO;
            p.ultimoDisparo = agora;
            p.frame = 0;
            p.ultimoFrame = agora;
        }

        return;
    }

    // Voo (indo ou pairando): quadros 0 a 3 em loop
    if (agora - p.ultimoFrame > (Uint64)dados.msPorFrame)
    {
        p.frame = (p.frame + 1) % dados.framesAndar;
        p.ultimoFrame = agora;
    }

    if (p.estadoPirulito == PIRULITO_INDO)
    {
        p.rect.x -= p.speed;

        if (p.rect.x <= p.alvoX)
        {
            p.rect.x = p.alvoX;
            p.estadoPirulito = PIRULITO_PAIRANDO;
            p.ultimoDisparo = agora;    // o primeiro tiro vem depois de um intervalo inteiro
        }
    }
    else    // PAIRANDO
    {
        p.fase += PIRULITO_PAIRAR_PASSO;
        p.rect.y = CarameloY(p.yBase, PIRULITO_PAIRAR_AMPLITUDE * escalaJogo, p.fase,
            (float)alturaTela, p.rect.h);

        if (agora - p.ultimoDisparo >= (Uint64)PIRULITO_INTERVALO_TIRO_MS)
        {
            p.estadoPirulito = PIRULITO_DISPARANDO;
            p.disparoInicio = agora;
            p.atirou = false;
        }
    }
}

// Cria o tiro na boca do Pirulito, mirando na posicao do jogador NESTE instante (nao persegue).
// Sem vaga no vetor, o tiro e perdido (nunca estoura o array).
void Jogo::DispararTiroInimigo(const Inimigo& p)
{
    for (int k = 0; k < MAX_TIROS_INIMIGOS; k++)
    {
        if (tirosInimigos[k].ativo)
            continue;

        float tam = TIRO_INIMIGO_TAMANHO * escalaJogo;        float bocaX = p.rect.x + p.rect.w * PIRULITO_BOCA_X;
        float bocaY = p.rect.y + p.rect.h * PIRULITO_BOCA_Y;

        MiraDoTiro mira = MirarTiro(bocaX, bocaY,
            player.x + player.w / 2.0f, player.y + player.h / 2.0f,
            TIRO_INIMIGO_VELOCIDADE * escalaJogo);

        tirosInimigos[k].ativo = true;
        tirosInimigos[k].rect = { bocaX - tam / 2.0f, bocaY - tam / 2.0f, tam, tam };
        tirosInimigos[k].velX = mira.velX;
        tirosInimigos[k].velY = mira.velY;
        tirosInimigos[k].angulo = mira.angulo;
        tirosInimigos[k].frame = 0;
        tirosInimigos[k].ultimoFrame = SDL_GetTicks();
        tirosInimigos[k].impacto = false;
        return;
    }
}

void Jogo::AtualizarTirosInimigos()
{
    if (estado != JOGAR)
        return;

    for (int k = 0; k < MAX_TIROS_INIMIGOS; k++)
    {
        if (!tirosInimigos[k].ativo)
            continue;

        // Impacto: parado, toca os quadros de tiro_calda_impacto.png uma vez e some
        if (tirosInimigos[k].impacto)
        {
            if (SDL_GetTicks() - tirosInimigos[k].ultimoFrame > (Uint64)TIRO_IMPACTO_MS_FRAME)
            {
                tirosInimigos[k].frame++;
                tirosInimigos[k].ultimoFrame = SDL_GetTicks();

                if (tirosInimigos[k].frame >= TIRO_IMPACTO_FRAMES)
                    tirosInimigos[k].ativo = false;
            }

            continue;
        }

        tirosInimigos[k].rect.x += tirosInimigos[k].velX;
        tirosInimigos[k].rect.y += tirosInimigos[k].velY;

        // Quadros de voo (tiro_calda.png tem 4)
        if (SDL_GetTicks() - tirosInimigos[k].ultimoFrame > (Uint64)TIRO_INIMIGO_MS_FRAME)
        {
            tirosInimigos[k].frame = (tirosInimigos[k].frame + 1) % TIRO_INIMIGO_FRAMES;
            tirosInimigos[k].ultimoFrame = SDL_GetTicks();
        }

        // Some quando sai inteiro da tela
        if (tirosInimigos[k].rect.x + tirosInimigos[k].rect.w < 0 ||
            tirosInimigos[k].rect.x > larguraTela ||
            tirosInimigos[k].rect.y + tirosInimigos[k].rect.h < 0 ||
            tirosInimigos[k].rect.y > alturaTela)
        {
            tirosInimigos[k].ativo = false;
        }
    }
}

void Jogo::AtualizarInimigos() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_INIMIGOS; i++)
        {
            if (inimigos[i].morrendo)
            {
                if (SDL_GetTicks() - inimigos[i].flashInicio >= (Uint64)DadosDoInimigo(inimigos[i].tipo).msMorte) {
                    SDL_FRect posicaoPai = inimigos[i].rect;
                    bool eraDivisor = (inimigos[i].tipo == INIMIGO_DIVISOR);

                    // Libera a vaga ANTES de criar os filhotes (um deles pode ocupar a vaga do pai)
                    inimigos[i].morrendo = false;
                    inimigos[i].ativo = false;

                    if (eraDivisor)
                        DividirDivisor(posicaoPai);
                }
                continue;
            }

            if (inimigos[i].ativo)
            {
                // O Pirulito tem movimento e animacao proprios
                if (inimigos[i].tipo == INIMIGO_PIRULITO)
                {
                    AtualizarPirulito(inimigos[i]);
                    continue;
                }

                if (SDL_GetTicks() - inimigos[i].ultimoFrame > (Uint64)DadosDoInimigo(inimigos[i].tipo).msPorFrame) {
                    inimigos[i].frame++;

                    if (inimigos[i].frame >= DadosDoInimigo(inimigos[i].tipo).framesAndar) {
                        inimigos[i].frame = 0;
                    }

                    inimigos[i].ultimoFrame = SDL_GetTicks();
                }

                inimigos[i].rect.x -= inimigos[i].speed;

                // Caramelo: o y acompanha a senoide (sempre dentro da tela) e o sprite inclina
                if (inimigos[i].tipo == INIMIGO_CARAMELO)
                {
                    inimigos[i].fase += AvancoFaseCaramelo(inimigos[i].speed, escalaJogo);

                    inimigos[i].rect.y = CarameloY(inimigos[i].yBase, CARAMELO_AMPLITUDE * escalaJogo,
                        inimigos[i].fase, (float)alturaTela, inimigos[i].rect.h);
                    inimigos[i].angulo = CarameloAngulo(inimigos[i].fase);
                }

                if (inimigos[i].rect.x + inimigos[i].rect.w < 0)
                {
                    inimigos[i].ativo = false;
                }
            }
        }
    }
}

void Jogo::AtualizarPowerups() {
    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPS; i++)
        {
            if (!vidasPowerup[i].ativo)
                continue;

            vidasPowerup[i].rect.x -= vidasPowerup[i].speed;

            if (vidasPowerup[i].rect.x +
                vidasPowerup[i].rect.w < 0)
            {
                vidasPowerup[i].ativo = false;
            }
        }
    }

    if (estado == JOGAR) {
        for (int i = 0; i < MAX_POWERUPSATQ; i++)
        {
            if (!ataquePowerup[i].ativo)
                continue;

            ataquePowerup[i].rect.x -= ataquePowerup[i].speed;

            if (ataquePowerup[i].rect.x +
                ataquePowerup[i].rect.w < 0)
            {
                ataquePowerup[i].ativo = false;
            }
        }
    }
}

void Jogo::AtualizarBoss() {
    if (estado == JOGAR) {
        // Faiscas do escudo: somem quando a animacao termina
        for (int i = 0; i < MAX_FAISCAS_ESCUDO; i++)
        {
            if (faiscas[i].ativa && SDL_GetTicks() - faiscas[i].inicio >= FAISCA_MS_FRAME * FAISCA_FRAMES)
                faiscas[i].ativa = false;
        }

        if (boss.ativo)
        {
            // Touro: andando -> avisando -> investindo -> andando (uma vez por boss)
            if (boss.tipo == BOSS_TOURO)
            {
                float progresso = ProgressoBoss(boss.rect.x, (float)larguraTela, base.x + base.w);
                EstadoBoss novo = TransicaoTouro(boss.estado, boss.investiu, progresso,
                    SDL_GetTicks(), boss.estadoInicio);

                if (novo != boss.estado)
                {
                    if (novo == BOSS_AVISANDO)
                        boss.investiu = true;

                    boss.estado = novo;
                    boss.estadoInicio = SDL_GetTicks();
                    boss.frame = 0;
                    boss.ultimoFrame = SDL_GetTicks();
                }
            }

            // Brownie: vulneravel -> escudo entrando -> escudo -> escudo saindo -> vulneravel
            if (boss.tipo == BOSS_BROWNIE)
            {
                EstadoBoss novo = TransicaoBrownie(boss.estado, SDL_GetTicks(), boss.estadoInicio);

                if (novo != boss.estado)
                {
                    boss.estado = novo;
                    boss.estadoInicio = SDL_GetTicks();
                    boss.frame = 0;
                    boss.ultimoFrame = SDL_GetTicks();
                }
            }

            // Churros: andando -> invocando -> andando; os minions nascem no pico do brilho
            if (boss.tipo == BOSS_CHURROS)
            {
                Uint64 agora = SDL_GetTicks();
                EstadoBoss novo = TransicaoChurros(boss.estado, agora, boss.estadoInicio, boss.proximaInvocacao);

                if (novo != boss.estado)
                {
                    boss.estado = novo;
                    boss.estadoInicio = agora;
                    boss.frame = 0;
                    boss.ultimoFrame = agora;
                    boss.invocou = false;

                    if (novo == BOSS_ANDANDO)
                        boss.proximaInvocacao = agora + CHURROS_MS_INTERVALO;
                }

                if (boss.estado == BOSS_INVOCANDO && !boss.invocou && InvocacaoPronta(agora, boss.estadoInicio))
                {
                    boss.invocou = true;
                    InvocarMinionsDoBoss();
                }
            }

            // Glacê Pegajoso: enquanto a lentidão durar, o boss anda mais devagar
            float fatorBoss = FatorVelocidadeBoss(boss.tipo, boss.estado);

            if (SDL_GetTicks() < boss.lentidaoFim)
                fatorBoss *= (100 - boss.lentidaoPercent) / 100.0f;

            boss.rect.x -= boss.speed * fatorBoss;

            const AnimBoss& anim = TABELA_ANIM_BOSS[AnimacaoDoBoss(boss.tipo, boss.estado)];

            if (SDL_GetTicks() - boss.ultimoFrame > (Uint64)anim.msFrame)
            {
                boss.frame++;

                if (boss.frame >= anim.frames)
                    boss.frame = AnimacaoBossEmLoop(boss.estado) ? 0 : anim.frames - 1;   // transicoes seguram o ultimo frame

                boss.ultimoFrame = SDL_GetTicks();
            }
        }
    }
}

void Jogo::InvocarMinionsDoBoss() {

    contadores.invocacoes++;
    // Excecao deliberada a regra "sem spawn com boss vivo": so o boss invoca.
    // Respeita so MAX_INIMIGOS (SpawnarInimigo devolve false sem vaga) e ignora inimigosAtivos.
    for (int k = 0; k < CHURROS_QTD_INVOCACAO; k++)
    {
        TipoInimigo tipo = SortearTipoInvocado();
        float lado = tamanhoInimigo * DadosDoInimigo(tipo).fatorTamanho;

        PosicaoInvocado p = PosInvocado(k, boss.rect.x, boss.rect.y, boss.rect.w, boss.rect.h,
            lado, lado, base.x + base.w, (float)larguraTela, (float)alturaTela, escalaJogo);

        SpawnarInimigo(tipo, p.x, p.y);
    }
}

void Jogo::AtualizarObstaculos()
{
    if (estado == JOGAR) {

        for (int i = 0; i < MAX_OBSTACULOS; i++)
        {
            if (!obstaculos[i].ativo)
                continue;

            switch (obstaculos[i].estado)
            {
            case OBSTACULO_SOMBRA:

                if (SDL_GetTicks() - obstaculos[i].inicioEstado >= 1000)
                {
                    obstaculos[i].estado = OBSTACULO_CAINDO;
                    obstaculos[i].inicioEstado = SDL_GetTicks();
                }

                break;

            case OBSTACULO_CAINDO:

                obstaculos[i].rect.y += obstaculos[i].velocidadeY;

                // Animação
                if (SDL_GetTicks() - obstaculos[i].ultimoFrame > 120)
                {
                    if (obstaculos[i].frame < 2)
                        obstaculos[i].frame++;

                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                }

                // Colisão com o player
                if (SDL_HasRectIntersectionFloat(&player, &obstaculos[i].rect))
                {

                    if (player.x + player.w / 2 <
                        obstaculos[i].rect.x + obstaculos[i].rect.w / 2)
                        player.x = obstaculos[i].rect.x - player.w;
                    else
                        player.x = obstaculos[i].rect.x + obstaculos[i].rect.w;

                    stun = true;
                    stunStart = SDL_GetTicks();
                    stunDuracaoAtual = STUN_DURACAO;
                    danoDuracaoAtual = DURACAO_DANO;

                    dano = true;
                    danoStart = SDL_GetTicks();

                    if (olhandoDireita)
                        framePlayer = 13;
                    else
                        framePlayer = 27;

                    obstaculos[i].rect.y = alturaChao;

                    obstaculos[i].estado = OBSTACULO_CHAO;

                    obstaculos[i].frame = 3;
                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                    obstaculos[i].inicioEstado = SDL_GetTicks();
                }

                // Tocou no chão
                if (obstaculos[i].rect.y >= alturaChao)
                {
                    obstaculos[i].rect.y = alturaChao;

                    obstaculos[i].estado = OBSTACULO_CHAO;

                    obstaculos[i].frame = 3;
                    obstaculos[i].ultimoFrame = SDL_GetTicks();
                    obstaculos[i].inicioEstado = SDL_GetTicks();
                }

                break;

            case OBSTACULO_CHAO:
            {
                Uint64 tempoNoChao = SDL_GetTicks() - obstaculos[i].inicioEstado;

                if (tempoNoChao < 8000)
                {
                    obstaculos[i].frame = 4; // parado normal
                }
                else if (tempoNoChao < 8700)
                {
                    obstaculos[i].frame = 3; // frame quebrando
                }
                else
                {
                    obstaculos[i].ativo = false;
                    TocarEfeito(somQuebra);
                    continue;
                }
                break;
            }

            case OBSTACULO_QUEBRANDO:

                obstaculos[i].frame = 3;

                if (SDL_GetTicks() - obstaculos[i].inicioEstado >= 300)
                {
                    obstaculos[i].ativo = false;
                    TocarEfeito(somQuebra);
                }

                break;
            }
        }
    }
}