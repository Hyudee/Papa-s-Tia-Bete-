#include <cmath>
#include "anim_cartas.h"

// ============================================================
// CURVAS
// ============================================================

float Limitar01(float t)
{
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

float EaseOutCubic(float t)
{
    float u = 1.0f - Limitar01(t);
    return 1.0f - u * u * u;
}

float EaseInCubic(float t)
{
    t = Limitar01(t);
    return t * t * t;
}

float EaseOutQuad(float t)
{
    float u = 1.0f - Limitar01(t);
    return 1.0f - u * u;
}

// Passa um pouco de 1 antes de assentar (c maior = mais exagerado)
float EaseOutBack(float t, float c)
{
    float u = Limitar01(t) - 1.0f;
    return 1.0f + (c + 1.0f) * u * u * u + c * u * u;
}

// ============================================================
// LAYOUT
// ============================================================

LayoutCartas CalcularLayoutCartas(int total, int larguraTela, int alturaTela)
{
    LayoutCartas l = {};

    if (total > CARTAS_POR_OFERTA) total = CARTAS_POR_OFERTA;
    if (total < 0) total = 0;

    l.total = total;
    l.largura = larguraTela * 0.14f;
    l.altura = alturaTela * 0.62f;

    float intervalo = larguraTela * 0.025f;
    float blocoLargura = total * l.largura + (total > 1 ? (total - 1) * intervalo : 0.0f);
    float blocoX = larguraTela / 2.0f - blocoLargura / 2.0f;
    float blocoY = alturaTela * 0.18f;

    for (int i = 0; i < total; i++)
        l.carta[i] = { blocoX + i * (l.largura + intervalo), blocoY, l.largura, l.altura };

    return l;
}

int CartaNaPosicao(const LayoutCartas& layout, float x, float y)
{
    for (int i = 0; i < layout.total; i++)
    {
        const SDL_FRect& r = layout.carta[i];

        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h)
            return i;
    }

    return -1;
}

// ============================================================
// FASES E RELÓGIO
// ============================================================

// Tempo desde 'inicio' (0 se o relógio estiver atrás, em vez de dar a volta no Uint64)
static Uint64 Decorrido(Uint64 agora, Uint64 inicio)
{
    return agora > inicio ? agora - inicio : 0;
}

// Tempo até a última carta terminar de entrar
static Uint64 DuracaoEntrada(const AnimCartas& a)
{
    Uint64 escalonamento = (a.total > 1) ? (Uint64)(a.total - 1) * CARTAS_ESCALONA_MS : 0;

    return CARTAS_ENTRADA_MS + escalonamento;
}

void IniciarAnimCartas(AnimCartas& a, Uint64 agora, int total)
{
    a = AnimCartas();

    if (total > CARTAS_POR_OFERTA) total = CARTAS_POR_OFERTA;
    if (total < 0) total = 0;

    a.fase = FASE_CARTAS_ENTRADA;
    a.inicioTela = agora;
    a.ultimoTick = agora;
    a.total = total;
}

void AtualizarAnimCartas(AnimCartas& a, Uint64 agora, int selecionada)
{
    float dt = (float)(Decorrido(agora, a.ultimoTick));

    if (dt > 50.0f)   // quadro muito longo (ou relógio estranho): não dá um salto
        dt = 50.0f;

    a.ultimoTick = agora;

    if (a.fase == FASE_CARTAS_ENTRADA && Decorrido(agora, a.inicioTela) >= DuracaoEntrada(a))
        a.fase = FASE_CARTAS_IDLE;

    // Confirmação acabou: começa a saída, sem perder o tempo que já passou
    if (a.fase == FASE_CARTAS_CONFIRMANDO && Decorrido(agora, a.inicioFase) >= CARTAS_CONFIRMA_MS)
    {
        a.fase = FASE_CARTAS_SAIDA;
        a.inicioFase += CARTAS_CONFIRMA_MS;
    }

    // A carta selecionada vai a 1 e as outras a 0, em CARTAS_HOVER_MS
    float passo = dt / (float)CARTAS_HOVER_MS;

    for (int i = 0; i < a.total; i++)
    {
        float alvo = (i == selecionada) ? 1.0f : 0.0f;

        if (a.destaque[i] < alvo)
            a.destaque[i] = std::fmin(alvo, a.destaque[i] + passo);
        else
            a.destaque[i] = std::fmax(alvo, a.destaque[i] - passo);
    }
}

bool InputLiberadoCartas(const AnimCartas& a, Uint64 agora)
{
    if (a.fase == FASE_CARTAS_CONFIRMANDO || a.fase == FASE_CARTAS_SAIDA)
        return false;

    return Decorrido(agora, a.inicioTela) >= CARTAS_TRAVA_MS;
}

bool ConfirmarAnimCartas(AnimCartas& a, Uint64 agora, int indice, bool tremer)
{
    if (!InputLiberadoCartas(a, agora))
        return false;

    if (indice < 0 || indice >= a.total)
        return false;

    a.fase = FASE_CARTAS_CONFIRMANDO;
    a.inicioFase = agora;
    a.escolhida = indice;
    a.tremer = tremer;

    return true;
}

bool AnimacaoCartasTerminou(const AnimCartas& a, Uint64 agora)
{
    return a.fase == FASE_CARTAS_SAIDA && Decorrido(agora, a.inicioFase) >= CARTAS_SAIDA_MS;
}

// ============================================================
// POSE
// ============================================================

PoseCarta CalcularPoseCarta(const AnimCartas& a, const LayoutCartas& layout, int indice,
    Uint64 agora, float escalaJogo)
{
    PoseCarta p = { 0.0f, 0.0f, 1.0f, 0.0f, 255.0f, 0.0f };

    // Entrada: cada carta começa um pouco depois da anterior
    float ms = (float)(Decorrido(agora, a.inicioTela)) - (float)indice * (float)CARTAS_ESCALONA_MS;
    float t = Limitar01(ms / (float)CARTAS_ENTRADA_MS);
    float e = EaseOutBack(t);

    float sinal = (indice % 2 == 0) ? -1.0f : 1.0f;

    p.dy = (1.0f - e) * CARTAS_SOBE_ENTRADA * escalaJogo;
    p.angulo = sinal * CARTAS_ROTACAO_ENTRADA * (1.0f - e);
    p.escala = 0.85f + 0.15f * e;
    p.alfa = 255.0f * Limitar01(e);

    // Idle: a carta "respira" (entra aos poucos junto com a carta)
    float fase = 6.2831853f * (float)(Decorrido(agora, a.inicioTela)) / CARTAS_IDLE_MS;
    p.escala *= 1.0f + CARTAS_IDLE_AMPLITUDE * std::sin(fase) * t;

    // Seleção: a carta destacada cresce e sobe; as outras ficam mais apagadas
    float h = EaseOutCubic(a.destaque[indice]);

    p.escala *= 1.0f + CARTAS_HOVER_ESCALA * h;
    p.dy -= CARTAS_HOVER_SOBE * escalaJogo * h;
    p.alfa *= (CARTAS_OUTRAS_ALFA + (255.0f - CARTAS_OUTRAS_ALFA) * h) / 255.0f;

    // Confirmação (e saída, que continua de onde a confirmação parou)
    if (a.fase == FASE_CARTAS_CONFIRMANDO || a.fase == FASE_CARTAS_SAIDA)
    {
        float desde = (a.fase == FASE_CARTAS_SAIDA) ? (float)CARTAS_CONFIRMA_MS : (float)(Decorrido(agora, a.inicioFase));
        float tc = Limitar01(desde / (float)CARTAS_CONFIRMA_MS);

        if (indice == a.escolhida)
        {
            // Cresce de 1,10 (seleção) até 1,22, com um flash branco no começo
            p.escala *= 1.0f + (CARTAS_CONFIRMA_ESCALA / (1.0f + CARTAS_HOVER_ESCALA) - 1.0f) * EaseOutQuad(tc);
            p.flash = Limitar01(1.0f - desde / (float)CARTAS_FLASH_MS);

            if (a.tremer)
            {
                float forca = CARTAS_TREMOR_PX * escalaJogo * (1.0f - tc);

                p.dx += std::sin(desde * 0.09f) * forca;
                p.dy += std::cos(desde * 0.13f) * forca * 0.6f;
            }

            // Saída: encolhe até 0,5 e voa até o HUD; some no final
            if (a.fase == FASE_CARTAS_SAIDA)
            {
                float linear = Limitar01((float)(Decorrido(agora, a.inicioFase)) / (float)CARTAS_SAIDA_MS);
                float ts = EaseInCubic(linear);

                const SDL_FRect& base = layout.carta[indice];
                float centroX = base.x + base.w / 2.0f + p.dx;
                float centroY = base.y + base.h / 2.0f + p.dy;

                p.dx += (CARTAS_HUD_X - centroX) * ts;
                p.dy += (CARTAS_HUD_Y - centroY) * ts;
                p.escala *= 1.0f + (CARTAS_SAIDA_ESCALA / CARTAS_CONFIRMA_ESCALA - 1.0f) * ts;
                p.alfa *= 1.0f - Limitar01((linear - 0.7f) / 0.3f);
            }
        }
        else
        {
            // As outras caem e somem
            float queda = EaseInCubic(tc);

            p.dy += CARTAS_CONFIRMA_CAI * escalaJogo * queda;
            p.alfa *= 1.0f - queda;
        }
    }

    return p;
}

float AlfaFundoCartas(const AnimCartas& a, Uint64 agora)
{
    // Na saída o fundo some
    if (a.fase == FASE_CARTAS_SAIDA)
    {
        float t = (float)(Decorrido(agora, a.inicioFase)) / (float)CARTAS_FUNDO_SAIDA_MS;

        return CARTAS_FUNDO_ALFA * (1.0f - Limitar01(t));
    }

    float t = (float)(Decorrido(agora, a.inicioTela)) / (float)CARTAS_FUNDO_ENTRADA_MS;

    return CARTAS_FUNDO_ALFA * EaseOutCubic(t);
}

float AlfaBrilhoCartas(const AnimCartas& a, Uint64 agora)
{
    Uint64 fimEntrada = a.inicioTela + DuracaoEntrada(a);

    if (agora < fimEntrada)
        return 0.0f;

    return Limitar01((float)(agora - fimEntrada) / 250.0f);
}

int QuadroBrilhoCartas(const AnimCartas& a, Uint64 agora)
{
    return (int)(((Decorrido(agora, a.inicioTela)) / CARTAS_BRILHO_FRAME_MS) % CARTAS_BRILHO_FRAMES);
}

int QuadroBurstCartas(const AnimCartas& a, Uint64 agora)
{
    Uint64 desde;

    if (a.fase == FASE_CARTAS_CONFIRMANDO)
        desde = Decorrido(agora, a.inicioFase);
    else if (a.fase == FASE_CARTAS_SAIDA)
        desde = CARTAS_CONFIRMA_MS + (Decorrido(agora, a.inicioFase));
    else
        return -1;

    Uint64 quadro = desde / CARTAS_BURST_FRAME_MS;

    return (quadro < (Uint64)CARTAS_BURST_FRAMES) ? (int)quadro : -1;
}