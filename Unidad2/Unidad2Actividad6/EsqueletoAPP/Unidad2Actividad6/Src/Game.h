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

    b2Body* canonBody;           
    std::vector<b2Body*> balas;  

    sf::Vector2f mousePosition;  

public:
    Game(int ancho, int alto, std::string titulo);
    ~Game();
    void InitPhysics();
    void Loop();
    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
};