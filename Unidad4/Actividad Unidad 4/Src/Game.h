#pragma once
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include "Box2DHelper.h"
#include "SFMLRenderer.h"

class Ragdoll {
public:
    b2Body* cabeza;
    b2Body* torso;
    b2Body* brazoIzq;
    b2Body* brazoDer;
    b2Body* piernaIzq;
    b2Body* piernaDer;
    b2Body* escudo;

    b2Joint* cuello;
    b2Joint* hombroIzq;
    b2Joint* hombroDer;
    b2Joint* caderaIzq;
    b2Joint* caderaDer;

    float tiempoVida = 0.0f;
    Ragdoll(b2World* world, const b2Vec2& position);
    void ApplyImpulse(const b2Vec2& impulse);
};

class Obstaculo {
public:
    b2Body* body;
    sf::Color color;
    bool esEstatico;
    sf::Sprite spriteVisual;
    b2Vec2 posicionInicial;

    Obstaculo(b2World* world, const b2Vec2& position, float width, float height, bool estatico, sf::Texture& textura);
    void Draw(sf::RenderWindow* window) const;
};

enum class GameState {
    MAIN_MENU,
    LEVEL_SELECT,
    RAGDOLL_GUIDE,
    PLAYING,
	LEVEL_WON
};



class Game {
private:
    sf::RenderWindow* window;
    b2World* physicsWorld;
    SFMLRenderer* debugRenderer;

    sf::Texture texturaCaja;
    sf::Texture texturaPlataforma;
	sf::Texture texturaMetal;
    sf::Texture texturaMetalPlat;
	sf::Texture texturaMetalRueda;
    sf::Texture texturaFondo;
    sf::Texture texturaCañon;

    sf::RectangleShape cannonBase;
    sf::RectangleShape cannonBarrel;

    
    std::vector<std::unique_ptr<Ragdoll>> ragdolls;
    std::vector<std::unique_ptr<Obstaculo>> obstaculos;
    std::vector<b2Body*> boundaryWalls;

    
    const float PIXELS_TO_METERS = 30.0f;
    const float CANNON_LENGTH = 50.0f;
    const float MAX_CANNON_POWER = 30.0f;
    const float MIN_CANNON_POWER = 5.0f;
    const float MAX_DISTANCE = 300.0f;
	float timerVictoria = 0.0f;

    GameState estadoActual;
    int nivelSeleccionado; // 1, 2 o 3

    // UI Elements
    sf::Font fuente;
    // Podrías usar un pequeño struct para botones para que sea más limpio
    struct Boton {
        sf::RectangleShape forma;
        sf::Text texto;
    };

    // Botones del Menú Principal
    Boton btnCerrar, btnNiveles, btnGuia;
    // Botones de Selección de Nivel
    Boton btnLvl1, btnLvl2, btnLvl3, btnVolver;
    // Botón durante el juego
    Boton btnMenuPrincipal;
	Boton btnVolverMenu;

    void InicializarBotones();
    void CargarNivel(int num);

public:
    Game(int width, int height, const std::string& title);
    ~Game();
    void Loop();
    void Update(float deltaTime);
    void Render();
    void HandleEvents();
    void UpdateCannonRotation();
    void FireRagdoll();
    void CreateBoundaries(float width, float height);
    void CreateObstacles(); 
    float CalculatePower(const sf::Vector2f& mousePos);
    void DrawBody(b2Body* body, const sf::Color& color); 
    bool EsClicEnBoton(const Boton& b, const sf::Vector2i& mousePos);
    void LimpiarMundo();
};

