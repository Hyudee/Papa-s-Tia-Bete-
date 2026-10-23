#include "assets.h"

#include <SDL3_image/SDL_image.h>

void Assets::Carregar(SDL_Renderer* renderer)
{
    auto carregar = [renderer](SdlPtr<SDL_Texture>& destino, const char* caminho)
        {
            destino.reset(IMG_LoadTexture(renderer, caminho));

            if (!destino)
                SDL_Log("Falha ao carregar a textura %s: %s", caminho, SDL_GetError());
        };

    carregar(background, "assets/cozinha.png");
    carregar(backgroundNome, "assets/background.png");
    carregar(menu, "assets/menu.png");
    carregar(logo, "assets/logo.png");
    carregar(jogar, "assets/jogar.png");
    carregar(creditos, "assets/creditos.png");
    carregar(sair, "assets/sair.png");
    carregar(texturaInimigo, "assets/Acucar.png");
    carregar(texturaPlayer, "assets/player.png");
    carregar(texturaPlayerSujo, "assets/playerSujo.png");
    carregar(projetil, "assets/tiro.png");
    carregar(gameover, "assets/gameover.png");
    carregar(coracao, "assets/coracao.png");
    carregar(powerUpAtq, "assets/powerupatq.png");
    carregar(powerUpVida, "assets/powerupvida.png");
    carregar(texturaTrofeu, "assets/rankingaba.png");
    carregar(texturaEngrenagem, "assets/configuracaoaba.png");
    carregar(texturaControles, "assets/controlesaba.png");
    carregar(texturaAjuda, "assets/interrogacaoaba.png");
    carregar(texturaBoss, "assets/boss.png");
    carregar(texturaObstaculo, "assets/obstaculo.png");
    carregar(texturaSombra, "assets/sombra.png");
    carregar(baseTexture, "assets/base.png");
    carregar(upgradeImgDano, "assets/mais_dano.png");
    carregar(upgradeImgVida, "assets/mais_vida.png");
    carregar(upgradeImgBalas, "assets/mais_balas.png");
    carregar(upgradeImgDuplo, "assets/tiro_duplo.png");
    carregar(upgradeImgCacador, "assets/matador_de_boss.png");
}