#include "Game.h"

Game::Game(int ancho, int alto, std::string titulo) {
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    coeficienteRozamiento = 0.5f;
    InitPhysics();
}

void Game::InitPhysics() {
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);


    pisoBody = Box2DHelper::CreateRectangularStaticBody(phyWorld, 800.0f, 10.0f);
    pisoBody->SetTransform(b2Vec2(400.0f, 590.0f), 0.0f);
    b2Fixture* pisoFixture = pisoBody->GetFixtureList();
    pisoFixture->SetFriction(coeficienteRozamiento);

  
    cajaBody = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 40.0f, 40.0f, 0.5f, 0.7f, 0.1f);
    cajaBody->SetTransform(b2Vec2(400.0f, 300.0f), 0.0f);
}

void Game::DrawGame() {
    wnd->clear(sf::Color(245, 245, 245));

  
    sf::RectangleShape piso(sf::Vector2f(800.0f, 10.0f));
    piso.setFillColor(sf::Color(100, 100, 120));
    piso.setOrigin(400.0f, 5.0f);
    piso.setPosition(pisoBody->GetPosition().x, pisoBody->GetPosition().y);
    wnd->draw(piso);

    
    sf::RectangleShape caja(sf::Vector2f(40.0f, 40.0f));
    caja.setFillColor(sf::Color(150, 75, 0));  
    caja.setOutlineThickness(2.0f);
    caja.setOutlineColor(sf::Color(100, 50, 0));
    caja.setOrigin(20.0f, 20.0f);
    caja.setPosition(cajaBody->GetPosition().x, cajaBody->GetPosition().y);
    caja.setRotation(cajaBody->GetAngle() * 180.0f / b2_pi);
    wnd->draw(caja);

 
    sf::Font font;
    if (font.loadFromFile("arial.ttf")) {
        sf::Text texto;
        texto.setFont(font);
        texto.setString("Rozamiento: " + std::to_string(coeficienteRozamiento).substr(0, 4));
        texto.setCharacterSize(24);
        texto.setFillColor(sf::Color::Black);
        texto.setPosition(20.0f, 20.0f);
        wnd->draw(texto);
        texto.setString("\n " + std::string(int(coeficienteRozamiento * 10), '""') + "");
        texto.setFillColor(sf::Color::White);
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
            if (evt.key.code >= sf::Keyboard::Num1 && evt.key.code <= sf::Keyboard::Num5) {
               
                float valoresRozamiento[] = { 10.1f, 50.4f, 100.7f, 150.0f, 200.3f };
                coeficienteRozamiento = valoresRozamiento[evt.key.code - sf::Keyboard::Num1];

              
                pisoBody->GetFixtureList()->SetFriction(coeficienteRozamiento);
                cajaBody->GetFixtureList()->SetFriction(coeficienteRozamiento * 0.5f);
            }
        }
    }

  
    float fuerzaBase = 120.0f;
    float fuerza = fuerzaBase * (1.0f + coeficienteRozamiento);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        cajaBody->ApplyLinearImpulse(b2Vec2(-fuerza, 0.0f), cajaBody->GetWorldCenter(), true);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        cajaBody->ApplyLinearImpulse(b2Vec2(fuerza, 0.0f), cajaBody->GetWorldCenter(), true);
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