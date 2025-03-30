#pragma once
#include <SFML/Graphics.hpp>
#include "SFMLRenderer.h"
#include "Box2DHelper.h"

class Game
{
private:

    sf::RenderWindow* wnd;
    sf::Color clearColor;


    b2World* phyWorld;
    SFMLRenderer* debugRender;


    b2Body* pelotaBody;                 
    b2Body* bordeSuperior;           
    b2Body* bordeInferior;            
    b2Body* bordeIzquierdo;            
    b2Body* bordeDerecho;                

public:

    Game(int ancho, int alto, std::string titulo);
    ~Game();


    void InitPhysics();


    void Loop();


    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
};