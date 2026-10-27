#include "assets.h"

#include <SDL3_image/SDL_image.h>


#include <algorithm>
#include <cmath>

// Opacidade (0 a 255) do pixel (x, y) da textura de destaque, de tam x tam pixels:
// contorno de 3 px opaco + preenchimento suave, com cantos arredondados e antisserrilhado.
static Uint8 AlfaDestaque(int x, int y, int tam)
{
    const float raio = 20.0f;           // raio dos cantos
    const float borda = 2.0f;           // espessura do contorno
    const float opacidadeFundo = 0.12f; // preenchimento

    float metade = tam / 2.0f;
    float px = x + 0.5f - metade;
    float py = y + 0.5f - metade;

    // distância com sinal até a borda do retângulo arredondado (negativa = dentro)
    float qx = std::fabs(px) - (metade - raio);
    float qy = std::fabs(py) - (metade - raio);
    float fx = std::max(qx, 0.0f);
    float fy = std::max(qy, 0.0f);
    float d = std::sqrt(fx * fx + fy * fy) + std::min(std::max(qx, qy), 0.0f) - raio;

    float anel = std::clamp(0.5f + borda / 2.0f - std::fabs(d + borda / 2.0f), 0.0f, 1.0f);
    float fundo = std::clamp(0.5f - d, 0.0f, 1.0f) * opacidadeFundo;

    return (Uint8)(std::max(anel, fundo) * 255.0f + 0.5f);
}

static SDL_Texture* CriarTexturaDestaque(SDL_Renderer* renderer)
{
    const int tam = 64;

    SdlPtr<SDL_Surface> surface(SDL_CreateSurface(tam, tam, SDL_PIXELFORMAT_RGBA32));

    if (!surface)
        return nullptr;

    for (int y = 0; y < tam; y++)
    {
        Uint32* linha = (Uint32*)((Uint8*)surface->pixels + y * surface->pitch);

        for (int x = 0; x < tam; x++)
            linha[x] = SDL_MapSurfaceRGBA(surface.get(), 255, 255, 255, AlfaDestaque(x, y, tam));
    }

    SDL_Texture* textura = SDL_CreateTextureFromSurface(renderer, surface.get());

    if (textura)
        SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);

    return textura;
}

void Assets::Carregar(SDL_Renderer* renderer)
{
    auto carregar = [renderer](SdlPtr<SDL_Texture>& destino, const char* caminho)
        {
            destino.reset(IMG_LoadTexture(renderer, caminho));

            if (!destino)
                SDL_Log("Falha ao carregar a textura %s: %s", caminho, SDL_GetError());
        };

    // Arte opcional: a falta do arquivo não é erro
    auto carregarOpcional = [renderer](SdlPtr<SDL_Texture>& destino, const char* caminho)
        {
            destino.reset(IMG_LoadTexture(renderer, caminho));

            if (!destino)
                SDL_Log("Arte opcional ausente (%s): usando formas simples no lugar", caminho);
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
    carregar(upgradeImgCadencia, "assets/cadencia.png");
    carregar(upgradeImgBalaPesada, "assets/bala_pesada.png");
    carregar(upgradeImgPernas, "assets/pernas_acucar.png");
    carregar(upgradeImgDoceSorte, "assets/doce_sorte.png");
    carregar(upgradeImgGlace, "assets/glace_pegajoso.png");
    carregar(upgradeImgPerfurante, "assets/perfurante.png");
    carregar(upgradeImgBarreira, "assets/barreira_base.png");
    carregar(upgradeImgExplosao, "assets/explosao_acucarada.png");
    carregar(upgradeImgFuria, "assets/furia_combo.png");
    carregar(upgradeImgPacto, "assets/pacto_amargo.png");
    carregarOpcional(molduraCarta[0], "assets/carta_comum.png");
    carregarOpcional(molduraCarta[1], "assets/carta_rara.png");
    carregarOpcional(molduraCarta[2], "assets/carta_epica.png");
    carregarOpcional(brilhoCarta, "assets/carta_brilho.png");
    carregarOpcional(burstCarta, "assets/carta_burst.png");

    // Arte dos minions por tipo: se faltar o arquivo, o jogo desenha o cubo de acucar com a cor provisoria da tabela
    carregarOpcional(texturasInimigo[INIMIGO_RAPADURA], "assets/minion_rapadura.png");
    carregarOpcional(texturasInimigo[INIMIGO_GRANULADO], "assets/minion_granulado.png");
    carregarOpcional(texturasInimigo[INIMIGO_CARAMELO], "assets/minion_caramelo.png");
    carregarOpcional(texturasInimigo[INIMIGO_DIVISOR], "assets/minion_marshmallow.png");
    carregarOpcional(texturasInimigo[INIMIGO_FILHOTE], "assets/minion_marshmallow_filhote.png");
    carregarOpcional(texturasInimigo[INIMIGO_PIRULITO], "assets/minion_pirulito.png");
    carregarOpcional(texturaDivisaoMarshmallow, "assets/minion_marshmallow_divisao.png");

    carregarOpcional(texturaTiroCalda, "assets/tiro_calda.png");
    carregarOpcional(texturaTiroCaldaImpacto, "assets/tiro_calda_impacto.png");
    carregarOpcional(texturaFaiscaEscudo, "assets/fx_faisca_escudo.png");
    carregarOpcional(texturaPedacoChocolate, "assets/fx_pedaco_chocolate.png");
    carregarOpcional(texturaPedacoImpacto, "assets/fx_pedaco_impacto.png");

    // Bosses: o chocolate usa texturaBoss; os demais sao opcionais
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_E1_ANDAR], "assets/boss_chocolate_e1_andar.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_E1_ARREMESSO], "assets/boss_chocolate_e1_arremesso.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_E2_ANDAR], "assets/boss_chocolate_e2_andar.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_E2_ARREMESSO], "assets/boss_chocolate_e2_arremesso.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_E3_ANDAR], "assets/boss_chocolate_e3_andar.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_E3_ARREMESSO], "assets/boss_chocolate_e3_arremesso.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_TRANSICAO_1_2], "assets/boss_chocolate_transicao_1_2.png");
    carregarOpcional(texturasBoss[ANIM_CHOCOLATE_TRANSICAO_2_3], "assets/boss_chocolate_transicao_2_3.png");
    carregarOpcional(texturasBoss[ANIM_TOURO_ANDAR], "assets/boss_touro_andar.png");
    carregarOpcional(texturasBoss[ANIM_TOURO_CARREGAR], "assets/boss_touro_carregar.png");
    carregarOpcional(texturasBoss[ANIM_TOURO_INVESTIDA], "assets/boss_touro_investida.png");
    carregarOpcional(texturasBoss[ANIM_CHURROS_ANDAR], "assets/boss_churros_andar.png");
    carregarOpcional(texturasBoss[ANIM_CHURROS_INVOCAR], "assets/boss_churros_invocar.png");
    carregarOpcional(texturasBoss[ANIM_BROWNIE_ANDAR], "assets/boss_brownie_andar.png");
    carregarOpcional(texturasBoss[ANIM_BROWNIE_TRANSICAO], "assets/boss_brownie_transicao.png");
    carregarOpcional(texturasBoss[ANIM_BROWNIE_ESCUDO], "assets/boss_brownie_escudo.png");

    pilulaDestaque.reset(CriarTexturaDestaque(renderer));

    if (!pilulaDestaque)
        SDL_Log("Falha ao criar a textura de destaque: %s", SDL_GetError());
}