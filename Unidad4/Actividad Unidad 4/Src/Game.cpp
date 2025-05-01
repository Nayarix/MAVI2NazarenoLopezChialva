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

    
    CreateBoundaries(width, height);
    CreateObstacles();
}

Ragdoll::Ragdoll(b2World* world, const b2Vec2& position) {
    
    cabeza = Box2DHelper::CreateCircularDynamicBody(world, 0.5f, 0.8f, 0.2f, 0.1f);
    cabeza->SetTransform(position + b2Vec2(0.0f, -1.0f), 0.0f);

    torso = Box2DHelper::CreateRectangularDynamicBody(world, 1.0f, 1.7f, 2.0f, 0.2f, 0.1f);
    torso->SetTransform(position, 0.0f);

    brazoIzq = Box2DHelper::CreateRectangularDynamicBody(world, 1.3f, 0.4f, 0.6f, 0.2f, 0.1f);
    brazoIzq->SetTransform(position + b2Vec2(-1.2f, 0.0f), 0.0f);

    brazoDer = Box2DHelper::CreateRectangularDynamicBody(world, 1.3f, 0.4f, 0.6f, 0.2f, 0.1f);
    brazoDer->SetTransform(position + b2Vec2(1.2f, 0.0f), 0.0f);

    piernaIzq = Box2DHelper::CreateRectangularDynamicBody(world, 0.4f, 1.3f, 1.0f, 0.2f, 0.1f);
    piernaIzq->SetTransform(position + b2Vec2(-0.3f, 1.7f), 0.0f);

    piernaDer = Box2DHelper::CreateRectangularDynamicBody(world, 0.4f, 1.3f, 1.0f, 0.2f, 0.1f);
    piernaDer->SetTransform(position + b2Vec2(0.3f, 1.7f), 0.0f);

    
    auto CreateFlexibleJoint = [world](b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchorA, const b2Vec2& anchorB) {
        b2DistanceJointDef jointDef;
        jointDef.Initialize(bodyA, bodyB,
            bodyA->GetWorldPoint(anchorA),
            bodyB->GetWorldPoint(anchorB));
        jointDef.length = 0.3f;
        jointDef.collideConnected = true;
        jointDef.stiffness = 5.0f;
        jointDef.damping = 0.5f;
        return world->CreateJoint(&jointDef);
        };

 
    cuello = CreateFlexibleJoint(torso, cabeza, b2Vec2(0.0f, -0.85f), b2Vec2(0.0f, 0.5f));
    hombroIzq = CreateFlexibleJoint(torso, brazoIzq, b2Vec2(-0.5f, 0.0f), b2Vec2(0.6f, 0.0f));
    hombroDer = CreateFlexibleJoint(torso, brazoDer, b2Vec2(0.5f, 0.0f), b2Vec2(-0.6f, 0.0f));
    caderaIzq = CreateFlexibleJoint(torso, piernaIzq, b2Vec2(-0.2f, 0.85f), b2Vec2(0.0f, -0.6f));
    caderaDer = CreateFlexibleJoint(torso, piernaDer, b2Vec2(0.2f, 0.85f), b2Vec2(0.0f, -0.6f));
}

void Ragdoll::ApplyImpulse(const b2Vec2& impulse) {
    cabeza->ApplyLinearImpulse(impulse, cabeza->GetWorldCenter(), true);
    torso->ApplyLinearImpulse(impulse, torso->GetWorldCenter(), true);
    brazoIzq->ApplyLinearImpulse(impulse, brazoIzq->GetWorldCenter(), true);
    brazoDer->ApplyLinearImpulse(impulse, brazoDer->GetWorldCenter(), true);
    piernaIzq->ApplyLinearImpulse(impulse, piernaIzq->GetWorldCenter(), true);
    piernaDer->ApplyLinearImpulse(impulse, piernaDer->GetWorldCenter(), true);
}

Obstaculo::Obstaculo(b2World* world, const b2Vec2& position, float width, float height, bool estatico, const sf::Color& col)
    : color(col), esEstatico(estatico) {

    b2BodyDef bodyDef;
    bodyDef.type = estatico ? b2_staticBody : b2_dynamicBody;
    bodyDef.position = position;
    body = world->CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(width / 2, height / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = estatico ? 0.0f : 1.0f;
    fixtureDef.friction = 0.4f;
    fixtureDef.restitution = 0.2f;

    body->CreateFixture(&fixtureDef);

    if (!estatico) {
        body->SetAwake(false);
    }
}

void Obstaculo::Draw(sf::RenderWindow* window, float pixelsToMeters) const {
    b2Fixture* fixture = body->GetFixtureList();
    if (fixture && fixture->GetType() == b2Shape::e_polygon) {
        b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
        sf::ConvexShape convex;
        convex.setPointCount(poly->m_count);

        for (int i = 0; i < poly->m_count; i++) {
            b2Vec2 vertex = poly->m_vertices[i];
            convex.setPoint(i, sf::Vector2f(vertex.x * pixelsToMeters, vertex.y * pixelsToMeters));
        }

        b2Vec2 pos = body->GetPosition();
        float angle = body->GetAngle();
        convex.setPosition(pos.x * pixelsToMeters, pos.y * pixelsToMeters);
        convex.setRotation(angle * 180.0f / b2_pi);
        convex.setFillColor(color);

        convex.setOutlineThickness(2.0f);
        convex.setOutlineColor(sf::Color(
            std::max(0, color.r - 50),
            std::max(0, color.g - 50),
            std::max(0, color.b - 50)
        ));

        window->draw(convex);
    }
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
        physicsWorld, b2Vec2(10.0f, 5.0f), 3.0f, 0.5f, true, sf::Color(150, 150, 150)));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(20.0f, 8.0f), 0.5f, 3.0f, true, sf::Color(150, 150, 150)));

    
    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(15.0f, 5.0f), 1.0f, 1.0f, false, sf::Color(255, 165, 0)));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(18.0f, 5.0f), 1.0f, 1.0f, false, sf::Color(255, 165, 0)));

    obstaculos.push_back(std::make_unique<Obstaculo>(
        physicsWorld, b2Vec2(12.0f, 3.0f), 1.5f, 0.5f, false, sf::Color(255, 165, 0)));
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
    while (window->pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window->close();

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            FireRagdoll();
        }
    }
}

void Game::Update(float deltaTime) {
    physicsWorld->Step(deltaTime, 8, 3);
    UpdateCannonRotation();
}

void Game::Render() {
    window->clear(sf::Color::Black);

  
    window->draw(cannonBase);
    window->draw(cannonBarrel);

  
    for (auto& obstaculo : obstaculos) {
        obstaculo->Draw(window, PIXELS_TO_METERS);
    }


    for (auto& ragdoll : ragdolls) {
        DrawBody(ragdoll->cabeza, sf::Color::Green);
        DrawBody(ragdoll->torso, sf::Color::Blue);
        DrawBody(ragdoll->brazoIzq, sf::Color::Red);
        DrawBody(ragdoll->brazoDer, sf::Color::Red);
        DrawBody(ragdoll->piernaIzq, sf::Color::Yellow);
        DrawBody(ragdoll->piernaDer, sf::Color::Yellow);
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