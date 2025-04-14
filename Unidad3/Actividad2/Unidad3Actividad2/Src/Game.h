#pragma once
#include <SFML/Graphics.hpp>
#include "SFMLRenderer.h"
#include "Box2DHelper.h"
#include <vector>

class Game {
private:
    sf::RenderWindow* wnd;
    b2World* phyWorld;
    SFMLRenderer* debugRender;

    std::vector<b2Body*> pelotas;  
    b2Joint* resorte;             
    b2Body* pelotaSeleccionada;    
    bool arrastrando;

public:
    Game(int ancho, int alto, std::string titulo);
    ~Game();
    void InitPhysics();
    void Loop();
    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
    void CrearResorte(b2Body* cuerpoA, b2Body* cuerpoB);
};