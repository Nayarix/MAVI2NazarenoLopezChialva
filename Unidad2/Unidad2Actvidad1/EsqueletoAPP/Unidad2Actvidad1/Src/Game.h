#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "SFMLRenderer.h"
#include <list>

using namespace sf;
class Game
{
private:
    
    int alto;
    int ancho;
    RenderWindow* wnd;
    Color clearColor;

    
    b2World* phyWorld;
    SFMLRenderer* debugRender;

  
    b2Body* pisoBody;
    b2Body* bloqueBody;

    sf::View gameView;
public:
    Game(int ancho, int alto, std::string titulo);
    ~Game(void);
    void InitPhysics();

  
    void Loop();
    void DrawGame();
    void UpdatePhysics();
    void DoEvents();
};