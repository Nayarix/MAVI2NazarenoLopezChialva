#include "Game.h"
// Punto de entrada del programa
int main() {
	// Se crea una instancia del juego con dimensiones y título
    Game juego(800, 600, "Actividad Unidad 4");
	// Se inicia el bucle principal del juego, que se ejecutará hasta que la ventana se cierre
    juego.Loop();
    return 0;
}