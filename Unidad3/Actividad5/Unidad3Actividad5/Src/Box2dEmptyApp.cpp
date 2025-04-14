#include "Game.h"

int main() {
    Game* juego = new Game(800, 600, "Simulación Ragdoll");
    juego->Loop();
    delete juego;
    return 0;
}