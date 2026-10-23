#include <Windows.h>

#include "jogo.h"

int main(int, char*[])
{
#ifdef NDEBUG
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

    Jogo jogo;

    if (!jogo.Iniciar())
        return -1;

    jogo.Executar();

    return 0;
}