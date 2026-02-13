#include "Game.h"

Game::Game(int width, int height, const std::string& title) {
    window = new sf::RenderWindow(sf::VideoMode(width, height), title);
    window->setFramerateLimit(60);

    physicsWorld = new b2World(b2Vec2(0.0f, 9.8f));

    debugRenderer = new SFMLRenderer(window);
    debugRenderer->SetFlags(0);
    physicsWorld->SetDebugDraw(debugRenderer);

    
    cannonBase.setSize(sf::Vector2f(30.0f, 30.0f));
    cannonBase.setOrigin(15.0f, 15.0f);
    cannonBase.setPosition(30.0f, height - 30.0f);
    cannonBase.setFillColor(sf::Color(150, 150, 150));

    cannonBarrel.setSize(sf::Vector2f(CANNON_LENGTH, 10.0f));
    cannonBarrel.setOrigin(0.0f, 5.0f);
    cannonBarrel.setPosition(30.0f, height - 30.0f);
    cannonBarrel.setFillColor(sf::Color(200, 50, 50));

    if (!texturaCaja.loadFromFile("imagen.png")) {
        // Manejo de error (opcional: pintar un cuadrado rojo si falla)
    }

    if (!texturaPlataforma.loadFromFile("plataforma.png")) {
        // Manejo de error (opcional: pintar un cuadrado azul si falla)
	}
    
    CreateBoundaries(width, height);
    CreateObstacles();

    estadoActual = GameState::MAIN_MENU;
    InicializarBotones();


}

Ragdoll::Ragdoll(b2World* world, const b2Vec2& position) {
    
    cabeza = Box2DHelper::CreateCircularDynamicBody(world, 0.3f, 0.8f, 0.2f, 0.1f);
    cabeza->SetTransform(position + b2Vec2(0.0f, -0.7f), 0.0f);

    torso = Box2DHelper::CreateRectangularDynamicBody(world, 0.5f, 0.8f, 5.0f, 0.2f, 0.1f);
    torso->SetTransform(position, 0.0f);

    brazoIzq = Box2DHelper::CreateRectangularDynamicBody(world, 0.6f, 0.2f, 0.6f, 0.2f, 0.1f);
    brazoIzq->SetTransform(position + b2Vec2(-0.55f, 0.0f), 0.0f);

    brazoDer = Box2DHelper::CreateRectangularDynamicBody(world, 0.6f, 0.2f, 0.6f, 0.2f, 0.1f);
    brazoDer->SetTransform(position + b2Vec2(0.55f, 0.0f), 0.0f);

    piernaIzq = Box2DHelper::CreateRectangularDynamicBody(world, 0.2f, 0.6f, 1.0f, 0.2f, 0.1f);
    piernaIzq->SetTransform(position + b2Vec2(-0.2f, 0.7f), 0.0f);

    piernaDer = Box2DHelper::CreateRectangularDynamicBody(world, 0.2f, 0.6f, 1.0f, 0.2f, 0.1f);
    piernaDer->SetTransform(position + b2Vec2(0.2f, 0.7f), 0.0f);

    
    auto CreateFlexibleJoint = [world](b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchorA, const b2Vec2& anchorB, float stiffness, float damping) {
        b2DistanceJointDef jointDef;
        jointDef.Initialize(bodyA, bodyB,
            bodyA->GetWorldPoint(anchorA),
            bodyB->GetWorldPoint(anchorB));
        jointDef.collideConnected = true;
        jointDef.stiffness = stiffness;
        jointDef.damping = damping;
        return world->CreateJoint(&jointDef);
        };

 
    auto CreateHingeJoint = [world](b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor) {
        b2RevoluteJointDef jointDef;
        jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldPoint(anchor));
        jointDef.collideConnected = false; // Importante: que la pierna no choque con el torso
        // Opcional: jointDef.enableLimit = true; // Para limitar el giro de la rodilla
        return world->CreateJoint(&jointDef);
        };  


    // Lambda para Soldaduras (Weld Joint)
    auto CreateHardWeld = [world](b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor) {
        b2WeldJointDef jointDef;
        // Initialize pide: Quién A, Quién B, y DÓNDE se pegan (punto mundial)
        jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldPoint(anchor));

        jointDef.collideConnected = false; // No chocar entre ellos

        return world->CreateJoint(&jointDef);
        };



    cuello = CreateFlexibleJoint(torso, cabeza, b2Vec2(0.0f, -0.4f), b2Vec2(0.0f, 0.3f), 0.1f, 0.0f);
    hombroIzq = CreateFlexibleJoint(torso, brazoIzq, b2Vec2(-0.25f, 0.0f), b2Vec2(0.3f, 0.0f), 2.0f, 0.2f);
    hombroDer = CreateFlexibleJoint(torso, brazoDer, b2Vec2(0.25f, 0.0f), b2Vec2(-0.3f, 0.0f), 2.0f, 0.2f);
    // Usamos la bisagra. El vector es la posición RELATIVA en el Torso donde va la pierna.
    caderaIzq = CreateHingeJoint(torso, piernaIzq, b2Vec2(-0.1f, 0.4f));
    caderaDer = CreateHingeJoint(torso, piernaDer, b2Vec2(0.1f, 0.4f));


    // --- CREACIÓN DEL ESCUDO ---

// 1. Crear el cuerpo físico del escudo
// Mide 0.2m de ancho x 0.8m de alto (Un rectángulo alto)
// Densidad 5.0f (Muy pesado, para que parezca metal y proteja)
    escudo = Box2DHelper::CreateRectangularDynamicBody(world, 0.2f, 0.2f, 1.0f, 0.5f, 0.1f);

    // 2. Posicionarlo inicialmente
    // Lo ponemos justo donde está el brazo izquierdo para que no "salte" al crearse
    escudo->SetTransform(brazoIzq->GetPosition(), 0.0f);

	//Para cambiar la posicion del objeto:
    //b2Vec2 posicionMano = brazoIzq->GetWorldPoint(b2Vec2(0.0f, 1.0f));
    //escudo->SetTransform(posicionMano, 0.0f);

    // 3. SOLDARLO
    // Anchor (0.0f, 0.5f):
    // X=0.0 (Centro del brazo a lo ancho)
    // Y=0.5 (En el borde inferior del brazo, hacia la mano)
    CreateHardWeld(brazoIzq, escudo, b2Vec2(0.0f, 0.1f));
}

void Ragdoll::ApplyImpulse(const b2Vec2& impulse) {
    //cabeza->ApplyLinearImpulse(impulse, cabeza->GetWorldCenter(), true);
    b2Vec2 impulsePotente(impulse.x * 1.5f, impulse.y * 1.5f);
    torso->ApplyLinearImpulse(impulsePotente, torso->GetWorldCenter(), true);
    //brazoIzq->ApplyLinearImpulse(impulse, brazoIzq->GetWorldCenter(), true);
    //brazoDer->ApplyLinearImpulse(impulse, brazoDer->GetWorldCenter(), true);
    //piernaIzq->ApplyLinearImpulse(impulse, piernaIzq->GetWorldCenter(), true);
    //piernaDer->ApplyLinearImpulse(impulse, piernaDer->GetWorldCenter(), true);
}

Obstaculo::Obstaculo(b2World* world, const b2Vec2& position, float width, float height, bool estatico, sf::Texture& textura)
    :esEstatico(estatico) {
    

    b2BodyDef bodyDef;
    bodyDef.type = estatico ? b2_staticBody : b2_dynamicBody;
    bodyDef.position = position;
    body = world->CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(width / 2, height / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = estatico ? 0.0f : 0.66f;
    fixtureDef.friction = 0.4f;
    fixtureDef.restitution = 0.2f;

    body->CreateFixture(&fixtureDef);

    spriteVisual.setTexture(textura);

    sf::FloatRect bounds = spriteVisual.getLocalBounds();
    spriteVisual.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

    float anchoEnPixeles = width * 30.0f;
    float altoEnPixeles = height * 30.0f;

    spriteVisual.setScale(anchoEnPixeles / bounds.width, altoEnPixeles / bounds.height);

    if (!estatico) {
        body->SetAwake(false);
    }
}

void Obstaculo::Draw(sf::RenderWindow* window) const {
    // 1. Preguntar a la física dónde está
    b2Vec2 posicionFisica = body->GetPosition();
    float anguloFisico = body->GetAngle();

    // 2. Mover el Sprite (que ya tiene la textura desde el constructor)
    // NOTA: 'spriteVisual' es mutable aunque el método sea const en SFML, 
    // pero si te da error de const, quita el 'const' de la función Draw.
    // Lo correcto en C++ purista sería usar 'mutable spriteVisual' en el header
    // o crear un sprite temporal copiado del original.

    // TRUCO RÁPIDO: Copiamos el sprite configurado para dibujarlo en esta posición
    sf::Sprite spriteTemp = spriteVisual;

    spriteTemp.setPosition(posicionFisica.x * 30.0f, posicionFisica.y * 30.0f);
    spriteTemp.setRotation(anguloFisico * 180.0f / b2_pi);

    // 3. Dibujar
    window->draw(spriteTemp);
}

void Game::CreateBoundaries(float width, float height) {
    float worldWidth = width / PIXELS_TO_METERS;
    float worldHeight = height / PIXELS_TO_METERS;
    float wallThickness = 1.0f;

    b2BodyDef wallDef;
    wallDef.type = b2_staticBody;

   
    b2Body* leftWall = physicsWorld->CreateBody(&wallDef);
    b2PolygonShape leftShape;
    leftShape.SetAsBox(wallThickness, worldHeight, b2Vec2(-wallThickness, worldHeight / 2), 0);
    leftWall->CreateFixture(&leftShape, 0.0f);
    boundaryWalls.push_back(leftWall);

    b2Body* rightWall = physicsWorld->CreateBody(&wallDef);
    b2PolygonShape rightShape;
    rightShape.SetAsBox(wallThickness, worldHeight, b2Vec2(worldWidth + wallThickness, worldHeight / 2), 0);
    rightWall->CreateFixture(&rightShape, 0.0f);
    boundaryWalls.push_back(rightWall);

    b2Body* topWall = physicsWorld->CreateBody(&wallDef);
    b2PolygonShape topShape;
    topShape.SetAsBox(worldWidth, wallThickness, b2Vec2(worldWidth / 2, -wallThickness), 0);
    topWall->CreateFixture(&topShape, 0.0f);
    boundaryWalls.push_back(topWall);

    b2Body* bottomWall = physicsWorld->CreateBody(&wallDef);
    b2PolygonShape bottomShape;
    bottomShape.SetAsBox(worldWidth, wallThickness * 2, b2Vec2(worldWidth / 2, worldHeight + wallThickness * 2), 0);
    bottomWall->CreateFixture(&bottomShape, 0.0f);
    boundaryWalls.push_back(bottomWall);
}

void Game::CreateObstacles() {
   
    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(10.0f, 5.0f), 3.0f, 0.5f, true, texturaPlataforma));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(20.0f, 8.0f), 0.5f, 3.0f, true, texturaPlataforma));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(15.0f, 10.0f), 0.5f, 3.0f, true, texturaPlataforma));
    
    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(15.0f, 5.0f), 1.0f, 1.0f, false, texturaCaja));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(18.0f, 5.0f), 1.0f, 1.0f, false, texturaCaja));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(12.0f, 3.0f), 1.5f, 0.5f, false, texturaCaja));
}

float Game::CalculatePower(const sf::Vector2f& mousePos) {
    sf::Vector2f cannonPos = cannonBarrel.getPosition();
    float distance = sqrt(pow(mousePos.x - cannonPos.x, 2) + pow(mousePos.y - cannonPos.y, 2));
    float normalizedDistance = std::min(distance / MAX_DISTANCE, 1.0f);
    float power = MIN_CANNON_POWER + (MAX_CANNON_POWER - MIN_CANNON_POWER) * normalizedDistance;
    return std::min(power, MAX_CANNON_POWER);
}

void Game::DrawBody(b2Body* body, const sf::Color& color) {
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture) {
        if (fixture->GetType() == b2Shape::e_circle) {
            b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
            sf::CircleShape sfCircle(circle->m_radius * PIXELS_TO_METERS);
            sfCircle.setOrigin(sfCircle.getRadius(), sfCircle.getRadius());
            b2Vec2 pos = body->GetPosition();
            sfCircle.setPosition(pos.x * PIXELS_TO_METERS, pos.y * PIXELS_TO_METERS);
            sfCircle.setFillColor(color);
            window->draw(sfCircle);
        }
        else if (fixture->GetType() == b2Shape::e_polygon) {
            b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
            sf::ConvexShape convex;
            convex.setPointCount(poly->m_count);
            for (int i = 0; i < poly->m_count; i++) {
                b2Vec2 vertex = poly->m_vertices[i];
                convex.setPoint(i, sf::Vector2f(vertex.x * PIXELS_TO_METERS, vertex.y * PIXELS_TO_METERS));
            }
            b2Vec2 pos = body->GetPosition();
            float angle = body->GetAngle();
            convex.setPosition(pos.x * PIXELS_TO_METERS, pos.y * PIXELS_TO_METERS);
            convex.setRotation(angle * 180.0f / b2_pi);
            convex.setFillColor(color);
            window->draw(convex);
        }
        fixture = fixture->GetNext();
    }
}

void Game::FireRagdoll() {
    float angle = cannonBarrel.getRotation() * b2_pi / 180.0f;
    sf::Vector2f barrelEnd = cannonBarrel.getPosition() +
        sf::Vector2f(cos(angle), sin(angle)) * CANNON_LENGTH;

    b2Vec2 spawnPos(barrelEnd.x / PIXELS_TO_METERS, barrelEnd.y / PIXELS_TO_METERS);

    auto newRagdoll = std::make_unique<Ragdoll>(physicsWorld, spawnPos);

    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*window));
    float power = CalculatePower(mousePos);

    b2Vec2 force(cos(angle) * power, sin(angle) * power);
    newRagdoll->ApplyImpulse(force);

    ragdolls.push_back(std::move(newRagdoll));
}

void Game::UpdateCannonRotation() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    sf::Vector2f cannonPos = cannonBarrel.getPosition();
    sf::Vector2f direction = sf::Vector2f(mousePos) - cannonPos;
    float angle = atan2f(direction.y, direction.x);
    cannonBarrel.setRotation(angle * 180.0f / b2_pi);
}

void Game::HandleEvents() {
    sf::Event event;
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);

    while (window->pollEvent(event)) {
        if (event.type == sf::Event::Closed) window->close();

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

            if (estadoActual == GameState::MAIN_MENU) {
                if (EsClicEnBoton(btnNiveles, mousePos)) estadoActual = GameState::LEVEL_SELECT;
                if (EsClicEnBoton(btnGuia, mousePos)) estadoActual = GameState::RAGDOLL_GUIDE;
                if (EsClicEnBoton(btnCerrar, mousePos)) window->close();
            }
            else if (estadoActual == GameState::LEVEL_SELECT) {
                if (EsClicEnBoton(btnLvl1, mousePos)) { CargarNivel(1); estadoActual = GameState::PLAYING; }
                if (EsClicEnBoton(btnLvl2, mousePos)) { CargarNivel(2); estadoActual = GameState::PLAYING; }
                if (EsClicEnBoton(btnLvl3, mousePos)) { CargarNivel(3); estadoActual = GameState::PLAYING; }
                if (EsClicEnBoton(btnVolver, mousePos)) estadoActual = GameState::MAIN_MENU;
            }
            else if (estadoActual == GameState::RAGDOLL_GUIDE) {
                estadoActual = GameState::MAIN_MENU; // Cualquier clic vuelve al menú
            }
            else if (estadoActual == GameState::PLAYING) {
                if (EsClicEnBoton(btnMenuPrincipal, mousePos)) {
                    estadoActual = GameState::MAIN_MENU;
                }
                else {
                    FireRagdoll(); // Solo dispara si NO tocó el botón de menú
                }
            }
        }
    }
}

void Game::Update(float deltaTime) {
    physicsWorld->Step(deltaTime, 8, 3);
    UpdateCannonRotation();
}

void Game::Render() {
    window->clear(sf::Color(20, 20, 20)); // Fondo oscuro



    if (estadoActual == GameState::MAIN_MENU) {
        window->draw(btnNiveles.forma); window->draw(btnNiveles.texto);
        window->draw(btnGuia.forma);    window->draw(btnGuia.texto);
        window->draw(btnCerrar.forma);  window->draw(btnCerrar.texto);
    }
    else if (estadoActual == GameState::LEVEL_SELECT) {
        window->draw(btnLvl1.forma);   window->draw(btnLvl1.texto);
        window->draw(btnLvl2.forma);   window->draw(btnLvl2.texto);
        window->draw(btnLvl3.forma);   window->draw(btnLvl3.texto);
        window->draw(btnVolver.forma); window->draw(btnVolver.texto);
    }
    else if (estadoActual == GameState::RAGDOLL_GUIDE) {
        sf::Text guia("GUIA:\n1. Apunta con el Mouse\n2. Clic para disparar\n3. No dejes que los Ragdolls se acumulen\n\nClic para volver", fuente, 20);
        guia.setPosition(200, 200);
        window->draw(guia);
    }
    else if (estadoActual == GameState::PLAYING) {
        // 1. Dibujar el cañón y obstáculos
        window->draw(cannonBase);
        window->draw(cannonBarrel);
        for (auto& obs : obstaculos) obs->Draw(window);

        // 2. DIBUJAR RAGDOLLS COMPLETOS
        for (auto& rag : ragdolls) {
            DrawBody(rag->cabeza, sf::Color::Green);
            DrawBody(rag->torso, sf::Color::Blue);
            DrawBody(rag->brazoIzq, sf::Color::Red);
            DrawBody(rag->brazoDer, sf::Color::Red);
            DrawBody(rag->piernaIzq, sf::Color::Yellow);
            DrawBody(rag->piernaDer, sf::Color::Yellow);

            // Dibujar el escudo si existe
            if (rag->escudo) {
                DrawBody(rag->escudo, sf::Color(100, 100, 100)); // Gris metalizado
            }
        }

        // 3. Dibujar la interfaz del juego (Botón menú y contador si lo tienes)
        window->draw(btnMenuPrincipal.forma);
        window->draw(btnMenuPrincipal.texto);
    }

    window->display();

}

Game::~Game() {
    delete physicsWorld;
    delete debugRenderer;
    delete window;
}

void Game::Loop() {
    sf::Clock clock;
    while (window->isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        HandleEvents();
        Update(deltaTime);
        Render();
    }
}

bool Game::EsClicEnBoton(const Boton& b, const sf::Vector2i& mousePos) {
    return b.forma.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

void Game::InicializarBotones() {
    // 1. Cargar Fuente (Asegúrate de tener el archivo .ttf en la carpeta)
    if (!fuente.loadFromFile("font.ttf")) {
        // Manejar error
    }

    // Función auxiliar para configurar botones rápido
    auto ConfigurarBoton = [&](Boton& b, std::string texto, sf::Vector2f pos) {
        b.forma.setSize(sf::Vector2f(250, 50));
        b.forma.setOrigin(125, 25);
        b.forma.setPosition(pos);
        b.forma.setFillColor(sf::Color(70, 70, 70));
        b.forma.setOutlineThickness(2);
        b.forma.setOutlineColor(sf::Color::White);

        b.texto.setFont(fuente);
        b.texto.setString(texto);
        b.texto.setCharacterSize(20);
        b.texto.setFillColor(sf::Color::White);
        // Centrar texto en el botón
        sf::FloatRect textRect = b.texto.getLocalBounds();
        b.texto.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        b.texto.setPosition(pos);
        };

    // --- Botones Menú Principal ---
    ConfigurarBoton(btnNiveles, "SELECCIONAR NIVELES", { 400, 250 });
    ConfigurarBoton(btnGuia, "GUIA DE RAGDOLLS", { 400, 320 });
    ConfigurarBoton(btnCerrar, "SALIR", { 400, 390 });

    // --- Botones Selección de Nivel ---
    ConfigurarBoton(btnLvl1, "NIVEL 1 (FACIL)", { 400, 200 });
    ConfigurarBoton(btnLvl2, "NIVEL 2 (MEDIO)", { 400, 270 });
    ConfigurarBoton(btnLvl3, "NIVEL 3 (DIFICIL)", { 400, 340 });
    ConfigurarBoton(btnVolver, "VOLVER", { 400, 450 });

    // --- Botón Durante el Juego ---
    ConfigurarBoton(btnMenuPrincipal, "MENU", { 740, 30 });
    btnMenuPrincipal.forma.setSize(sf::Vector2f(80, 40));
    btnMenuPrincipal.forma.setOrigin(40, 20);
    btnMenuPrincipal.texto.setCharacterSize(15);
}

void Game::CargarNivel(int num) {
    LimpiarMundo(); // <--- FUNDAMENTAL para que no haya fantasmas

    nivelSeleccionado = num;

    if (num == 1) {
        // Objetos Estáticos (Gris en el original)
  LimpiarMundo(); // Limpiamos nivel anterior

    // 1. EL EJE (Cuerpo estático invisible que sostiene todo)
    b2BodyDef anchorDef;
    anchorDef.position.Set(10.0f, 13.0f); // Centro de la rueda en el mundo
    b2Body* ejeInmóvil = physicsWorld->CreateBody(&anchorDef);

	b2BodyDef anchorDef2;
	anchorDef2.position.Set(17.0f, 10.0f); // Posición de la rueda 2 fija
	b2Body* ejeInmóvil2 = physicsWorld->CreateBody(&anchorDef2);


    // 2. LA RUEDA (Círculo dinámico)
    // Creamos un obstáculo circular (ajusta el tamaño si es necesario)
    auto rueda = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(10.0f, 13.0f), 4.0f, 4.0f, false, texturaPlataforma);

	auto rueda2 = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(17.0f, 10.0f), 4.0f, 4.0f, false, texturaPlataforma);
    
    // 3. EL BLOQUE SOBRESALIENTE (Rectángulo dinámico)
    // Lo posicionamos un poco a la derecha del centro de la rueda
    auto bloque = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(7.0f, 13.0f), 3.0f, 0.8f, false, texturaCaja);

	auto bloque2 = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(14.0f, 10.0f), 3.0f, 0.8f, false, texturaCaja);

    // --- CONFIGURACIÓN FÍSICA ---

    // Quitamos la gravedad para que no gire por su propio peso
    rueda->body->SetGravityScale(0.0f);
    bloque->body->SetGravityScale(0.0f);

	rueda2->body->SetGravityScale(0.0f);
	bloque2->body->SetGravityScale(0.0f);

    // 4. SOLDAR EL BLOQUE A LA RUEDA (Weld Joint)
    // Esto hace que se muevan como una sola pieza
    b2WeldJointDef weldDef;
    weldDef.Initialize(rueda->body, bloque->body, rueda->body->GetWorldCenter());
    physicsWorld->CreateJoint(&weldDef);

	b2WeldJointDef weldDef2;
	weldDef2.Initialize(rueda2->body, bloque2->body, rueda2->body->GetWorldCenter());
	physicsWorld->CreateJoint(&weldDef2);

    // 5. UNIR LA RUEDA AL EJE (Revolute Joint)
    // Esto permite que rote sobre el punto fijo
    b2RevoluteJointDef revDef;
    revDef.Initialize(ejeInmóvil, rueda->body, ejeInmóvil->GetWorldCenter());

	b2RevoluteJointDef revDef2;
	revDef2.Initialize(ejeInmóvil2, rueda2->body, ejeInmóvil2->GetWorldCenter());
    
    // Opcional: Si quieres que tenga un poco de resistencia al giro (fricción)
    // revDef.enableMotor = true;
    // revDef.maxMotorTorque = 5.0f;
    // revDef.motorSpeed = 0.0f; 

    physicsWorld->CreateJoint(&revDef);
    physicsWorld->CreateJoint(&revDef2);

    // 6. GUARDAR PARA RENDERIZAR
    obstaculos.push_back(std::move(rueda));
    obstaculos.push_back(std::move(bloque));

	obstaculos.push_back(std::move(rueda2));
	obstaculos.push_back(std::move(bloque2));

    obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(13.5, 3), 1.5, 6, true, texturaPlataforma));

    obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(20, 6), 2, 2, false, texturaCaja));


    }
    else if (num == 2) {
        // 1. Cuerpo de anclaje
        b2BodyDef anchorDef;
        anchorDef.position.Set(0, 0);
        b2Body* sueloFijo = physicsWorld->CreateBody(&anchorDef);

        // 2. Plataformas (Dinámicas, con poca densidad para que el Ragdoll las mueva)
        auto obsA = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(7.0f, 10.0f), 4.0f, 0.4f, false, texturaPlataforma);
        auto obsB = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19.0f, 10.0f), 4.0f, 0.4f, false, texturaPlataforma);

        obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(13, 10), 2, 20, false, texturaPlataforma));
        obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19.0f, 5.0f), 2.0f, 2.0f, false, texturaCaja));
        obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19.0f, 15.0f), 2.0f, 2.0f, false, texturaCaja));


        // 3. Rieles Verticales (Prismatic Joints)
        b2PrismaticJointDef pDefA;
        pDefA.Initialize(sueloFijo, obsA->body, obsA->body->GetWorldCenter(), b2Vec2(0, 1));
        pDefA.lowerTranslation = -4.0f;
        pDefA.upperTranslation = 4.0f;
        pDefA.enableLimit = true;
        b2PrismaticJoint* jointIzquierdo = (b2PrismaticJoint*)physicsWorld->CreateJoint(&pDefA);

        b2PrismaticJointDef pDefB;
        pDefB.Initialize(sueloFijo, obsB->body, obsB->body->GetWorldCenter(), b2Vec2(0, 1));
        pDefB.lowerTranslation = -5.0f;
        pDefB.upperTranslation = 5.0f;
        pDefB.enableLimit = true;
        b2PrismaticJoint* jointDerecho = (b2PrismaticJoint*)physicsWorld->CreateJoint(&pDefB);

        // 4. EL ENGRANAJE (Ratio 1.0 para movimiento invertido)
        b2GearJointDef gearDef;
        gearDef.bodyA = obsA->body;
        gearDef.bodyB = obsB->body;
        gearDef.joint1 = jointIzquierdo;
        gearDef.joint2 = jointDerecho;
        gearDef.ratio = 1.0f; // <--- CAMBIADO A 1.0f
        physicsWorld->CreateJoint(&gearDef);

        obstaculos.push_back(std::move(obsA));
        obstaculos.push_back(std::move(obsB));

        
       
    }
    else if (num == 3) {
        // Nivel Difícil: Muchos obstáculos y plataformas pequeñas
        for (int i = 0; i < 5; i++)
            obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(10 + i * 3, 5), 0.5, 2, true, texturaCaja));
    }
}

void Game::LimpiarMundo() {
    // 1. Destruir cuerpos de Ragdolls
    for (auto& rag : ragdolls) {
        physicsWorld->DestroyBody(rag->cabeza);
        physicsWorld->DestroyBody(rag->torso);
        physicsWorld->DestroyBody(rag->brazoIzq);
        physicsWorld->DestroyBody(rag->brazoDer);
        physicsWorld->DestroyBody(rag->piernaIzq);
        physicsWorld->DestroyBody(rag->piernaDer);
        if (rag->escudo) physicsWorld->DestroyBody(rag->escudo);
    }
    ragdolls.clear();

    // 2. Destruir cuerpos de Obstáculos
    for (auto& obs : obstaculos) {
        physicsWorld->DestroyBody(obs->body);
    }
    obstaculos.clear();
}