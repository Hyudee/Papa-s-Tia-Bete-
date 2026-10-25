#pragma once

#include <SDL3/SDL.h>
#include "config.h"

// Animação da tela de escolha de cartas.
// Aqui fica só a lógica (tempos, fases, curvas, posições); o desenho está em jogo_upgrade.cpp.
//
// A tela tem relógio próprio: os instantes guardados em AnimCartas são comparados com
// SDL_GetTicks() e NÃO entram em DeslocarTimers (a tela só existe com o jogo parado).

// ---- Tempos (ms) e medidas ----
inline constexpr Uint64 CARTAS_ENTRADA_MS = 560;         // entrada de cada carta
inline constexpr Uint64 CARTAS_ESCALONA_MS = 130;         // atraso de uma carta para a seguinte
inline constexpr Uint64 CARTAS_TRAVA_MS = 700;           // sem input depois que a tela abre
inline constexpr Uint64 CARTAS_FUNDO_ENTRADA_MS = 300;   // o fundo escurece
inline constexpr float CARTAS_IDLE_MS = 1800.0f;         // período do "respirar"
inline constexpr float CARTAS_IDLE_AMPLITUDE = 0.015f;   // 1,5% de escala
inline constexpr float CARTAS_FUNDO_ALFA = 153.0f;       // 60% de 255
inline constexpr float CARTAS_SOBE_ENTRADA = 48.0f;      // px (vezes escalaJogo)
inline constexpr float CARTAS_ROTACAO_ENTRADA = 6.0f;    // graus
inline constexpr Uint64 CARTAS_HOVER_MS = 140;           // troca de carta selecionada
inline constexpr float CARTAS_HOVER_ESCALA = 0.10f;      // a selecionada cresce 10%
inline constexpr float CARTAS_HOVER_SOBE = 10.0f;        // e sobe 10 px (vezes escalaJogo)
inline constexpr float CARTAS_OUTRAS_ALFA = 160.0f;      // opacidade das não selecionadas
inline constexpr Uint64 CARTAS_CONFIRMA_MS = 220;        // confirmação da escolha
inline constexpr float CARTAS_CONFIRMA_ESCALA = 1.22f;   // tamanho final da escolhida
inline constexpr float CARTAS_CONFIRMA_CAI = 30.0f;      // as outras caem 30 px (vezes escalaJogo)
inline constexpr Uint64 CARTAS_FLASH_MS = 120;           // flash branco
inline constexpr float CARTAS_TREMOR_PX = 5.0f;          // tremor da escolhida (Rara e Épica)
inline constexpr Uint64 CARTAS_SAIDA_MS = 260;           // a escolhida voa para o HUD
inline constexpr Uint64 CARTAS_FUNDO_SAIDA_MS = 220;     // o fundo some
inline constexpr float CARTAS_SAIDA_ESCALA = 0.5f;       // tamanho com que chega ao HUD
inline constexpr float CARTAS_HUD_X = 30.0f;             // para onde ela voa (px da tela; é o canto dos corações)
inline constexpr float CARTAS_HUD_Y = 26.0f;
inline constexpr int CARTAS_BRILHO_FRAMES = 8;           // spritesheet de brilho (Épica)
inline constexpr Uint64 CARTAS_BRILHO_FRAME_MS = 70;
inline constexpr int CARTAS_BURST_FRAMES = 6;            // spritesheet do burst da confirmação
inline constexpr Uint64 CARTAS_BURST_FRAME_MS = 50;

// ---- Curvas (t de 0 a 1) ----
float Limitar01(float t);
float EaseOutCubic(float t);
float EaseInCubic(float t);
float EaseOutQuad(float t);
float EaseOutBack(float t, float c = 1.4f);

// ---- Posição das cartas na tela (sem animação) ----
struct LayoutCartas
{
    int total;
    float largura;
    float altura;
    SDL_FRect carta[CARTAS_POR_OFERTA];
};

LayoutCartas CalcularLayoutCartas(int total, int larguraTela, int alturaTela);

// Índice da carta sob o ponto (x, y), ou -1
int CartaNaPosicao(const LayoutCartas& layout, float x, float y);

// ---- Fases e relógio da tela ----
enum FaseCartas
{
    FASE_CARTAS_ENTRADA,
    FASE_CARTAS_IDLE,
    FASE_CARTAS_CONFIRMANDO,
    FASE_CARTAS_SAIDA
};

struct AnimCartas
{
    FaseCartas fase = FASE_CARTAS_IDLE;
    Uint64 inicioTela = 0;      // t0: quando a tela abriu
    Uint64 ultimoTick = 0;
    int total = 0;
    float destaque[CARTAS_POR_OFERTA] = { 0 };   // 0 a 1: quanto cada carta está "selecionada"

    // Confirmação
    Uint64 inicioFase = 0;      // quando a fase atual (confirmando) começou
    int escolhida = -1;
    bool tremer = false;        // Rara e Épica sacodem ao ser escolhidas
};

// Abre a tela (ou reabre, depois de trocar as cartas)
void IniciarAnimCartas(AnimCartas& a, Uint64 agora, int total);

// Uma vez por quadro, com a carta selecionada no momento
void AtualizarAnimCartas(AnimCartas& a, Uint64 agora, int selecionada);

// false nos primeiros CARTAS_TRAVA_MS (evita escolher sem querer com o clique de quem estava atirando)
// e também depois que uma carta foi confirmada
bool InputLiberadoCartas(const AnimCartas& a, Uint64 agora);

// Confirma a carta 'indice'. false (e nada muda) se o input está travado ou o índice é inválido.
bool ConfirmarAnimCartas(AnimCartas& a, Uint64 agora, int indice, bool tremer);

// true quando a animação de confirmação acabou e o jogo pode voltar
bool AnimacaoCartasTerminou(const AnimCartas& a, Uint64 agora);

// ---- Como cada carta está desenhada neste instante ----
struct PoseCarta
{
    float dx;       // deslocamento em relação à posição do layout (px)
    float dy;
    float escala;
    float angulo;   // graus
    float alfa;     // 0 a 255
    float flash;    // 0 a 1: quanto de branco aditivo por cima da carta
};

PoseCarta CalcularPoseCarta(const AnimCartas& a, const LayoutCartas& layout, int indice,
    Uint64 agora, float escalaJogo);

// Opacidade (0 a 255) do fundo escuro
float AlfaFundoCartas(const AnimCartas& a, Uint64 agora);

// Brilho das cartas Rara e Épica: 0 (apagado) a 1. Só aparece depois que as cartas assentam.
float AlfaBrilhoCartas(const AnimCartas& a, Uint64 agora);

// Quadro atual do spritesheet de brilho (0 a CARTAS_BRILHO_FRAMES - 1)
int QuadroBrilhoCartas(const AnimCartas& a, Uint64 agora);

// Quadro atual do burst da confirmação, ou -1 se ele não está tocando
int QuadroBurstCartas(const AnimCartas& a, Uint64 agora);
