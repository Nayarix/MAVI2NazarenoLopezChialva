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

   
    cuadrado = Box2DHelper::CreateRectangularKinematicBody(phyWorld, 40.0f, 40.0f);
    cuadrado->SetTransform(b2Vec2(400.0f, 300.0f), 0.0f);

   
    float grosor = 20.0f;
    int anchoPantalla = wnd->getSize().x;
    int altoPantalla = wnd->getSize().y;

    bordes[0] = Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor);
    bordes[0]->SetTransform(b2Vec2(anchoPantalla / 2.0f, -grosor / 2.0f), 0.0f);

    bordes[1] = Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor);
    bordes[1]->SetTransform(b2Vec2(anchoPantalla / 2.0f, altoPantalla + grosor / 2.0f), 0.0f);

    bordes[2] = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla);
    bordes[2]->SetTransform(b2Vec2(-grosor / 2.0f, altoPantalla / 2.0f), 0.0f);

    bordes[3] = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla);
    bordes[3]->SetTransform(b2Vec2(anchoPantalla + grosor / 2.0f, altoPantalla / 2.0f), 0.0f);
}

void Game::ProcesarInput() {
    const float fuerzaMovimiento = 10.0f;
    b2Vec2 velocidadActual = cuadrado->GetLinearVelocity();
    b2Vec2 nuevaVelocidad(0.0f, 0.0f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        nuevaVelocidad.x = -fuerzaMovimiento;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        nuevaVelocidad.x = fuerzaMovimiento;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        nuevaVelocidad.y = -fuerzaMovimiento;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        nuevaVelocidad.y = fuerzaMovimiento;
    }

    
    if (nuevaVelocidad.x != 0 || nuevaVelocidad.y != 0) {
        cuadrado->SetLinearVelocity(nuevaVelocidad);
    }
    else {
        cuadrado->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    }
}

void Game::DrawGame() {
    wnd->clear(sf::Color(40, 40, 50));

    
    sf::RectangleShape rect(sf::Vector2f(40.0f, 40.0f));
    rect.setOrigin(20.0f, 20.0f);
    rect.setPosition(cuadrado->GetPosition().x, cuadrado->GetPosition().y);
    rect.setFillColor(sf::Color(100, 200, 255));
    wnd->draw(rect);

    
    for (int i = 0; i < 4; i++) {
        b2Fixture* fixture = bordes[i]->GetFixtureList();
        b2PolygonShape* shape = (b2PolygonShape*)fixture->GetShape();
        debugRender->DrawSolidPolygon(shape->m_vertices, shape->m_count, b2Color(0.5f, 0.5f, 0.5f));
    }
}

void Game::UpdatePhysics() {
    ProcesarInput();  
    phyWorld->Step(1.0f / 60.0f, 8, 8);

    
    b2Vec2 pos = cuadrado->GetPosition();
    int ancho = wnd->getSize().x;
    int alto = wnd->getSize().y;

    if (pos.x < 20.0f) pos.x = 20.0f;
    if (pos.x > ancho - 20.0f) pos.x = ancho - 20.0f;
    if (pos.y < 20.0f) pos.y = 20.0f;
    if (pos.y > alto - 20.0f) pos.y = alto - 20.0f;

    cuadrado->SetTransform(pos, 0.0f);
}

void Game::DoEvents() {
    sf::Event evt;
    while (wnd->pollEvent(evt)) {
        if (evt.type == sf::Event::Closed)
            wnd->close();
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