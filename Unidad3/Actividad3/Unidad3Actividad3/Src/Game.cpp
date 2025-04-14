#include "Game.h"

Game::Game(int ancho, int alto, std::string titulo) {
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    arrastrando = false;
    pelotaSeleccionada = nullptr;
    InitPhysics();
}

void Game::CrearResorte(b2Body* cuerpoA, b2Body* cuerpoB) {
    b2DistanceJointDef jointDef;
    jointDef.Initialize(cuerpoA, cuerpoB, cuerpoA->GetPosition(), cuerpoB->GetPosition());
    jointDef.minLength = 50.0f;
    jointDef.maxLength = 400.0f;
    jointDef.collideConnected = true;
    jointDef.stiffness = 8.0f;
    jointDef.damping = 0.3f;
    resorte = phyWorld->CreateJoint(&jointDef);
}

void Game::InitPhysics() {
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);

    
    pelotaFija = Box2DHelper::CreateKinematicBody(phyWorld);
    pelotaFija->SetTransform(b2Vec2(400.0f, 300.0f), 0.0f);

    
    pelotaDinamica = Box2DHelper::CreateCircularDynamicBody(phyWorld, 20.0f, 1.0f, 0.3f, 0.5f);
    pelotaDinamica->SetTransform(b2Vec2(600.0f, 300.0f), 0.0f);

    CrearResorte(pelotaFija, pelotaDinamica);

    
    float grosor = 20.0f;
    int anchoPantalla = wnd->getSize().x;
    int altoPantalla = wnd->getSize().y;

    b2Body* borde;
    borde = Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor);
    borde->SetTransform(b2Vec2(anchoPantalla / 2.0f, -grosor / 2.0f), 0.0f);

    borde = Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor);
    borde->SetTransform(b2Vec2(anchoPantalla / 2.0f, altoPantalla + grosor / 2.0f), 0.0f);

    borde = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla);
    borde->SetTransform(b2Vec2(-grosor / 2.0f, altoPantalla / 2.0f), 0.0f);

    borde = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla);
    borde->SetTransform(b2Vec2(anchoPantalla + grosor / 2.0f, altoPantalla / 2.0f), 0.0f);
}

void Game::DrawGame() {
    wnd->clear(sf::Color(40, 40, 50));

  
    sf::Vertex linea[] = {
        sf::Vertex(sf::Vector2f(pelotaFija->GetPosition().x, pelotaFija->GetPosition().y), sf::Color::White),
        sf::Vertex(sf::Vector2f(pelotaDinamica->GetPosition().x, pelotaDinamica->GetPosition().y), sf::Color::White)
    };
    wnd->draw(linea, 2, sf::Lines);

    
    sf::CircleShape pelota(20.0f);
    pelota.setOrigin(20.0f, 20.0f);

    pelota.setFillColor(sf::Color(100, 255, 100));
    pelota.setPosition(pelotaFija->GetPosition().x, pelotaFija->GetPosition().y);
    wnd->draw(pelota);

    pelota.setFillColor(sf::Color(255, 100, 100)); 
    pelota.setPosition(pelotaDinamica->GetPosition().x, pelotaDinamica->GetPosition().y);
    wnd->draw(pelota);
}

void Game::UpdatePhysics() {
    phyWorld->Step(1.0f / 60.0f, 8, 8);
}

void Game::DoEvents() {
    sf::Event evt;
    while (wnd->pollEvent(evt)) {
        if (evt.type == sf::Event::Closed)
            wnd->close();

        
        if (evt.type == sf::Event::MouseButtonPressed && evt.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(evt.mouseButton.x, evt.mouseButton.y));

            
            b2Vec2 posVerde = pelotaFija->GetPosition();
            if (sqrt(pow(mousePos.x - posVerde.x, 2) + pow(mousePos.y - posVerde.y, 2)) <= 20.0f) {
                pelotaSeleccionada = pelotaFija;
                arrastrando = true;
            }
            
            else {
                b2Vec2 posRoja = pelotaDinamica->GetPosition();
                if (sqrt(pow(mousePos.x - posRoja.x, 2) + pow(mousePos.y - posRoja.y, 2)) <= 20.0f) {
                    pelotaSeleccionada = pelotaDinamica;
                    arrastrando = true;
                    
                    pelotaDinamica->SetType(b2_staticBody);
                }
            }
        }

       
        if (evt.type == sf::Event::MouseButtonReleased && evt.mouseButton.button == sf::Mouse::Left) {
            if (arrastrando && pelotaSeleccionada) {
               
                if (pelotaSeleccionada == pelotaDinamica) {
                    pelotaDinamica->SetType(b2_dynamicBody);
                }
                pelotaSeleccionada = nullptr;
                arrastrando = false;
            }
        }

        
        if (arrastrando && pelotaSeleccionada && evt.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(evt.mouseMove.x, evt.mouseMove.y));
            pelotaSeleccionada->SetTransform(b2Vec2(mousePos.x, mousePos.y), 0.0f);
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