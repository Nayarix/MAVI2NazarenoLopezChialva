#include "Game.h"

int main() {
    Game juego(800, 600, "Pelota rebotando con ley de elasticidad");
    juego.Loop();
    return 0;
}