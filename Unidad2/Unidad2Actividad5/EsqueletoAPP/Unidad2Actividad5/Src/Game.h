#pragma once
#include <SFML/Graphics.hpp>
#include "SFMLRenderer.h"
#include "Box2DHelper.h"

class Game {
private:
    sf::RenderWindow* wnd;
    b2World* phyWorld;
    SFMLRenderer* debugRender;
    b2Body* cajaBody;
    b2Body* planoInclinadoBody;
    float angulo;          
    float rozamiento;      

public:
    Game(int ancho, int alto, std::string titulo);
    ~Game();
    void InitPhysics();
    void Loop();
    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
};