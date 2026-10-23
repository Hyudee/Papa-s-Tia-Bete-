#pragma once

#include "sdl_raii.h"

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

    void Carregar(SDL_Renderer * renderer);
};