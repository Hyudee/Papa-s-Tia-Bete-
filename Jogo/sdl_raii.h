#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Wrappers RAII para recursos SDL.
// Os recursos são liberados automaticamente, na ordem inversa da declaração.

struct SdlDeleter
{
    void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); }
    void operator()(SDL_Renderer* p) const { SDL_DestroyRenderer(p); }
    void operator()(SDL_Texture* p) const { SDL_DestroyTexture(p); }
    void operator()(SDL_Surface* p) const { SDL_DestroySurface(p); }
    void operator()(SDL_Cursor* p) const { SDL_DestroyCursor(p); }
    void operator()(TTF_Font* p) const { TTF_CloseFont(p); }
};

template <typename T>
using SdlPtr = std::unique_ptr<T, SdlDeleter>;

// Chama SDL_Quit() ao sair do escopo (declare logo após SDL_Init ter sucesso)
struct SdlGuard
{
    SdlGuard() = default;
    SdlGuard(const SdlGuard&) = delete;
    SdlGuard& operator=(const SdlGuard&) = delete;
    ~SdlGuard() { SDL_Quit(); }
};

// Chama TTF_Quit() ao sair do escopo (declare logo após TTF_Init ter sucesso)
struct TtfGuard
{
    TtfGuard() = default;
    TtfGuard(const TtfGuard&) = delete;
    TtfGuard& operator=(const TtfGuard&) = delete;
    ~TtfGuard() { TTF_Quit(); }
};