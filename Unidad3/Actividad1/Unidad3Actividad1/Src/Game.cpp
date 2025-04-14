#include "Game.h"

Game::Game(int ancho, int alto, std::string titulo) {
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    InitPhysics();
}

void Game::InitPhysics() {
    phyWorld = new b2World(b2Vec2(0.0f, 0.0f));

    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);

   
    pelotaBody = Box2DHelper::CreateCircularDynamicBody(
        phyWorld,
        15.0f,   
        1.0f,     
        0.0f,     
        1.0f      
    );


    pelotaBody->SetTransform(b2Vec2(100.0f, 100.0f), 0.0f);
    pelotaBody->SetLinearVelocity(b2Vec2(5.0f, -30.0f));

    
    float grosor = 20.0f;
    int anchoPantalla = wnd->getSize().x;
    int altoPantalla = wnd->getSize().y;

  
    bordes.push_back(Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor));
    bordes.back()->SetTransform(b2Vec2(anchoPantalla / 2.0f, -grosor / 2.0f), 0.0f);

  
    bordes.push_back(Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor));
    bordes.back()->SetTransform(b2Vec2(anchoPantalla / 2.0f, altoPantalla + grosor / 2.0f), 0.0f);


    bordes.push_back(Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla));
    bordes.back()->SetTransform(b2Vec2(-grosor / 2.0f, altoPantalla / 2.0f), 0.0f);

   
    bordes.push_back(Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla));
    bordes.back()->SetTransform(b2Vec2(anchoPantalla + grosor / 2.0f, altoPantalla / 2.0f), 0.0f);
}

void Game::DrawGame() {
    wnd->clear(sf::Color::Black);

  
    sf::CircleShape pelota(15.0f);
    pelota.setFillColor(sf::Color(100, 150, 255));
    pelota.setOrigin(15.0f, 15.0f);
    pelota.setPosition(pelotaBody->GetPosition().x, pelotaBody->GetPosition().y);
    wnd->draw(pelota);

 
    sf::RectangleShape borde(sf::Vector2f(wnd->getSize().x, 20.0f));
    borde.setFillColor(sf::Color(100, 100, 100, 150));
    borde.setOrigin(wnd->getSize().x / 2.0f, 10.0f);
    borde.setPosition(bordes[0]->GetPosition().x, bordes[0]->GetPosition().y);
    wnd->draw(borde); 

    borde.setPosition(bordes[1]->GetPosition().x, bordes[1]->GetPosition().y);
    wnd->draw(borde); 

    borde.setSize(sf::Vector2f(20.0f, wnd->getSize().y));
    borde.setOrigin(10.0f, wnd->getSize().y / 2.0f);
    borde.setPosition(bordes[2]->GetPosition().x, bordes[2]->GetPosition().y);
    wnd->draw(borde); 

    borde.setPosition(bordes[3]->GetPosition().x, bordes[3]->GetPosition().y);
    wnd->draw(borde);
}

void Game::UpdatePhysics() {
    phyWorld->Step(1.0f / 60.0f, 8, 8); 
}

void Game::DoEvents() {
    sf::Event evt;
    while (wnd->pollEvent(evt)) {
        if (evt.type == sf::Event::Closed)
            wnd->close();


        if (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::R) {
            pelotaBody->SetTransform(b2Vec2(100.0f, 100.0f), 0.0f);
            pelotaBody->SetLinearVelocity(b2Vec2(5.0f, -30.0f));
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