#pragma once
#include <SFML/Graphics.hpp>
#include "SFMLRenderer.h"
#include "Box2DHelper.h"
#include <vector>

class Game {
private:
    sf::RenderWindow* wnd;
    sf::Color clearColor;
    b2World* phyWorld;
    SFMLRenderer* debugRender;
    b2Body* pelotaBody;
    b2Body* bordeSuperior, * bordeInferior, * bordeIzquierdo, * bordeDerecho;
    std::vector<b2Body*> obstaculos;
public:
    Game(int ancho, int alto, std::string titulo);
    ~Game();
    void InitPhysics();
    void Loop();
    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
};