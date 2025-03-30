#include "Game.h"
#include "Box2DHelper.h"

Game::Game(int ancho, int alto, std::string titulo)
{
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    clearColor = sf::Color(30, 30, 40); 
    InitPhysics();
}

void Game::InitPhysics()
{

    phyWorld = new b2World(b2Vec2(0.0f, 0.0f));


    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);

    pelotaBody = Box2DHelper::CreateCircularDynamicBody(
        phyWorld,
        15.0f,    
        1.0f,    
        0.05f,   
        1.0f      
    );
    pelotaBody->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, wnd->getSize().y / 2.0f), 0.0f);

  
    float velocidad = 5000.0f;
    pelotaBody->ApplyLinearImpulse(b2Vec2(velocidad, -velocidad), pelotaBody->GetWorldCenter(), true);

    float grosorBordes = 20.0f;

   
    bordeSuperior = Box2DHelper::CreateRectangularStaticBody(phyWorld, wnd->getSize().x, grosorBordes);
    bordeSuperior->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, -grosorBordes / 2.0f), 0.0f);

    
    bordeInferior = Box2DHelper::CreateRectangularStaticBody(phyWorld, wnd->getSize().x, grosorBordes);
    bordeInferior->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, wnd->getSize().y + grosorBordes / 2.0f), 0.0f);

   
    bordeIzquierdo = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosorBordes, wnd->getSize().y);
    bordeIzquierdo->SetTransform(b2Vec2(-grosorBordes / 2.0f, wnd->getSize().y / 2.0f), 0.0f);

  
    bordeDerecho = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosorBordes, wnd->getSize().y);
    bordeDerecho->SetTransform(b2Vec2(wnd->getSize().x + grosorBordes / 2.0f, wnd->getSize().y / 2.0f), 0.0f);
}

void Game::DrawGame()
{
    wnd->clear(clearColor);

   
    sf::CircleShape pelota(15.0f);
    pelota.setFillColor(sf::Color(255, 100, 255));
    pelota.setOrigin(15.0f, 15.0f);
    pelota.setPosition(pelotaBody->GetPosition().x, pelotaBody->GetPosition().y);
    wnd->draw(pelota);

   
    sf::RectangleShape borde(sf::Vector2f(wnd->getSize().x, 20.0f));
    borde.setFillColor(sf::Color(100, 100, 100, 150));
    borde.setOrigin(wnd->getSize().x / 2.0f, 10.0f);

  
    borde.setPosition(bordeSuperior->GetPosition().x, bordeSuperior->GetPosition().y);
    wnd->draw(borde);

  
    borde.setPosition(bordeInferior->GetPosition().x, bordeInferior->GetPosition().y);
    wnd->draw(borde);

   
    borde.setSize(sf::Vector2f(20.0f, wnd->getSize().y));
    borde.setOrigin(10.0f, wnd->getSize().y / 2.0f);

  
    borde.setPosition(bordeIzquierdo->GetPosition().x, bordeIzquierdo->GetPosition().y);
    wnd->draw(borde);

 
    borde.setPosition(bordeDerecho->GetPosition().x, bordeDerecho->GetPosition().y);
    wnd->draw(borde);
}

void Game::UpdatePhysics()
{
  
    phyWorld->Step(1.0f / 60.0f, 8, 8);
}

void Game::DoEvents()
{
    sf::Event evt;
    while (wnd->pollEvent(evt))
    {
        if (evt.type == sf::Event::Closed)
            wnd->close();

        if (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::R)
        {
            pelotaBody->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, wnd->getSize().y / 2.0f), 0.0f);
            float velocidad = 5000.0f;
            pelotaBody->SetLinearVelocity(b2Vec2(velocidad, -velocidad));
        }
    }
}

void Game::Loop()
{
    while (wnd->isOpen())
    {
        DoEvents();
        UpdatePhysics();
        DrawGame();
        wnd->display();
    }
}

Game::~Game()
{
    delete phyWorld;
    delete debugRender;
    delete wnd;
}