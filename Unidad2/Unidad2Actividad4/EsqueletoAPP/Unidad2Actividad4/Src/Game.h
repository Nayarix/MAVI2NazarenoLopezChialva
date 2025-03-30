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
    b2Body* pisoBody;
    float coeficienteRozamiento;
public:
    Game(int ancho, int alto, std::string titulo);
    ~Game();
    void InitPhysics();
    void Loop();
    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
};