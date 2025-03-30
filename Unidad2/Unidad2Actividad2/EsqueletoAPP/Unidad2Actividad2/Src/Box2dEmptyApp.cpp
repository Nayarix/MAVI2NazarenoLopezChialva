#include "Game.h"
#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
    Game* juego = new Game(800, 600, "Pelota rebontando en paredes");
    juego->Loop();
    delete juego;
    return 0;
}