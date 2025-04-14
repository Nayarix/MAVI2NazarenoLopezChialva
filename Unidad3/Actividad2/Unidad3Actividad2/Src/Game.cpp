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
    jointDef.minLength = 100.0f;  
    jointDef.maxLength = 300.0f;  
    jointDef.collideConnected = true;

    
    jointDef.stiffness = 5.0f;   
    jointDef.damping = 0.5f;      

    resorte = phyWorld->CreateJoint(&jointDef);
}

void Game::InitPhysics() {
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));  
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit);
    phyWorld->SetDebugDraw(debugRender);

    
    pelotas.push_back(Box2DHelper::CreateCircularDynamicBody(phyWorld, 20.0f, 1.0f, 0.3f, 0.7f));
    pelotas[0]->SetTransform(b2Vec2(300.0f, 200.0f), 0.0f);

    pelotas.push_back(Box2DHelper::CreateCircularDynamicBody(phyWorld, 20.0f, 1.0f, 0.3f, 0.7f));
    pelotas[1]->SetTransform(b2Vec2(500.0f, 200.0f), 0.0f);

    
    CrearResorte(pelotas[0], pelotas[1]);

    
    float grosor = 20.0f;
    int anchoPantalla = wnd->getSize().x;
    int altoPantalla = wnd->getSize().y;

    
    b2Body* bordeSup = Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor);
    bordeSup->SetTransform(b2Vec2(anchoPantalla / 2.0f, -grosor / 2.0f), 0.0f);

    
    b2Body* bordeInf = Box2DHelper::CreateRectangularStaticBody(phyWorld, anchoPantalla, grosor);
    bordeInf->SetTransform(b2Vec2(anchoPantalla / 2.0f, altoPantalla + grosor / 2.0f), 0.0f);

    
    b2Body* bordeIzq = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla);
    bordeIzq->SetTransform(b2Vec2(-grosor / 2.0f, altoPantalla / 2.0f), 0.0f);

   
    b2Body* bordeDer = Box2DHelper::CreateRectangularStaticBody(phyWorld, grosor, altoPantalla);
    bordeDer->SetTransform(b2Vec2(anchoPantalla + grosor / 2.0f, altoPantalla / 2.0f), 0.0f);
}

void Game::DrawGame() {
    wnd->clear(sf::Color(40, 40, 50));

    
    sf::Vertex linea[] = {
        sf::Vertex(sf::Vector2f(pelotas[0]->GetPosition().x, pelotas[0]->GetPosition().y), sf::Color::White),
        sf::Vertex(sf::Vector2f(pelotas[1]->GetPosition().x, pelotas[1]->GetPosition().y), sf::Color::White)
    };
    wnd->draw(linea, 2, sf::Lines);

    
    sf::CircleShape pelota(20.0f);
    pelota.setOrigin(20.0f, 20.0f);

    pelota.setFillColor(sf::Color(255, 100, 100));  
    pelota.setPosition(pelotas[0]->GetPosition().x, pelotas[0]->GetPosition().y);
    wnd->draw(pelota);

    pelota.setFillColor(sf::Color(100, 100, 255));  
    pelota.setPosition(pelotas[1]->GetPosition().x, pelotas[1]->GetPosition().y);
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

            for (b2Body* pelota : pelotas) {
                b2Vec2 pos = pelota->GetPosition();
                float distancia = sqrt(pow(mousePos.x - pos.x, 2) + pow(mousePos.y - pos.y, 2));

                if (distancia <= 20.0f) {  
                    pelotaSeleccionada = pelota;
                    arrastrando = true;
                    pelota->SetType(b2_staticBody);  
                    break;
                }
            }
        }

        if (evt.type == sf::Event::MouseButtonReleased && evt.mouseButton.button == sf::Mouse::Left) {
            if (arrastrando && pelotaSeleccionada) {
                pelotaSeleccionada->SetType(b2_dynamicBody);
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