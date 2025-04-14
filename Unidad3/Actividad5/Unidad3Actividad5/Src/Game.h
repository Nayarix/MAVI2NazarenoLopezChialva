#pragma once
#include <SFML/Graphics.hpp>
#include "SFMLRenderer.h"
#include "Box2DHelper.h"

class Game {
private:
    sf::RenderWindow* wnd;
    b2World* phyWorld;
    SFMLRenderer* debugRender;

   
    b2Body* cabeza;
    b2Body* torso;
    b2Body* brazoIzq;
    b2Body* brazoDer;
    b2Body* piernaIzq;
    b2Body* piernaDer;

 
    b2RevoluteJoint* cuello;
    b2RevoluteJoint* hombroIzq;
    b2RevoluteJoint* hombroDer;
    b2RevoluteJoint* caderaIzq;
    b2RevoluteJoint* caderaDer;

    b2Body* cuerpoSeleccionado;
    b2Vec2 puntoAnclaje;
    bool arrastrando;

public:
    Game(int ancho, int alto, std::string titulo);
    ~Game();
    void InitPhysics();
    void Loop();
    void UpdatePhysics();
    void DrawGame();
    void DoEvents();
    void CrearRagdoll();
    b2RevoluteJoint* CrearJointRevolute(b2Body* cuerpoA, b2Body* cuerpoB, b2Vec2 anclaje);
    b2Body* GetBodyAtPosition(float x, float y);
};