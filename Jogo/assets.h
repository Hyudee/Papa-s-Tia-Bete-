#pragma once

#include "sdl_raii.h"
#include "inimigos.h"

// Texturas do jogo (RAII)

struct Assets
{
    SdlPtr<SDL_Texture> background;
    SdlPtr<SDL_Texture> backgroundNome;
    SdlPtr<SDL_Texture> menu;
    SdlPtr<SDL_Texture> logo;
    SdlPtr<SDL_Texture> jogar;
    SdlPtr<SDL_Texture> creditos;
    SdlPtr<SDL_Texture> sair;
    SdlPtr<SDL_Texture> texturaInimigo;
    SdlPtr<SDL_Texture> texturasInimigo[TOTAL_TIPOS_INIMIGO];   // arte de cada tipo; [0] nao e usado (o comum e texturaInimigo)
    SdlPtr<SDL_Texture> texturaDivisaoMarshmallow;   // animacao de divisao do Divisor (3 quadros de 64x64)
    SdlPtr<SDL_Texture> texturaPlayer;
    SdlPtr<SDL_Texture> texturaPlayerSujo;
    SdlPtr<SDL_Texture> projetil;
    SdlPtr<SDL_Texture> gameover;
    SdlPtr<SDL_Texture> coracao;
    SdlPtr<SDL_Texture> powerUpAtq;
    SdlPtr<SDL_Texture> powerUpVida;
    SdlPtr<SDL_Texture> texturaTrofeu;
    SdlPtr<SDL_Texture> texturaEngrenagem;
    SdlPtr<SDL_Texture> texturaControles;
    SdlPtr<SDL_Texture> texturaAjuda;
    SdlPtr<SDL_Texture> texturaBoss;
    SdlPtr<SDL_Texture> texturaObstaculo;
    SdlPtr<SDL_Texture> texturaSombra;
    SdlPtr<SDL_Texture> baseTexture;
    SdlPtr<SDL_Texture> upgradeImgDano;
    SdlPtr<SDL_Texture> upgradeImgVida;
    SdlPtr<SDL_Texture> upgradeImgBalas;
    SdlPtr<SDL_Texture> upgradeImgDuplo;
    SdlPtr<SDL_Texture> upgradeImgCacador;
    SdlPtr<SDL_Texture> upgradeImgCadencia;
    SdlPtr<SDL_Texture> upgradeImgBalaPesada;
    SdlPtr<SDL_Texture> upgradeImgPernas;
    SdlPtr<SDL_Texture> upgradeImgDoceSorte;
    SdlPtr<SDL_Texture> upgradeImgGlace;
    SdlPtr<SDL_Texture> upgradeImgPerfurante;
    SdlPtr<SDL_Texture> upgradeImgBarreira;
    SdlPtr<SDL_Texture> upgradeImgExplosao;
    SdlPtr<SDL_Texture> upgradeImgFuria;
    SdlPtr<SDL_Texture> upgradeImgPacto;

    // Arte opcional da tela de cartas: se o arquivo não existir, o jogo desenha formas simples no lugar
    SdlPtr<SDL_Texture> molduraCarta[3];   // comum, rara e épica (carta_comum.png, carta_rara.png, carta_epica.png)
    SdlPtr<SDL_Texture> brilhoCarta;       // carta_brilho.png: 8 quadros lado a lado
    SdlPtr<SDL_Texture> burstCarta;        // carta_burst.png: 6 quadros lado a lado

    // Gerada em código (sem arquivo): retângulo arredondado branco, para tingir com ColorMod
    SdlPtr<SDL_Texture> pilulaDestaque;

    void Carregar(SDL_Renderer * renderer);
};