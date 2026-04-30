// ============================================================
// CLASE SFMLRenderer: Automatización de dibujo con SFML
// Provee métodos para simplificar el renderizado de objetos Box2D
// NO SE ESTA UTILIZANDO EN ESTE PROYECTO, PERO SE DEJA COMO REFERENCIA PARA FUTURAS MEJORAS
// LA BASE DE ESTE PROYECTO PROVENIA DE UN EJEMPLOPUBLICADO EN LA CATEDRA, PERO DECIDÍ HACER UN RENDERIZADO MANUAL PARA ENTENDER MEJOR CÓMO FUNCIONA LA INTEGRACIÓN ENTRE BOX2D Y SFML
// ============================================================




/*#pragma once
#include <Box2D/Box2D.h>
#include <SFML\Graphics.hpp>

using namespace sf;

class SFMLRenderer : public b2Draw
{
private:
	// Puntero a la ventana de SFML donde se dibujará la física
	RenderWindow* wnd;

public:
	// Constructor: Recibe la ventana donde se renderizará el DebugDraw
	SFMLRenderer(RenderWindow* window);

	// Destructor virtual
	~SFMLRenderer(void);

	// --- UTILIDADES ---
	// Convierte el formato de color de Box2D (0.0 a 1.0) al formato SFML (0 a 255)
	inline Color box2d2SFMLColor(const b2Color& _color);

	// --- MÉTODOS DE DIBUJO SOBRESCRITOS (OVERRIDE) ---
	// Estos métodos son llamados automáticamente por Box2D cuando se activa el DebugDraw

	// Dibuja el contorno de formas poligonales (cajas, triángulos, etc.)
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	// Dibuja formas poligonales rellenas
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	// Dibuja el contorno de un círculo (como la cabeza del Ragdoll)
	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;

	// Dibuja un círculo relleno indicando su eje de rotación
	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;

	// Dibuja una línea entre dos puntos (útil para ver las uniones o Joints)
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

	// Dibuja la posición y rotación (ejes) de un cuerpo dinámico
	void DrawTransform(const b2Transform& xf) override;

	// Dibuja un punto en una coordenada específica
	void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

	// --- MÉTODOS ADICIONALES ---

	// Permite dibujar texto de depuración en coordenadas específicas
	void DrawString(int x, int y, const char* string, ...);

	// Dibuja la "Caja de Alineación" (AABB) que Box2D usa para cálculos rápidos de colisión
	void DrawAABB(b2AABB* aabb, const b2Color& color);
};

*/