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

    Ragdoll(b2World* world, const b2Vec2& position);
    void ApplyImpulse(const b2Vec2& impulse);
};

class Obstaculo {
public:
    b2Body* body;
    sf::Color color;
    bool esEstatico;

    Obstaculo(b2World* world, const b2Vec2& position, float width, float height, bool estatico, const sf::Color& col);
    void Draw(sf::RenderWindow* window, float pixelsToMeters) const;
};

class Game {
private:
    sf::RenderWindow* window;
    b2World* physicsWorld;
    SFMLRenderer* debugRenderer;

    
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
};