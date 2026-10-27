#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include "config.h"
#include "tipos.h"
#include "escala.h"
#include "sdl_raii.h"
#include "jogo.h"

// Tela de escolha de cartas (depois de cada boss).
// A lógica de tempo e fases está em anim_cartas.h/.cpp; aqui ficam o desenho e a ligação com o Jogo.
//
// Cada carta é desenhada UMA vez numa textura (com o texto TTF já dentro) ao abrir a tela.
// Depois, a cada quadro, só se desenha essa textura com escala, rotação e opacidade.

// Cor da moldura de cada raridade
static SDL_Color CorDaRaridade(Raridade raridade)
{
    switch (raridade)
    {
    case RARIDADE_RARA:  return SDL_Color{ 70, 150, 240, 255 };
    case RARIDADE_EPICA: return SDL_Color{ 190, 90, 240, 255 };
    default:             return SDL_Color{ 140, 140, 165, 255 };
    }
}

// Texto em branco numa textura (a cor é aplicada na hora de desenhar com ColorMod)
static SdlPtr<SDL_Texture> TextoEmTextura(SDL_Renderer* renderer, TTF_Font* fonte, const char* texto)
{
    SdlPtr<SDL_Surface> surface(TTF_RenderText_Blended(fonte, texto, 0, SDL_Color{ 255, 255, 255, 255 }));

    if (!surface)
        return SdlPtr<SDL_Texture>();

    return SdlPtr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, surface.get()));
}

// Desenha um texto centrado em centroX, reduzindo se passar de larguraMaxima
static void DesenharTextoNaCarta(SDL_Renderer* renderer, SDL_Texture* textura,
    float centroX, float y, float larguraMaxima, SDL_Color cor)
{
    if (!textura)
        return;

    float w, h;
    SDL_GetTextureSize(textura, &w, &h);

    if (w > larguraMaxima)
    {
        float fator = larguraMaxima / w;
        w *= fator;
        h *= fator;
    }

    SDL_FRect destino = { centroX - w / 2.0f, y, w, h };

    SDL_SetTextureColorMod(textura, cor.r, cor.g, cor.b);
    SDL_RenderTexture(renderer, textura, NULL, &destino);
}

// Desenha uma carta (normal ou destacada) dentro da textura 'alvo'
void Jogo::DesenharCartaNaTextura(SDL_Texture* alvo, IdCarta id, bool destacada)
{
    const CartaDef& carta = DadosDaCarta(id);

    float w, h;
    SDL_GetTextureSize(alvo, &w, &h);

    SDL_BlendMode modoAnterior;
    SDL_GetRenderDrawBlendMode(renderer, &modoAnterior);

    SDL_SetRenderTarget(renderer, alvo);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_Color fundo = destacada ? SDL_Color{ 255, 230, 160, 255 } : SDL_Color{ 40, 35, 50, 255 };
    SDL_Color corRaridade = CorDaRaridade(carta.raridade);
    SDL_Color corInterna = destacada
        ? SDL_Color{ 220, 160, 20, 255 }
    : SDL_Color{ (Uint8)(corRaridade.r * 6 / 10), (Uint8)(corRaridade.g * 6 / 10), (Uint8)(corRaridade.b * 6 / 10), 255 };

    // Fundo (com moldura de arte, recua 3% para o canto arredondado ficar transparente)
    SDL_FRect cartaRect = { 0.0f, 0.0f, w, h };
    float recuoFundo = assets.molduraCarta[carta.raridade] ? w * 0.03f : 0.0f;
    SDL_FRect fundoRect = { recuoFundo, recuoFundo, w - 2.0f * recuoFundo, h - 2.0f * recuoFundo };
    SDL_SetRenderDrawColor(renderer, fundo.r, fundo.g, fundo.b, 255);
    SDL_RenderFillRect(renderer, &fundoRect);

    // Moldura da raridade: se existir a arte, ela é desenhada por cima no fim;
    // senão, borda externa na cor da raridade e uma linha interna
    SDL_Texture* molduraArte = assets.molduraCarta[carta.raridade].get();

    if (!molduraArte)
    {
        float espessura = SDL_max(2.0f, h * 0.008f);

        SDL_SetRenderDrawColor(renderer, corRaridade.r, corRaridade.g, corRaridade.b, 255);
        for (float k = 0.0f; k < espessura; k += 1.0f)
        {
            SDL_FRect borda = { k, k, w - 2.0f * k, h - 2.0f * k };
            SDL_RenderRect(renderer, &borda);
        }

        SDL_SetRenderDrawColor(renderer, corInterna.r, corInterna.g, corInterna.b, 255);
        float recuo = espessura + 3.0f;
        SDL_FRect bordaInterna = { recuo, recuo, w - 2.0f * recuo, h - 2.0f * recuo };
        SDL_RenderRect(renderer, &bordaInterna);
    }

    // Imagem
    const float alturaImagem = h * 0.42f;
    SDL_Texture* imagem = ImagemDaCarta(id);

    if (imagem)
    {
        float margem = w * 0.08f;
        SDL_FRect imagemRect = { margem, h * 0.10f, w - margem * 2.0f, alturaImagem };
        SDL_RenderTexture(renderer, imagem, NULL, &imagemRect);
    }

    // Nome e descrição
    SDL_Color corNome = destacada ? SDL_Color{ 20, 20, 20, 255 } : SDL_Color{ 255, 255, 255, 255 };
    SDL_Color corDescricao = destacada ? SDL_Color{ 60, 40, 10, 255 } : SDL_Color{ 180, 160, 200, 255 };

    SdlPtr<SDL_Texture> nome = TextoEmTextura(renderer, fontHud, carta.nome);
    SdlPtr<SDL_Texture> descricao = TextoEmTextura(renderer, fontHud, carta.descricao);

    DesenharTextoNaCarta(renderer, nome.get(), w / 2.0f,
        h * 0.10f + alturaImagem + h * 0.04f, w - 8.0f, corNome);

    DesenharTextoNaCarta(renderer, descricao.get(), w / 2.0f,
        h * 0.10f + alturaImagem + h * 0.14f, w * 0.72f, corDescricao);

    if (molduraArte)
        SDL_RenderTexture(renderer, molduraArte, NULL, &cartaRect);

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawBlendMode(renderer, modoAnterior);
}

// Esvazia o cache se a resolucao mudou desde que ele foi feito
void Jogo::ValidarCacheCartas()
{
    if (larguraCacheCartas == larguraTela && alturaCacheCartas == alturaTela)
        return;

    for (int i = 0; i < NUM_CARTAS; i++)
    {
        cacheCartaNormal[i].reset();
        cacheCartaDestacada[i].reset();
    }

    larguraCacheCartas = larguraTela;
    alturaCacheCartas = alturaTela;
}

// Cria as texturas (normal e destacada) de uma carta, se ainda nao existirem
void Jogo::GarantirTexturaCarta(IdCarta id)
{
    LayoutCartas medida = CalcularLayoutCartas(CARTAS_POR_OFERTA, larguraTela, alturaTela);
    int larguraTextura = (int)SDL_ceilf(medida.largura);
    int alturaTextura = (int)SDL_ceilf(medida.altura);

    for (int variante = 0; variante < 2; variante++)
    {
        SdlPtr<SDL_Texture>& destino = (variante == 0) ? cacheCartaNormal[id] : cacheCartaDestacada[id];

        if (destino)
            continue;

        destino.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET, larguraTextura, alturaTextura));

        if (!destino)
        {
            SDL_Log("Falha ao criar a textura da carta: %s", SDL_GetError());
            continue;
        }

        SDL_SetTextureBlendMode(destino.get(), SDL_BLENDMODE_BLEND);
        DesenharCartaNaTextura(destino.get(), id, variante == 1);
    }
}

// Cria o cache de todas as cartas (chamar fora do desenho do quadro, ex.: ao iniciar a partida)
void Jogo::PreAquecerCartas()
{
    ValidarCacheCartas();

    for (int i = 0; i < NUM_CARTAS; i++)
        GarantirTexturaCarta((IdCarta)i);
}

// Prepara a tela: layout, texturas das cartas oferecidas (do cache) e textos fixos
void Jogo::ConstruirTexturasCartas()
{
    layoutCartas = CalcularLayoutCartas(totalOfertaCartas, larguraTela, alturaTela);

    ValidarCacheCartas();

    for (int i = 0; i < layoutCartas.total; i++)
        GarantirTexturaCarta(ofertaCartas[i]);   // no-op quando ja esta no cache

    // Textos fixos da tela
    textoTituloCartas = TextoEmTextura(renderer, font, "ESCOLHA UMA MELHORIA");

    char dica[96];

    if (RerollsDisponiveis() > 0)
        sprintf_s(dica, sizeof(dica), "A / D - Navegar      ENTER - Confirmar      R - Trocar cartas (%d)",
            RerollsDisponiveis());
    else
        sprintf_s(dica, sizeof(dica), "A / D - Navegar      ENTER - Confirmar");

    textoDicaCartas = TextoEmTextura(renderer, fontHud, dica);

    cartasProntas = true;
    larguraCartasConstruida = larguraTela;
    alturaCartasConstruida = alturaTela;
}

void Jogo::EncerrarTelaCartas()
{
    textoTituloCartas.reset();
    textoDicaCartas.reset();
    cartasProntas = false;
}

// Chamado quando a oferta muda (boss morreu ou cartas trocadas): recomeça a animação
void Jogo::IniciarTelaCartas()
{
    IniciarAnimCartas(animCartas, SDL_GetTicks(), totalOfertaCartas);
    cartasProntas = false;   // as texturas são refeitas no começo do próximo quadro
}

// Uma vez por quadro, ANTES de desenhar (criar texturas no meio do desenho do quadro é arriscado)
void Jogo::AtualizarUpgrade()
{
    if (estado != UPGRADE)
        return;

    if (!cartasProntas ||
        larguraCartasConstruida != larguraTela ||
        alturaCartasConstruida != alturaTela)
    {
        ConstruirTexturasCartas();
    }

    Uint64 agora = SDL_GetTicks();

    AtualizarAnimCartas(animCartas, agora, opcaoUpgrade);

    // Confirmação terminou: volta ao jogo, empurrando os timers pelo tempo em que ficou parado
    if (AnimacaoCartasTerminou(animCartas, agora))
    {
        EncerrarTelaCartas();
        DeslocarTimers(agora - inicioPausa);
        estado = JOGAR;
    }
}

// Troca a carta selecionada (teclado ou mouse)
void Jogo::SelecionarCarta(int indice)
{
    if (indice < 0 || indice >= totalOfertaCartas || indice == opcaoUpgrade)
        return;

    opcaoUpgrade = indice;
    TocarEfeito(somHoverBotao);
}

// Confirma a carta: aplica o efeito já e começa a animação; o jogo só volta no fim dela
void Jogo::ConfirmarCarta(int indice)
{
    if (indice < 0 || indice >= totalOfertaCartas)
        return;

    IdCarta id = ofertaCartas[indice];
    bool tremer = DadosDaCarta(id).raridade != RARIDADE_COMUM;

    if (!ConfirmarAnimCartas(animCartas, SDL_GetTicks(), indice, tremer))
        return;   // input travado ou já confirmou

    opcaoUpgrade = indice;
    TocarEfeito(somConfirm);
    EscolherCarta(id);
}

// Brilho atrás de uma carta Rara ou Épica.
// Épica com arte: quadro do spritesheet (o quadro é 25% maior que a carta). Sem arte: contornos que pulsam.
void Jogo::DesenharBrilhoCarta(const SDL_FRect& destino, Raridade raridade, float alfa, Uint64 agora)
{
    if (raridade == RARIDADE_COMUM || alfa <= 0.0f)
        return;

    SDL_Texture* arte = assets.brilhoCarta.get();

    if (raridade == RARIDADE_EPICA && arte)
    {
        float larguraArte, alturaArte;
        SDL_GetTextureSize(arte, &larguraArte, &alturaArte);

        float larguraQuadro = larguraArte / (float)CARTAS_BRILHO_FRAMES;
        SDL_FRect origem = { QuadroBrilhoCartas(animCartas, agora) * larguraQuadro, 0.0f, larguraQuadro, alturaArte };

        SDL_FRect moldura = {
            destino.x - destino.w * 0.125f,
            destino.y - destino.h * 0.125f,
            destino.w * 1.25f,
            destino.h * 1.25f
        };

        SDL_SetTextureAlphaMod(arte, (Uint8)alfa);
        SDL_RenderTexture(renderer, arte, &origem, &moldura);
        return;
    }

    SDL_Color cor = CorDaRaridade(raridade);
    float pulso = 0.5f + 0.5f * SDL_sinf((float)agora * 0.004f);
    float forca = (raridade == RARIDADE_EPICA) ? 1.0f : 0.6f;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

    for (int k = 1; k <= 4; k++)
    {
        float margem = k * 3.0f * escalaJogo;
        SDL_FRect contorno = {
            destino.x - margem,
            destino.y - margem,
            destino.w + 2.0f * margem,
            destino.h + 2.0f * margem
        };

        float intensidade = alfa * forca * (0.25f + 0.35f * pulso) * (1.0f - (k - 1) * 0.22f);

        SDL_SetRenderDrawColor(renderer, cor.r, cor.g, cor.b, (Uint8)intensidade);
        SDL_RenderRect(renderer, &contorno);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

// Burst da confirmação, no centro da carta escolhida.
// Com arte: quadro do spritesheet. Sem arte: raios que se afastam do centro e somem.
void Jogo::DesenharBurstCarta(const SDL_FRect& destino, Raridade raridade, int quadro)
{
    float centroX = destino.x + destino.w / 2.0f;
    float centroY = destino.y + destino.h / 2.0f;

    SDL_Texture* arte = assets.burstCarta.get();

    if (arte)
    {
        float larguraArte, alturaArte;
        SDL_GetTextureSize(arte, &larguraArte, &alturaArte);

        float larguraQuadro = larguraArte / (float)CARTAS_BURST_FRAMES;
        SDL_FRect origem = { quadro * larguraQuadro, 0.0f, larguraQuadro, alturaArte };

        float lado = destino.w * 1.8f;
        SDL_FRect moldura = { centroX - lado / 2.0f, centroY - lado / 2.0f, lado, lado };

        SDL_SetTextureAlphaMod(arte, 255);
        SDL_RenderTexture(renderer, arte, &origem, &moldura);
        return;
    }

    float progresso = (quadro + 0.5f) / (float)CARTAS_BURST_FRAMES;
    SDL_Color cor = (raridade == RARIDADE_COMUM) ? SDL_Color{ 255, 230, 160, 255 } : CorDaRaridade(raridade);

    float raioInicial = destino.w * (0.3f + 0.9f * EaseOutCubic(progresso));
    float raioFinal = raioInicial + destino.w * 0.35f * (1.0f - progresso);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    SDL_SetRenderDrawColor(renderer, cor.r, cor.g, cor.b, (Uint8)(255.0f * (1.0f - progresso)));

    for (int k = 0; k < 12; k++)
    {
        float angulo = k * 0.5235988f;   // 30 graus
        float c = SDL_cosf(angulo);
        float s = SDL_sinf(angulo);

        SDL_RenderLine(renderer, centroX + c * raioInicial, centroY + s * raioInicial,
            centroX + c * raioFinal, centroY + s * raioFinal);

        SDL_RenderLine(renderer, centroX + c * raioInicial + 1.0f, centroY + s * raioInicial,
            centroX + c * raioFinal + 1.0f, centroY + s * raioFinal);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

bool Jogo::RenderizarUpgrade()
{
    if (estado != UPGRADE)
        return false;

    Uint64 agora = SDL_GetTicks();
    float alfaFundo = AlfaFundoCartas(animCartas, agora);
    Uint8 alfaTexto = (Uint8)(255.0f * alfaFundo / CARTAS_FUNDO_ALFA);

    // Fundo escuro
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)alfaFundo);
    SDL_FRect telaInteira = { 0.0f, 0.0f, (float)larguraTela, (float)alturaTela };
    SDL_RenderFillRect(renderer, &telaInteira);

    // Título
    if (textoTituloCartas)
    {
        float w, h;
        SDL_GetTextureSize(textoTituloCartas.get(), &w, &h);

        SDL_FRect destino = { larguraTela / 2.0f - w / 2.0f, alturaTela * 0.18f - alturaTela * 0.13f, w, h };

        SDL_SetTextureAlphaMod(textoTituloCartas.get(), alfaTexto);
        SDL_RenderTexture(renderer, textoTituloCartas.get(), NULL, &destino);
    }

    // Cartas: a selecionada por último, para ficar por cima das outras
    SDL_FRect destinoEscolhida = { 0.0f, 0.0f, 0.0f, 0.0f };
    bool temEscolhida = false;

    auto desenharCarta = [&](int i)
        {
            SDL_Texture* normal = cacheCartaNormal[ofertaCartas[i]].get();
            SDL_Texture* destacada = cacheCartaDestacada[ofertaCartas[i]].get();

            if (!normal || !destacada)
                return;

            PoseCarta pose = CalcularPoseCarta(animCartas, layoutCartas, i, agora, escalaJogo);
            const SDL_FRect& base = layoutCartas.carta[i];

            float w = base.w * pose.escala;
            float h = base.h * pose.escala;

            SDL_FRect destino = {
                base.x + base.w / 2.0f + pose.dx - w / 2.0f,
                base.y + base.h / 2.0f + pose.dy - h / 2.0f,
                w,
                h
            };

            double angulo = (double)pose.angulo;
            float selecao = EaseOutCubic(animCartas.destaque[i]);

            // Brilho atrás da carta (Rara e Épica)
            DesenharBrilhoCarta(destino, DadosDaCarta(ofertaCartas[i]).raridade,
                255.0f * AlfaBrilhoCartas(animCartas, agora) * pose.alfa / 255.0f, agora);

            // Guarda onde a escolhida foi desenhada, para o burst
            if (i == animCartas.escolhida)
            {
                destinoEscolhida = destino;
                temEscolhida = true;
            }

            // Sombra: a própria carta pintada de preto, deslocada
            SDL_FRect sombra = destino;
            sombra.x += 6.0f * escalaJogo;
            sombra.y += 6.0f * escalaJogo;

            SDL_SetTextureColorMod(normal, 0, 0, 0);
            SDL_SetTextureAlphaMod(normal, (Uint8)(pose.alfa * 100.0f / 255.0f));
            SDL_RenderTextureRotated(renderer, normal, NULL, &sombra, angulo, NULL, SDL_FLIP_NONE);
            SDL_SetTextureColorMod(normal, 255, 255, 255);

            // A carta normal por baixo; a destacada aparece por cima conforme a seleção
            SDL_SetTextureAlphaMod(normal, (Uint8)pose.alfa);
            SDL_RenderTextureRotated(renderer, normal, NULL, &destino, angulo, NULL, SDL_FLIP_NONE);

            if (selecao > 0.0f)
            {
                SDL_SetTextureAlphaMod(destacada, (Uint8)(pose.alfa * selecao));
                SDL_RenderTextureRotated(renderer, destacada, NULL, &destino, angulo, NULL, SDL_FLIP_NONE);
            }

            // Flash branco aditivo (confirmação)
            if (pose.flash > 0.0f)
            {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, (Uint8)(200.0f * pose.flash));
                SDL_RenderFillRect(renderer, &destino);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            }
        };

    for (int i = 0; i < layoutCartas.total; i++)
    {
        if (i != opcaoUpgrade)
            desenharCarta(i);
    }

    if (opcaoUpgrade >= 0 && opcaoUpgrade < layoutCartas.total)
        desenharCarta(opcaoUpgrade);


    // Burst da confirmação, por cima da carta escolhida
    int quadroBurst = QuadroBurstCartas(animCartas, agora);

    if (temEscolhida && quadroBurst >= 0)
        DesenharBurstCarta(destinoEscolhida, DadosDaCarta(ofertaCartas[animCartas.escolhida]).raridade, quadroBurst);

    // Dica de teclas
    if (textoDicaCartas)
    {
        float w, h;
        SDL_GetTextureSize(textoDicaCartas.get(), &w, &h);

        SDL_FRect destino = { larguraTela / 2.0f - w / 2.0f, (float)alturaTela - h - 18.0f, w, h };

        SDL_SetTextureAlphaMod(textoDicaCartas.get(), alfaTexto);
        SDL_RenderTexture(renderer, textoDicaCartas.get(), NULL, &destino);
    }

    SDL_RenderPresent(renderer);
    EsperarProximoFrame();

    return true;
}