// ============================================================
// CLASE SFMLRenderer: Automatización de dibujo con SFML
// Provee métodos para simplificar el renderizado de objetos Box2D
// NO SE ESTA UTILIZANDO EN ESTE PROYECTO, PERO SE DEJA COMO REFERENCIA PARA FUTURAS MEJORAS
// LA BASE DE ESTE PROYECTO PROVENIA DE UN EJEMPLOPUBLICADO EN LA CATEDRA, PERO DECIDÍ HACER UN RENDERIZADO MANUAL PARA ENTENDER MEJOR CÓMO FUNCIONA LA INTEGRACIÓN ENTRE BOX2D Y SFML
// ============================================================






/* 
#include "SFMLRenderer.h"

// --- CONSTRUCTOR ---
// Recibe un puntero a la ventana de SFML donde se realizarán los dibujos
SFMLRenderer::SFMLRenderer(RenderWindow* window)
{
    wnd = window;
}

// --- DESTRUCTOR ---
SFMLRenderer::~SFMLRenderer(void)
{
}

// --- DIBUJAR POLÍGONO (Solo contorno) ---
void SFMLRenderer::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{

    sf::ConvexShape polygon;
    polygon.setFillColor(Color(0, 0, 0, 0)); // Fondo transparente
    polygon.setOutlineColor(box2d2SFMLColor(color)); // Color de Box2D convertido a SFML
    polygon.setOutlineThickness(1.0f);

    polygon.setPointCount(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
        polygon.setPoint(i, Vector2f(vertices[i].x, vertices[i].y));

    wnd->draw(polygon);
}

// --- DIBUJAR POLÍGONO SÓLIDO (Relleno) ---
void SFMLRenderer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    sf::ConvexShape polygon;
    // El relleno tiene un poco de transparencia para ver objetos superpuestos
    polygon.setFillColor(box2d2SFMLColor(color));
    polygon.setOutlineColor(box2d2SFMLColor(color));

    polygon.setPointCount(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
        polygon.setPoint(i, Vector2f(vertices[i].x, vertices[i].y));

    wnd->draw(polygon);
}

// --- DIBUJAR CÍRCULO (Solo contorno) ---
void SFMLRenderer::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius); // Centrar el origen en el círculo
    circle.setPosition(center.x, center.y);
    circle.setFillColor(Color(0, 0, 0, 0)); // Transparente
    circle.setOutlineColor(box2d2SFMLColor(color));
    circle.setOutlineThickness(1.0f);

    wnd->draw(circle);
}

// --- DIBUJAR CÍRCULO SÓLIDO ---
void SFMLRenderer::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(center.x, center.y);
    circle.setFillColor(box2d2SFMLColor(color));
    circle.setOutlineColor(box2d2SFMLColor(color));

    wnd->draw(circle);
}

// --- DIBUJAR SEGMENTO (Líneas rectas) ---
void SFMLRenderer::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(p1.x, p1.y), box2d2SFMLColor(color)),
        sf::Vertex(sf::Vector2f(p2.x, p2.y), box2d2SFMLColor(color))
    };

    wnd->draw(line, 2, sf::Lines);
}

// --- DIBUJAR TRANSFORMACIÓN (Ejes X e Y de un cuerpo) ---
// Útil para ver hacia dónde está rotado un objeto (Rojo = X, Verde = Y)
void SFMLRenderer::DrawTransform(const b2Transform& xf)
{
    b2Vec2 p1 = xf.p, p2;
    const float k_axisScale = 20.0f; // Escala visual de los ejes

    // Eje X
    p2 = p1 + k_axisScale * xf.q.GetXAxis();
    sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(p1.x, p1.y), Color::Red),
        sf::Vertex(sf::Vector2f(p2.x, p2.y), Color::Green)
    };
    wnd->draw(line, 2, sf::Lines);

    // Eje Y
    p2 = p1 + k_axisScale * xf.q.GetYAxis();
    sf::Vertex line2[] =
    {
        sf::Vertex(sf::Vector2f(p1.x, p1.y), Color::Blue),
        sf::Vertex(sf::Vector2f(p2.x, p2.y), Color::Yellow)
    };
    wnd->draw(line2, 2, sf::Lines);
}

// --- DIBUJAR PUNTO ---
void SFMLRenderer::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
    const float radius = 0.5f;
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(p.x, p.y);
    circle.setFillColor(box2d2SFMLColor(color));

    wnd->draw(circle);
}

// --- DIBUJAR TEXTO ---
void SFMLRenderer::DrawString(int x, int y, const char* string, ...)
{
    sf::Text text;
    text.setString(string);
    text.setPosition((float)x, (float)y);
    text.setCharacterSize(12);

    wnd->draw(text);
}

// --- DIBUJAR AABB (Cajas de colisión alineadas) ---
void SFMLRenderer::DrawAABB(b2AABB* aabb, const b2Color& color)
{
    const auto halfSize = aabb->GetExtents();
    sf::RectangleShape rectangle(sf::Vector2f(halfSize.x, halfSize.y) * 2.0f);
    rectangle.setOrigin(halfSize.x, halfSize.y);
    const auto center = aabb->GetCenter();
    rectangle.setPosition(center.x, center.y);
    rectangle.setFillColor(Color(0, 0, 0, 0));
    rectangle.setOutlineColor(box2d2SFMLColor(color));
    rectangle.setOutlineThickness(1.0f);

    wnd->draw(rectangle);
}

// --- CONVERSIÓN DE COLOR: Box2D a SFML ---
// Box2D usa floats (0.0 a 1.0) para colores, SFML usa bytes (0 a 255)
Color SFMLRenderer::box2d2SFMLColor(const b2Color& _color)
{
    Uint8 R = (Uint8)(_color.r * 255);
    Uint8 G = (Uint8)(_color.g * 255);
    Uint8 B = (Uint8)(_color.b * 255);

    return Color(R, G, B);
}

*/