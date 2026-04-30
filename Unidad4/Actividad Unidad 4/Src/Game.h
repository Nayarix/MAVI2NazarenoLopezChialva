#pragma once
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include "Box2DHelper.h"
#include "SFMLRenderer.h"

// ============================================================
// CLASE RAGDOLL: Representa al personaje disparado
// ============================================================
class Ragdoll {
public:
    // --- Cuerpos Físicos (Box2D) ---
    b2Body* cabeza;
    b2Body* torso;
    b2Body* brazoIzq;
    b2Body* brazoDer;
    b2Body* piernaIzq;
    b2Body* piernaDer;

    // --- Uniones Físicas (Joints) ---
    b2Joint* cuello;
    b2Joint* hombroIzq;
    b2Joint* hombroDer;
    b2Joint* caderaIzq;
    b2Joint* caderaDer;

    // --- Lógica de Control ---
    float tiempoVida = 0.0f; // Temporizador para la desaparición automática

    Ragdoll(b2World* world, const b2Vec2& position);
    void ApplyImpulse(const b2Vec2& impulse); // Aplica la fuerza del disparo
};

// ============================================================
// CLASE OBSTACULO: Bloques, plataformas y objetivos
// ============================================================
class Obstaculo {
public:
    b2Body* body;            // Cuerpo físico en el mundo de Box2D
    sf::Color color;         // Color (si se usa dibujo por formas)
    bool esEstatico;         // Diferencia entre paredes y cajas movibles
    sf::Sprite spriteVisual; // Representación gráfica con textura
    b2Vec2 posicionInicial;  // Guardada para verificar la condición de victoria

    Obstaculo(b2World* world, const b2Vec2& position, float width, float height, bool estatico, sf::Texture& textura);
    void Draw(sf::RenderWindow* window) const;
};

// ============================================================
// ENUMERACIÓN DE ESTADOS: Control de pantallas del juego
// ============================================================
enum class GameState {
    MAIN_MENU,    // Pantalla inicial
    LEVEL_SELECT, // Selección de niveles 1, 2 o 3
    RAGDOLL_GUIDE,// Pantalla de instrucciones
    PLAYING,      // Durante el juego activo
    LEVEL_WON     // Pantalla de nivel superado
};

// ============================================================
// CLASE GAME: Motor principal del juego
// ============================================================
class Game {
private:
    // --- Motores y Ventana ---
    sf::RenderWindow* window;
    b2World* physicsWorld;       // El "universo" donde ocurre la física
    

    // --- Recursos Visuales (Texturas) ---
    sf::Texture texturaCaja;
    sf::Texture texturaMetal;
    sf::Texture texturaMetalPlat;
    sf::Texture texturaMetalRueda;
    sf::Texture texturaFondo;
    sf::Texture texturaCañon;

    // --- Piezas del Cañón ---
    sf::RectangleShape cannonBase;   // Base gris estática
    sf::RectangleShape cannonBarrel; // Tubo que rota

    // --- Contenedores de Objetos ---
    // Uso unique_ptr para una gestión de memoria automática y segura
    std::vector<std::unique_ptr<Ragdoll>> ragdolls;
    std::vector<std::unique_ptr<Obstaculo>> obstaculos;
    std::vector<b2Body*> boundaryWalls; // Paredes invisibles de los bordes

    // --- Constantes de Física y Configuración ---
    const float PIXELS_TO_METERS = 30.0f; // Factor de conversión Box2D <-> SFML
    const float CANNON_LENGTH = 50.0f;    // Largo visual del cañón
    const float MAX_CANNON_POWER = 30.0f; // Impulso máximo del disparo
    const float MIN_CANNON_POWER = 5.0f;  // Impulso mínimo del disparo
    const float MAX_DISTANCE = 300.0f;    // Distancia del mouse para potencia máxima
    float timerVictoria = 0.0f;           // Tiempo de espera para el menú de ganar

    GameState estadoActual;  // Estado de la máquina de estados
    int nivelSeleccionado;   // Identificador del nivel actual

    // --- Elementos de Interfaz de Usuario (UI) ---
    sf::Font fuente; // Fuente para todos los textos del juego

    struct Boton {
        sf::RectangleShape forma;
        sf::Text texto;
    };

    // Botones definidos por categoría
    Boton btnCerrar, btnNiveles, btnGuia;           // Menú Principal
    Boton btnLvl1, btnLvl2, btnLvl3, btnVolver;     // Selección de Nivel
    Boton btnMenuPrincipal, btnVolverMenu;          // Durante el juego y Victoria

    // --- Métodos Privados de Inicialización ---
    void InicializarBotones();
    void CargarNivel(int num); // Configura joints y cuerpos según el nivel

public:
    // --- Ciclo de Vida ---
    Game(int width, int height, const std::string& title);
    ~Game();

    // --- Métodos de ejecución principal ---
    void Loop();                     // El bucle While principal
    void Update(float deltaTime);    // Lógica física y de tiempo
    void Render();                   // Dibujado de todos los elementos
    void HandleEvents();             // Captura de mouse, teclado y cierre

    // --- Métodos de Mecánica de Juego ---
    void UpdateCannonRotation();               // Rota el cañón hacia el mouse
    void FireRagdoll();                        // Crea e impulsa un nuevo ragdoll
    void CreateBoundaries(float width, float height); // Crea los límites del mundo
    float CalculatePower(const sf::Vector2f& mousePos); // Calcula potencia de disparo

    // --- Métodos de Dibujo y Utilidad ---
    void DrawBody(b2Body* body, const sf::Color& color); // Dibuja formas de Box2D
    bool EsClicEnBoton(const Boton& b, const sf::Vector2f& mousePos); // Detección de clics
    void LimpiarMundo(); // Destruye cuerpos físicos al cambiar de escena
};