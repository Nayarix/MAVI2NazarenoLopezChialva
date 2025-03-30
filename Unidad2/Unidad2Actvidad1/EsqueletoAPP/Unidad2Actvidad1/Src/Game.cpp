#include "Game.h"
#include "Box2DHelper.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

Game::Game(int ancho, int alto, std::string titulo)
{

    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    wnd->setVisible(true);
    clearColor = sf::Color(40, 40, 40); 


    gameView.setSize(ancho, alto);
    gameView.setCenter(ancho / 2.0f, alto / 2.0f);
    wnd->setView(gameView);


    InitPhysics();
}

void Game::Loop()
{

    while (wnd->isOpen())
    {
        wnd->clear(clearColor);
        DoEvents();
        UpdatePhysics();
        DrawGame();
        wnd->display();
    }
}

void Game::UpdatePhysics()
{

    phyWorld->Step(1.0f / 60.0f, 8, 8);
    phyWorld->ClearForces();
    phyWorld->DebugDraw(); 
}

void Game::DrawGame()
{

    sf::Vector2f floorSize(wnd->getSize().x * 0.8f, 30.0f);
    b2Vec2 floorPos = pisoBody->GetPosition();

    sf::RectangleShape piso(floorSize);
    piso.setFillColor(sf::Color(50, 150, 50));
    piso.setOrigin(floorSize.x / 2.0f, floorSize.y / 2.0f);
    piso.setPosition(floorPos.x, floorPos.y);
    wnd->draw(piso);


    sf::Vector2f blockSize(50.0f, 50.0f);
    b2Vec2 blockPos = bloqueBody->GetPosition();

    sf::RectangleShape bloque(blockSize);
    bloque.setFillColor(sf::Color(220, 100, 0)); 
    bloque.setOrigin(blockSize.x / 2.0f, blockSize.y / 2.0f);
    bloque.setPosition(blockPos.x, blockPos.y);
    wnd->draw(bloque);


    sf::Font font;
    if (font.loadFromFile("Arial.ttf"))
    {
        sf::Text text;
        text.setFont(font);
        text.setString("Simulacion de caida libre con Box2D :)");
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setPosition(20, 20);
        wnd->draw(text);
    }
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
            bloqueBody->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, 100.0f), 0.0f);
            bloqueBody->SetLinearVelocity(b2Vec2_zero);
            bloqueBody->SetAngularVelocity(0.0f);
        }
    }
}

void Game::InitPhysics()
{
  
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));

   
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);
    phyWorld->SetDebugDraw(debugRender);

  
    float floorWidth = wnd->getSize().x * 0.8f;
    float floorHeight = 30.0f;
    pisoBody = Box2DHelper::CreateRectangularStaticBody(phyWorld, floorWidth, floorHeight);
    pisoBody->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, wnd->getSize().y - 50.0f), 0.0f);

  
    float blockSize = 50.0f;
    bloqueBody = Box2DHelper::CreateRectangularDynamicBody(
        phyWorld,
        blockSize,
        blockSize,
        1.0f,    
        0.4f,    
        0.3f);   


    bloqueBody->SetTransform(b2Vec2(wnd->getSize().x / 2.0f, 100.0f), 0.0f);
}

Game::~Game()
{
   
    delete phyWorld;
    delete debugRender;
    delete wnd;
}