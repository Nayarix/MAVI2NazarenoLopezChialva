#include "Game.h"

Game::Game(int ancho, int alto, std::string titulo) {
    wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
    arrastrando = false;
    cuerpoSeleccionado = nullptr;
    InitPhysics();
}

b2RevoluteJoint* Game::CrearJointRevolute(b2Body* cuerpoA, b2Body* cuerpoB, b2Vec2 anclaje) {
    b2RevoluteJointDef jointDef;
    jointDef.Initialize(cuerpoA, cuerpoB, anclaje);
    jointDef.collideConnected = false;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = -0.5f * b2_pi;
    jointDef.upperAngle = 0.5f * b2_pi;
    return (b2RevoluteJoint*)phyWorld->CreateJoint(&jointDef);
}

b2Body* Game::GetBodyAtPosition(float x, float y) {
    b2Vec2 punto(x, y);

    
    b2Fixture* fixture;
    b2Body* cuerpos[] = { cabeza, torso, brazoIzq, brazoDer, piernaIzq, piernaDer };

    for (b2Body* cuerpo : cuerpos) {
        for (fixture = cuerpo->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
            if (fixture->TestPoint(punto)) {
                return cuerpo;
            }
        }
    }
    return nullptr;
}

void Game::CrearRagdoll() {
    
    cabeza = Box2DHelper::CreateCircularDynamicBody(phyWorld, 15.0f, 0.8f, 0.2f, 0.1f);
    cabeza->SetTransform(b2Vec2(400.0f, 170.0f), 0.0f);

    torso = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 30.0f, 50.0f, 2.0f, 0.2f, 0.1f);
    torso->SetTransform(b2Vec2(400.0f, 220.0f), 0.0f);

    brazoIzq = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 40.0f, 12.0f, 0.6f, 0.2f, 0.1f);
    brazoIzq->SetTransform(b2Vec2(365.0f, 220.0f), 0.0f);

    brazoDer = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 40.0f, 12.0f, 0.6f, 0.2f, 0.1f);
    brazoDer->SetTransform(b2Vec2(435.0f, 220.0f), 0.0f);

    piernaIzq = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 12.0f, 40.0f, 1.0f, 0.2f, 0.1f);
    piernaIzq->SetTransform(b2Vec2(390.0f, 270.0f), 0.0f);

    piernaDer = Box2DHelper::CreateRectangularDynamicBody(phyWorld, 12.0f, 40.0f, 1.0f, 0.2f, 0.1f);
    piernaDer->SetTransform(b2Vec2(410.0f, 270.0f), 0.0f);

    
    cuello = CrearJointRevolute(torso, cabeza, b2Vec2(400.0f, 170.0f));
    hombroIzq = CrearJointRevolute(torso, brazoIzq, b2Vec2(380.0f, 220.0f));
    hombroDer = CrearJointRevolute(torso, brazoDer, b2Vec2(420.0f, 220.0f));
    caderaIzq = CrearJointRevolute(torso, piernaIzq, b2Vec2(400.0f, 245.0f));
    caderaDer = CrearJointRevolute(torso, piernaDer, b2Vec2(400.0f, 245.0f));
}

void Game::InitPhysics() {
    phyWorld = new b2World(b2Vec2(0.0f, 9.8f));
    debugRender = new SFMLRenderer(wnd);
    debugRender->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit);
    phyWorld->SetDebugDraw(debugRender);

    CrearRagdoll();

    
    b2Body* suelo = Box2DHelper::CreateRectangularStaticBody(phyWorld, 800.0f, 20.0f);
    suelo->SetTransform(b2Vec2(400.0f, 590.0f), 0.0f);
}

void Game::DrawGame() {
    wnd->clear(sf::Color(40, 40, 50));
    phyWorld->DebugDraw();

    
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        
        font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
    }

   
    sf::Text textoReiniciar;
    textoReiniciar.setFont(font);
    textoReiniciar.setString("Presiona R para reiniciar el personaje");
    textoReiniciar.setCharacterSize(20);
    textoReiniciar.setFillColor(sf::Color::White);
    textoReiniciar.setPosition(10.0f, 10.0f);

 
    sf::Text textoControl;
    textoControl.setFont(font);
    textoControl.setString("Haz click y arrastra cualquier parte del cuerpo para moverlo");
    textoControl.setCharacterSize(20);
    textoControl.setFillColor(sf::Color::White);
    textoControl.setPosition(10.0f, 40.0f);


    wnd->draw(textoReiniciar);
    wnd->draw(textoControl);
}

void Game::UpdatePhysics() {
    phyWorld->Step(1.0f / 60.0f, 8, 8);
}

void Game::DoEvents() {
    sf::Event evt;
    while (wnd->pollEvent(evt)) {
        if (evt.type == sf::Event::Closed) {
            wnd->close();
        }

        if (evt.type == sf::Event::MouseButtonPressed && evt.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(evt.mouseButton.x, evt.mouseButton.y));
            cuerpoSeleccionado = GetBodyAtPosition(mousePos.x, mousePos.y);

            if (cuerpoSeleccionado != nullptr) {
                arrastrando = true;
                puntoAnclaje = cuerpoSeleccionado->GetLocalPoint(b2Vec2(mousePos.x, mousePos.y));
            }
        }

        if (evt.type == sf::Event::MouseButtonReleased && evt.mouseButton.button == sf::Mouse::Left) {
            arrastrando = false;
            cuerpoSeleccionado = nullptr;
        }

        if (evt.type == sf::Event::MouseMoved && arrastrando && cuerpoSeleccionado != nullptr) {
            sf::Vector2f mousePos = wnd->mapPixelToCoords(sf::Vector2i(evt.mouseMove.x, evt.mouseMove.y));
            b2Vec2 posMundo(mousePos.x, mousePos.y);
            b2Vec2 posCuerpo = cuerpoSeleccionado->GetWorldPoint(puntoAnclaje);
            b2Vec2 fuerza = 5000.0f * (posMundo - posCuerpo);

            cuerpoSeleccionado->ApplyForce(fuerza, posCuerpo, true);
        }

        if (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::R) {
            phyWorld->DestroyJoint(cuello);
            phyWorld->DestroyJoint(hombroIzq);
            phyWorld->DestroyJoint(hombroDer);
            phyWorld->DestroyJoint(caderaIzq);
            phyWorld->DestroyJoint(caderaDer);

            phyWorld->DestroyBody(cabeza);
            phyWorld->DestroyBody(torso);
            phyWorld->DestroyBody(brazoIzq);
            phyWorld->DestroyBody(brazoDer);
            phyWorld->DestroyBody(piernaIzq);
            phyWorld->DestroyBody(piernaDer);

            CrearRagdoll();
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