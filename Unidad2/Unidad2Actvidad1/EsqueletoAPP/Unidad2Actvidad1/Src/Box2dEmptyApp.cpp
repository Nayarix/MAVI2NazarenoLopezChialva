#include "Game.h"
#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
    Game* juego = new Game(800, 600, "Caída de bloque con Box2D");
    juego->Loop();
    delete juego;
    return 0;
}