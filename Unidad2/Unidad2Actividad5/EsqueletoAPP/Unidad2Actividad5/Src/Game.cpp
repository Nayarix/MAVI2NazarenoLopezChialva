#include "Game.h"
#include <cmath>

Game::Game(int ancho, int alto, std::string titulo) {
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    angulo = 30.0f;      
    rozamiento = 0.2f;    
    InitPhysics();
}

void Game::InitPhysics() {
   
    float radianes = angulo * b2_pi / 180.0f;

   
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));

    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);

   
    b2BodyDef planoDef;
    planoDef.type = b2_staticBody;
    planoInclinadoBody = phyWorld->CreateBody(&planoDef);

    
    b2PolygonShape planoShape;
    planoShape.SetAsBox(400.0f, 10.0f, b2Vec2(400.0f, 500.0f), radianes);

    
    b2FixtureDef planoFixture;
    planoFixture.shape = &planoShape;
    planoFixture.friction = rozamiento;
    planoInclinadoBody->CreateFixture(&planoFixture);

    
    cajaBody = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 40.0f, 40.0f, 1.0f, 0.0f, 0.0f);

    
    float posX = 400.0f - 300.0f * cos(radianes);
    float posY = 500.0f - 300.0f * sin(radianes);
    cajaBody->SetTransform(b2Vec2(posX, posY - 30.0f), 0.0f);
}

void Game::DrawGame() {
    wnd->clear(sf::Color(200, 220, 255));

    
    sf::RectangleShape plano(sf::Vector2f(800.0f, 20.0f));
    plano.setFillColor(sf::Color(80, 80, 80));
    plano.setOrigin(400.0f, 10.0f);
    plano.setPosition(400.0f, 500.0f);
    plano.setRotation(angulo);
    wnd->draw(plano);

    
    sf::RectangleShape caja(sf::Vector2f(40.0f, 40.0f));
    caja.setFillColor(sf::Color(255, 100, 100));
    caja.setOrigin(20.0f, 20.0f);
    caja.setPosition(cajaBody->GetPosition().x, cajaBody->GetPosition().y);
    caja.setRotation(cajaBody->GetAngle() * 180.0f / b2_pi);
    wnd->draw(caja);

    
    sf::Font font;
    if (font.loadFromFile("arial.ttf")) {
        sf::Text texto;
        texto.setFont(font);
        texto.setString("Angulo: " + std::to_string((int)angulo)); 
        texto.setCharacterSize(24);
        texto.setFillColor(sf::Color::Black);
        texto.setPosition(20.0f, 20.0f);
        wnd->draw(texto);
    }
}

void Game::UpdatePhysics() {
    phyWorld->Step(1.0f / 60.0f, 8, 8);
}

void Game::DoEvents() {
    sf::Event evt;
    while (wnd->pollEvent(evt)) {
        if (evt.type == sf::Event::Closed)
            wnd->close();

        
        if (evt.type == sf::Event::KeyPressed) {
            if (evt.key.code == sf::Keyboard::R) { 
                float radianes = angulo * b2_pi / 180.0f;
                float posX = 400.0f - 300.0f * cos(radianes);
                float posY = 500.0f - 300.0f * sin(radianes);
                cajaBody->SetTransform(b2Vec2(posX, posY - 30.0f), 0.0f);
                cajaBody->SetLinearVelocity(b2Vec2_zero);
            }
            else if (evt.key.code == sf::Keyboard::Up) { 
                angulo = std::min(75.0f, angulo + 5.0f);
                planoInclinadoBody->GetFixtureList()->SetFriction(rozamiento);
                InitPhysics();
            }
            else if (evt.key.code == sf::Keyboard::Down) { 
                angulo = std::max(5.0f, angulo - 5.0f);
                planoInclinadoBody->GetFixtureList()->SetFriction(rozamiento);
                InitPhysics();
            }

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