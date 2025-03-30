#include "Game.h"
#include <cmath>
#include <algorithm>

Game::Game(int ancho, int alto, std::string titulo) {
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    InitPhysics();
}

void Game::InitPhysics() {
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);

  
    b2Vec2 vertices[3];
    vertices[0].Set(-25.0f, -25.0f);  
    vertices[1].Set(25.0f, -25.0f);
    vertices[2].Set(0.0f, 50.0f);     

    canonBody = Box2DHelper::CreateTriangularStaticBody(phyWorld, b2Vec2(50.0f, 50.0f), 100.0f);
    canonBody->SetTransform(b2Vec2(70.0f, 550.0f), 0.0f);  
}

void Game::DrawGame() {
    wnd->clear(sf::Color(30, 30, 40));

    
    b2Vec2 posCanon = canonBody->GetPosition();
    sf::ConvexShape canon(3);
    canon.setPoint(0, sf::Vector2f(-25.0f, -25.0f));
    canon.setPoint(1, sf::Vector2f(25.0f, -25.0f));
    canon.setPoint(2, sf::Vector2f(0.0f, 50.0f));
    canon.setPosition(posCanon.x, posCanon.y);
    canon.setRotation(canonBody->GetAngle() * 180.0f / b2_pi);
    canon.setFillColor(sf::Color(180, 100, 50));  
    canon.setOutlineThickness(2.0f);
    canon.setOutlineColor(sf::Color::Black);
    wnd->draw(canon);

    
    for (b2Body* bala : balas) {
        sf::CircleShape proyectil(12.0f);  
        proyectil.setFillColor(sf::Color(255, 150, 50));  
        proyectil.setOutlineThickness(2.0f);
        proyectil.setOutlineColor(sf::Color(200, 80, 0));
        proyectil.setOrigin(12.0f, 12.0f);
        proyectil.setPosition(bala->GetPosition().x, bala->GetPosition().y);
        wnd->draw(proyectil);
    }
}

void Game::UpdatePhysics() {
    phyWorld->Step(1.0f / 60.0f, 8, 8);

    
    balas.erase(std::remove_if(balas.begin(), balas.end(),
        [](b2Body* b) {
            b2Vec2 pos = b->GetPosition();
            return pos.x > 850 || pos.x < -50 || pos.y > 650 || pos.y < -50;
        }), balas.end());
}

void Game::DoEvents() {
    sf::Event evt;
    while (wnd->pollEvent(evt)) {
        if (evt.type == sf::Event::Closed)
            wnd->close();

        if (evt.type == sf::Event::MouseButtonPressed && evt.mouseButton.button == sf::Mouse::Left) {
            
            sf::Vector2f canonPos(70.0f, 550.0f); 
            sf::Vector2f target = wnd->mapPixelToCoords(sf::Mouse::getPosition(*wnd));
            sf::Vector2f direccion = target - canonPos;
            float magnitud = std::sqrt(direccion.x * direccion.x + direccion.y * direccion.y);
            sf::Vector2f direccionNormalizada = direccion / magnitud;

          
            b2Body* bala = Box2DHelper::CreateCircularDynamicBody(
                phyWorld,
                12.0f,   
                0.3f,     
                0.05f,    
                0.7f      
            );

            
            float angulo = canonBody->GetAngle();
            b2Vec2 posInicial(
                70.0f + 40.0f * cos(angulo),
                550.0f + 40.0f * sin(angulo)
            );
            bala->SetTransform(posInicial, angulo);

            
            float fuerza = 20000.0f;  
            b2Vec2 impulso(
                fuerza * direccionNormalizada.x,
                fuerza * direccionNormalizada.y * 0.7f  
            );
            bala->ApplyLinearImpulse(impulso, bala->GetWorldCenter(), true);

            balas.push_back(bala);
        }

        if (evt.type == sf::Event::MouseMoved) {
            
            sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(evt.mouseMove.x, evt.mouseMove.y));
            sf::Vector2f direccion = mousePos - sf::Vector2f(70.0f, 550.0f);
            float angulo = atan2(direccion.y, direccion.x);
            canonBody->SetTransform(canonBody->GetPosition(), angulo * 0.95f);  
        }
    }
}

void Game::Loop() {
    while (wnd->isOpen()) {
        DoEvents();
        UpdatePhysics();
        DrawGame();
        wnd->display();
    }
}

Game::~Game() {
    delete phyWorld;
    delete debugRender;
    delete wnd;
}