#include <Box2D/Box2D.h>
#pragma once

// ============================================================
// CLASE Box2DHelper: Automatización de creación de cuerpos
// Provee métodos estáticos para simplificar el flujo de Box2D
// ============================================================
class Box2DHelper
{
public:

	// --- 1. CREACIÓN DE CUERPOS BASE (SIN FORMA) ---

	// Crea un cuerpo dinámico: se mueve por fuerzas, gravedad y colisiones
	static b2Body* CreateDynamicBody(b2World* phyWorld) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		return phyWorld->CreateBody(&bodyDef);
	}

	// Crea un cuerpo estático: no se mueve nunca (suelos, paredes)
	static b2Body* CreateStaticBody(b2World* phyWorld)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.position.Set(0.0f, 0.0f);
		return phyWorld->CreateBody(&bodyDef);
	}

	// Crea un cuerpo cinemático: se mueve por velocidad programada, no por fuerzas
	static b2Body* CreateKinematicBody(b2World* phyWorld)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_kinematicBody;
		bodyDef.position.Set(0.0f, 0.0f);
		return phyWorld->CreateBody(&bodyDef);
	}

	// --- 2. DEFINICIÓN DE PROPIEDADES (FIXTURE DEFS) ---
	// La Fixture define la geometría y las propiedades físicas del material

	// Define una forma rectangular (Caja)
	static b2FixtureDef CreateRectangularFixtureDef(float sizeX, float sizeY, float density, float friction, float restitution)
	{
		b2PolygonShape* box = new b2PolygonShape();
		// SetAsBox toma la mitad del ancho y alto desde el centro
		box->SetAsBox(sizeX / 2.0f, sizeY / 2.0f, b2Vec2(0.0f, 0.0f), 0.0f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = box;
		fixtureDef.density = density;      // Masa por área
		fixtureDef.friction = friction;    // Resistencia al deslizamiento
		fixtureDef.restitution = restitution; // Rebote (0 = nada, 1 = elástico total)

		return fixtureDef;
	}

	// Define una forma circular
	static b2FixtureDef CreateCircularFixtureDef(float radius, float density, float friction, float restitution)
	{
		b2CircleShape* circle = new b2CircleShape();
		circle->m_p = b2Vec2(0.0f, 0.0f);
		circle->m_radius = radius;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = circle;
		fixtureDef.density = density;
		fixtureDef.friction = friction;
		fixtureDef.restitution = restitution;

		return fixtureDef;
	}

	// Define una forma de polígono irregular mediante vértices
	static b2FixtureDef CreatePolyFixtureDef(b2Vec2* v, int n, float density, float friction, float restitution)
	{
		b2PolygonShape* poly = new b2PolygonShape();
		poly->Set(v, n); // 'v' es el array de vértices, 'n' la cantidad

		b2FixtureDef fixtureDef;
		fixtureDef.shape = poly;
		fixtureDef.density = density;
		fixtureDef.friction = friction;
		fixtureDef.restitution = restitution;

		return fixtureDef;
	}

	// Define una forma triangular básica
	static b2FixtureDef CreateTriangularFixtureDef(b2Vec2 c, float h, float density, float friction, float restitution)
	{
		b2Vec2 verts[3];
		verts[0].x = -h / 2; verts[0].y = -h / 2;
		verts[1].x = h / 2;  verts[1].y = -h / 2;
		verts[2].x = 0;      verts[2].y = h / 2;

		return CreatePolyFixtureDef(verts, 3, density, friction, restitution);
	}

	// --- 3. MÉTODOS COMPUESTOS (BODY + FIXTURE) ---
	// Estos métodos crean el cuerpo y le pegan la forma inmediatamente

	// Crea un Rectángulo Dinámico (usado para cajas y el torso del Ragdoll)
	static b2Body* CreateRectangularDynamicBody(b2World* phyWorld, float sizeX, float sizeY, float density, float friction, float restitution)
	{
		b2Body* body = CreateDynamicBody(phyWorld);
		b2FixtureDef box = CreateRectangularFixtureDef(sizeX, sizeY, density, friction, restitution);
		body->CreateFixture(&box);
		return body;
	}

	// Crea un Rectángulo Estático (usado para las paredes y límites)
	static b2Body* CreateRectangularStaticBody(b2World* phyWorld, float sizeX, float sizeY)
	{
		b2Body* body = CreateStaticBody(phyWorld);
		b2FixtureDef box = CreateRectangularFixtureDef(sizeX, sizeY, 0.0f, 0.0f, 0.0f);
		body->CreateFixture(&box);
		return body;
	}

	// Crea un Círculo Dinámico (usado para la cabeza del Ragdoll)
	static b2Body* CreateCircularDynamicBody(b2World* phyWorld, float radius, float density, float friction, float restitution)
	{
		b2Body* body = CreateDynamicBody(phyWorld);
		b2FixtureDef cir = CreateCircularFixtureDef(radius, density, friction, restitution);
		body->CreateFixture(&cir);
		return body;
	}

	// Crea un Triángulo Dinámico
	static b2Body* CreateTriangularDynamicBody(b2World* phyWorld, b2Vec2 center, float h, float density, float friction, float restitution)
	{
		b2Body* body = CreateDynamicBody(phyWorld);
		b2FixtureDef tri = CreateTriangularFixtureDef(center, h, density, friction, restitution);
		body->CreateFixture(&tri);
		return body;
	}

	// Crea un Polígono Dinámico personalizado
	static b2Body* CreatePolyDynamicBody(b2World* phyWorld, b2Vec2* v, int n, float density, float friction, float restitution)
	{
		b2Body* body = CreateDynamicBody(phyWorld);
		b2FixtureDef poly = CreatePolyFixtureDef(v, n, density, friction, restitution);
		body->CreateFixture(&poly);
		return body;
	}

	// (Nota: Los métodos Kinematic y Static siguen el mismo patrón de arriba)
};