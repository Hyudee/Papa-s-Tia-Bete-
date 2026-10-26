#include "texto.h"

void DesenharTextoCentrado(SDL_Renderer* renderer,
    TTF_Font* fonte,
    const char* texto,
    SDL_Color cor,
    float centroX,
    float y)
{
    SdlPtr<SDL_Surface> surface(TTF_RenderText_Blended(fonte, texto, 0, cor));
    SdlPtr<SDL_Texture> texture(SDL_CreateTextureFromSurface(renderer, surface.get()));

    SDL_FRect rect;

    SDL_GetTextureSize(texture.get(), &rect.w, &rect.h);

    rect.x = centroX - rect.w / 2;
    rect.y = y;

    SDL_RenderTexture(renderer, texture.get(), NULL, &rect);
}

void DesenharTextoEsquerda(SDL_Renderer* renderer,
    TTF_Font* fonte,
    const char* texto,
    SDL_Color cor,
    float x,
    float y)
{
    SdlPtr<SDL_Surface> surface(TTF_RenderText_Blended(fonte, texto, 0, cor));
    SdlPtr<SDL_Texture> texture(SDL_CreateTextureFromSurface(renderer, surface.get()));

    SDL_FRect rect;

    SDL_GetTextureSize(texture.get(), &rect.w, &rect.h);

    rect.x = x;
    rect.y = y;

    SDL_RenderTexture(renderer, texture.get(), NULL, &rect);
}


SDL_FRect MedirTextoCentrado(TTF_Font* fonte,
    const char* texto,
    float centroX,
    float y)
{
    int largura = 0;
    int altura = 0;

    TTF_GetStringSize(fonte, texto, 0, &largura, &altura);

    SDL_FRect rect;

    rect.w = (float)largura;
    rect.h = (float)altura;
    rect.x = centroX - rect.w / 2;
    rect.y = y;

    return rect;
}