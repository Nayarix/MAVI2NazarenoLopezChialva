#include "Game.h"

// ========================================================
// 1. CONSTRUCTOR Y CONFIGURACIÓN INICIAL
// ========================================================
Game::Game(int width, int height, const std::string& title) {
	
	// Configuración de la ventana de SFML
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

	window = new sf::RenderWindow(desktop, title, sf::Style::Fullscreen);
	window->setFramerateLimit(60);


	sf::View view(sf::FloatRect(0, 0, width, height));
	window->setView(view);

	// Inicialización del mundo físico (Box2D) con gravedad estándar
	physicsWorld = new b2World(b2Vec2(0.0f, 9.8f));






	// --- CARGA DE TEXTURAS ---
	// Se cargan todas las imágenes necesarias para los elementos visuales
	if (!texturaCaja.loadFromFile("TexturaCaja.png")) {}
	if (!texturaMetal.loadFromFile("metalrustico.png")) {}
	if (!texturaMetalPlat.loadFromFile("MetalPlataforma.png")) {}
	if (!texturaMetalRueda.loadFromFile("MetalRueda.png")) {}
	if (!texturaFondo.loadFromFile("ImagenFondo.png")) {}
	if (!texturaCañon.loadFromFile("ImagenCañon2.png")) {}
	if (!fuente.loadFromFile("font.ttf")) {}

	// --- CONFIGURACIÓN DEL CAÑÓN ---
	// Base del cañón (Parte estática)
	cannonBase.setSize(sf::Vector2f(30.0f, 30.0f));
	cannonBase.setOrigin(15.0f, 15.0f);
	cannonBase.setPosition(30.0f, height - 30.0f);
	cannonBase.setFillColor(sf::Color(150, 150, 150));

	// Tubo del cañón (Parte que rota y dispara)
	cannonBarrel.setSize(sf::Vector2f(CANNON_LENGTH, 50.0f));
	cannonBarrel.setOrigin(0.0f, 5.0f); // Origen en la base para rotación correcta
	cannonBarrel.setPosition(30.0f, height - 30.0f);
	cannonBarrel.setTexture(&texturaCañon);

	// --- INICIALIZACIÓN DE ELEMENTOS ---
	CreateBoundaries(width, height); // Crear paredes del mundo       
	estadoActual = GameState::MAIN_MENU;
	InicializarBotones();            // Configurar botones de la interfaz
}

// Destructor para liberar memoria
Game::~Game() {
	delete physicsWorld;
	delete window;
}

// ========================================================
// 2. CLASE RAGDOLL: LÓGICA Y FÍSICA
// ========================================================
Ragdoll::Ragdoll(b2World* world, const b2Vec2& position) {
	// --- CREACIÓN DE CUERPOS FÍSICOS ---

	//Cabeza con un circulo
	cabeza = Box2DHelper::CreateCircularDynamicBody(world, 0.3f, 0.8f, 0.2f, 0.1f);
	cabeza->SetTransform(position + b2Vec2(0.0f, -0.7f), 0.0f);
	
	//Torso con un rectangulo
	torso = Box2DHelper::CreateRectangularDynamicBody(world, 0.5f, 0.8f, 5.0f, 0.2f, 0.1f);
	torso->SetTransform(position, 0.0f);


	// Extremidades con rectángulos más delgados
	brazoIzq = Box2DHelper::CreateRectangularDynamicBody(world, 0.6f, 0.2f, 0.6f, 0.2f, 0.1f);
	brazoDer = Box2DHelper::CreateRectangularDynamicBody(world, 0.6f, 0.2f, 0.6f, 0.2f, 0.1f);
	piernaIzq = Box2DHelper::CreateRectangularDynamicBody(world, 0.2f, 0.6f, 1.0f, 0.2f, 0.1f);
	piernaDer = Box2DHelper::CreateRectangularDynamicBody(world, 0.2f, 0.6f, 1.0f, 0.2f, 0.1f);

	// Posicionamiento inicial de las extremidades relativo al torso
	brazoIzq->SetTransform(position + b2Vec2(-0.55f, 0.0f), 0.0f);
	brazoDer->SetTransform(position + b2Vec2(0.55f, 0.0f), 0.0f);
	piernaIzq->SetTransform(position + b2Vec2(-0.2f, 0.7f), 0.0f);
	piernaDer->SetTransform(position + b2Vec2(0.2f, 0.7f), 0.0f);




	// --- CREACIÓN DE UNIONES (JOINTS) ---
	// Lambda para crear DistanceJoints que actúan como resortes elásticos
	auto CreateFlexibleJoint = [world](b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchorA, const b2Vec2& anchorB) {
		b2DistanceJointDef jointDef;
		jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldPoint(anchorA), bodyB->GetWorldPoint(anchorB));
		jointDef.collideConnected = true;
		jointDef.stiffness = 2.0f; // Nivel de elasticidad/rigidez (que tanto se opone al estiramiento)
		jointDef.damping = 0.2f;   // Amortiguación (que tan rapido frenan las oscilaciones)
		return world->CreateJoint(&jointDef);
		};	

	// Coneccion de todas las piezas usando el sistema de resortes
	cuello = CreateFlexibleJoint(torso, cabeza, { 0.0f, -0.4f }, { 0.0f, 0.3f });
	hombroIzq = CreateFlexibleJoint(torso, brazoIzq, { -0.25f, 0.0f }, { 0.3f, 0.0f });
	hombroDer = CreateFlexibleJoint(torso, brazoDer, { 0.25f, 0.0f }, { -0.3f, 0.0f });
	caderaIzq = CreateFlexibleJoint(torso, piernaIzq, { -0.1f, 0.4f }, { 0.0f, -0.3f });
	caderaDer = CreateFlexibleJoint(torso, piernaDer, { 0.1f, 0.4f }, { 0.0f, -0.3f });

	tiempoVida = 0.0f; // Inicia contador de vida, para que se eliminen solos con el pasar del tiempo
}

void Ragdoll::ApplyImpulse(const b2Vec2& impulse) {
	// Se aplica el impulso al torso multiplicado para compensar la masa
	b2Vec2 impulsePotente(impulse.x * 1.5f, impulse.y * 1.5f);
	torso->ApplyLinearImpulse(impulsePotente, torso->GetWorldCenter(), true);
}

// ========================================================
// 3. CLASE OBSTÁCULO: REPRESENTACIÓN VISUAL Y FÍSICA
// ========================================================
Obstaculo::Obstaculo(b2World* world, const b2Vec2& position, float width, float height, bool estatico, sf::Texture& textura)
	: esEstatico(estatico) {

	posicionInicial = position; // Guardar posición original para detectar movimiento

	// Definición del cuerpo físico
	b2BodyDef bodyDef;
	bodyDef.type = estatico ? b2_staticBody : b2_dynamicBody;
	bodyDef.position = position;
	body = world->CreateBody(&bodyDef);

	// Definición de la forma (Rectangular)
	b2PolygonShape shape;
	shape.SetAsBox(width / 2, height / 2);

	// Propiedades de la superficie
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = estatico ? 0.0f : 0.66f;
	fixtureDef.friction = 0.4f;
	fixtureDef.restitution = 0.2f;
	body->CreateFixture(&fixtureDef);

	// Configuración del Sprite visual
	spriteVisual.setTexture(textura);
	sf::FloatRect bounds = spriteVisual.getLocalBounds();
	spriteVisual.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

	// Ajustar escala según metros de Box2D a píxeles de SFML
	float anchoEnPixeles = width * 30.0f;
	float altoEnPixeles = height * 30.0f;
	spriteVisual.setScale(anchoEnPixeles / bounds.width, altoEnPixeles / bounds.height);

	if (!estatico) body->SetAwake(false); // Dormido hasta ser golpeado
}

void Obstaculo::Draw(sf::RenderWindow* window) const {
	// Sincronizar posición visual del Sprite con la posición física del cuerpo
	b2Vec2 posicionFisica = body->GetPosition();
	float anguloFisico = body->GetAngle();

	sf::Sprite spriteTemp = spriteVisual;
	spriteTemp.setPosition(posicionFisica.x * 30.0f, posicionFisica.y * 30.0f);
	spriteTemp.setRotation(anguloFisico * 180.0f / b2_pi);
	window->draw(spriteTemp);
}

// ========================================================
// 4. LÓGICA PRINCIPAL (LOOP, EVENTOS Y UPDATE)
// ========================================================
void Game::Loop() {
	sf::Clock clock;
	while (window->isOpen()) {
		float deltaTime = clock.restart().asSeconds();
		HandleEvents();
		Update(deltaTime);
		Render();
	}
}

void Game::HandleEvents() {
	sf::Event event;
	
	sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
	sf::Vector2f mousePos = window->mapPixelToCoords(pixelPos);

	while (window->pollEvent(event)) {
		if (event.type == sf::Event::Closed) window->close();

		if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
			// Gestión de clics según el estado del juego
			if (estadoActual == GameState::LEVEL_WON) {
				if (EsClicEnBoton(btnVolverMenu, mousePos)) {
					estadoActual = GameState::MAIN_MENU;
					LimpiarMundo();
				}
			}
			else if (estadoActual == GameState::MAIN_MENU) {
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
				estadoActual = GameState::MAIN_MENU;
			}
			else if (estadoActual == GameState::PLAYING) {
				if (EsClicEnBoton(btnMenuPrincipal, mousePos)) estadoActual = GameState::MAIN_MENU;
				else FireRagdoll(); // Disparar si no se hizo clic en el menú
			}
		}
	}
}

void Game::Update(float deltaTime) {
	if (estadoActual == GameState::PLAYING) {
		//Esta linea la encargada de hacer avanzar la simulación del motor físico, es decir, de calcular las nuevas posiciones, velocidades y colisiones de todos los cuerpos físicos en el mundo.
		physicsWorld->Step(deltaTime, 8, 3);
		UpdateCannonRotation();

		// --- GESTIÓN DE VIDA DEL RAGDOLL ---

		//Empieza en el primer muñeco (begin), termina en el último (end), pero NO da un paso adelante automáticamente
		//El auto it crea un Iterador (un puntero inteligente que señala en qué silla estás parado actualmente) que se va moviendo por cada elemento del vector de ragdolls
		for (auto it = ragdolls.begin(); it != ragdolls.end(); ) {
			(*it)->tiempoVida += deltaTime;
			if ((*it)->tiempoVida >= 5.0f) {
				// Destrucción de cuerpos físicos para liberar memoria. Lo destruyo en Box2D , no en el vector, porque el vector es solo un contenedor de punteros a los cuerpos físicos, no los cuerpos en sí.
				physicsWorld->DestroyBody((*it)->cabeza);
				physicsWorld->DestroyBody((*it)->torso);
				physicsWorld->DestroyBody((*it)->brazoIzq);
				physicsWorld->DestroyBody((*it)->brazoDer);
				physicsWorld->DestroyBody((*it)->piernaIzq);
				physicsWorld->DestroyBody((*it)->piernaDer);
				// Eliminación del ragdoll del vector usando el iterador actual.Borra al muñeco de la memoria de C++. devuelve un iterador nuevo que apunta automáticamente al muñeco que acaba de ocupar el lugar libre.
				it = ragdolls.erase(it);
			}
			else { ++it; }
		}

		// --- LÓGICA DE VICTORIA: DESPLAZAMIENTO ---
		bool todasMovidas = true;
		int contadorObjetivos = 0;

		for (auto it = obstaculos.begin(); it != obstaculos.end(); ) {
			b2Body* b = (*it)->body;

			// Si el objeto sale de los límites, se cuenta como movido y se elimina
			if (!(*it)->esEstatico && b->GetPosition().y > 22.0f) {
				physicsWorld->DestroyBody(b);
				it = obstaculos.erase(it);
				continue;
			}

			// Solo evalua cajas dinámicas que no estén atadas a joints (objetivos reales)
			if (!(*it)->esEstatico && b->GetJointList() == nullptr) {
				contadorObjetivos++;
				float distanciaRecorrida = (b->GetPosition() - (*it)->posicionInicial).Length();

				// Si alguna caja objetivo se ha movido menos de 0.5m, no hay victoria aún
				if (distanciaRecorrida < 0.5f) todasMovidas = false;
			}
			++it;
		}

		// Solo se gana si existen objetivos y todos se desplazaron
		if (contadorObjetivos > 0 && todasMovidas) {
			estadoActual = GameState::LEVEL_WON;
		}
	}
}

// ========================================================
// 5. RENDERIZADO VISUAL
// ========================================================
void Game::Render() {
	window->clear(sf::Color(20, 20, 20));
	sf::Vector2f logicalSize = window->getView().getSize();

	// --- FONDO ---
	sf::Sprite spriteFondo;
	spriteFondo.setTexture(texturaFondo);
	sf::Vector2u windowSize = window->getSize();
	sf::Vector2u texSize = texturaFondo.getSize();
	if (texSize.x > 0 && texSize.y > 0) {
		spriteFondo.setScale(logicalSize.x / texSize.x, logicalSize.y / texSize.y);
		window->draw(spriteFondo);
	}

	// --- ELEMENTOS SEGÚN ESTADO ---
	if (estadoActual == GameState::MAIN_MENU) {
		window->draw(btnNiveles.forma); window->draw(btnNiveles.texto);
		window->draw(btnGuia.forma);    window->draw(btnGuia.texto);
		window->draw(btnCerrar.forma);   window->draw(btnCerrar.texto);
	}
	else if (estadoActual == GameState::LEVEL_SELECT) {
		window->draw(btnLvl1.forma); window->draw(btnLvl1.texto);
		window->draw(btnLvl2.forma); window->draw(btnLvl2.texto);
		window->draw(btnLvl3.forma); window->draw(btnLvl3.texto);
		window->draw(btnVolver.forma); window->draw(btnVolver.texto);
	}
	else if (estadoActual == GameState::RAGDOLL_GUIDE) {
		sf::Text guia("GUIA:\n1. Apunta con el Mouse\n2. Clic para disparar\n3. Mueve todas las cajas\n\nClic para volver", fuente, 20);
		guia.setPosition(200, 200);
		guia.setFillColor(sf::Color::Black);
		window->draw(guia);
	}
	else if (estadoActual == GameState::PLAYING) {
		window->draw(cannonBase);
		window->draw(cannonBarrel);
		for (auto& obs : obstaculos) obs->Draw(window);

		// Dibujar Ragdolls (Color gris)
		for (auto& rag : ragdolls) {
			DrawBody(rag->cabeza, sf::Color(128, 128, 128));
			DrawBody(rag->torso, sf::Color(128, 128, 128));
			DrawBody(rag->brazoIzq, sf::Color(128, 128, 128));
			DrawBody(rag->brazoDer, sf::Color(128, 128, 128));
			DrawBody(rag->piernaIzq, sf::Color(128, 128, 128));
			DrawBody(rag->piernaDer, sf::Color(128, 128, 128));
		}

		// UI del nivel
		sf::Text contador("Ragdolls Activos: " + std::to_string(ragdolls.size()), fuente, 20);
		contador.setPosition(10.0f, 10.0f);
		window->draw(contador);
		window->draw(btnMenuPrincipal.forma);
		window->draw(btnMenuPrincipal.texto);
	}
	else if (estadoActual == GameState::LEVEL_WON) {
		// Overlay de victoria

		sf::RectangleShape overlay(logicalSize);
		overlay.setFillColor(sf::Color(0, 0, 0, 180));
		window->draw(overlay);

		//Texto de victoria
		sf::Text textoWin("SUPERASTE EL NIVEL", fuente, 50);
		textoWin.setFillColor(sf::Color::Yellow);
		sf::FloatRect tr = textoWin.getLocalBounds();
		textoWin.setOrigin(tr.left + tr.width / 2.0f, tr.top + tr.height / 2.0f);
		textoWin.setPosition(logicalSize.x / 2.0f, logicalSize.y / 2.5f);

		window->draw(textoWin);
		window->draw(btnVolverMenu.forma);
		window->draw(btnVolverMenu.texto);
	}

	window->display();
}

// ========================================================
// 6. FUNCIONES AUXILIARES DE FÍSICA Y UI
// ========================================================
void Game::CreateBoundaries(float width, float height) {
	// Creación de paredes invisibles alrededor de la ventana
	// Convierte las dimensiones de la ventana de píxeles a metros para Box2D
	float wW = width / PIXELS_TO_METERS;
	float wH = height / PIXELS_TO_METERS;
	//Es el grosor base de las paredes
	float thickness = 1.0f;


	// Definición de un cuerpo estático para las paredes
	b2BodyDef wallDef; wallDef.type = b2_staticBody;
	auto CreateWall = [&](b2Vec2 pos, b2Vec2 size) {
		b2Body* wall = physicsWorld->CreateBody(&wallDef);
		b2PolygonShape shape; shape.SetAsBox(size.x, size.y, pos, 0);
		wall->CreateFixture(&shape, 0.0f);
		boundaryWalls.push_back(wall);
		};

	// Paredes: izquierda, derecha, techo y suelo. El suelo es más grueso para evitar que los objetos lo atraviesen
	CreateWall({ -thickness, wH / 2 }, { thickness, wH }); // Izquierda
	CreateWall({ wW + thickness, wH / 2 }, { thickness, wH }); // Derecha
	CreateWall({ wW / 2, -thickness }, { wW, thickness }); // Techo
	CreateWall({ wW / 2, wH + thickness * 2 }, { wW, thickness * 2 }); // Suelo
}


//Disparo del Ragdoll
void Game::FireRagdoll() {
	//Transforma grados en radiales para que lo entienda c++, y asi calculo en angulo del cañon
	float angle = cannonBarrel.getRotation() * b2_pi / 180.0f;

	//Calcula de posición de la punta del cañón
	sf::Vector2f barrelEnd = cannonBarrel.getPosition() + sf::Vector2f(cos(angle), sin(angle)) * CANNON_LENGTH;
	
	//Define la posicion de spawn del Ragdoll
	b2Vec2 spawnPos(barrelEnd.x / PIXELS_TO_METERS, barrelEnd.y / PIXELS_TO_METERS);

	//Crea el Ragdoll
	auto newRagdoll = std::make_unique<Ragdoll>(physicsWorld, spawnPos);

	//Posicion del mouse
	sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
	sf::Vector2f mousePos = window->mapPixelToCoords(pixelPos);


	// Aplica fuerza proporcional a la distancia del mouse
	//Calcula el poder de disparo basado en la distancia del mouse al cañón, con un máximo definido
	float power = CalculatePower(mousePos);

	//Multiplica el cos(dirección X) por la fuerza total, y el sin(dirección Y) por la fuerza total
	b2Vec2 force(cos(angle) * power, sin(angle) * power);

	// Aplica el impulso al ragdoll
	newRagdoll->ApplyImpulse(force);

	// Agrega el nuevo ragdoll al vector de ragdolls activos
	ragdolls.push_back(std::move(newRagdoll));
}

// Calcula la potencia del disparo basada en la distancia del mouse al cañón, con un máximo definido
float Game::CalculatePower(const sf::Vector2f& mousePos) {
	// Obtengo la posición del cañón 
	sf::Vector2f cannonPos = cannonBarrel.getPosition();
	
	// Calcula la distancia entre el mouse y el cañón, usando el teorema de Pitágoras para obtener la distancia euclidiana
	float dist = sqrt(pow(mousePos.x - cannonPos.x, 2) + pow(mousePos.y - cannonPos.y, 2));
	
	//Divide la distancia real por la distancia máxima
	// Normaliza la distancia para que esté entre 0 y 1, donde 1 corresponde a MAX_DISTANCE o más
	float normDist = std::min(dist / MAX_DISTANCE, 1.0f);
	
	// Escala la potencia entre el mínimo y máximo definido, basado en la distancia normalizada
	return MIN_CANNON_POWER + (MAX_CANNON_POWER - MIN_CANNON_POWER) * normDist;
}

// Rota el cañón para que apunte hacia la posición actual del mouse
void Game::UpdateCannonRotation() {
	//Declaro la posición del mouse y del cañón para calcular la dirección
	sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
	sf::Vector2f mousePos = window->mapPixelToCoords(pixelPos);
	sf::Vector2f cannonPos = cannonBarrel.getPosition();	

	// Calcula la dirección del mouse respecto al cañón. Destino - Origen = Vector que apunta del origen al destino (direccion)
	sf::Vector2f dir = mousePos - cannonPos;

	// Calcula el ángulo usando atan2, que devuelve el ángulo en radiales entre el eje X positivo y el vector (dir.x, dir.y)
	//atan2f revisa los signos de X e Y para saber exactamente en qué cuadrante de la pantalla está tu mouse, y así devuelve el ángulo correcto entre -180 y 180 grados en radiales. 
	// Luego convierte los radiales a grados multiplicando por 180/pi.
	cannonBarrel.setRotation(atan2f(dir.y, dir.x) * 180.0f / b2_pi);
}

bool Game::EsClicEnBoton(const Boton& b, const sf::Vector2f& mousePos) { 
	return b.forma.getGlobalBounds().contains(mousePos); 
}

void Game::InicializarBotones() {
	auto Config = [&](Boton& b, std::string txt, sf::Vector2f pos) {
		b.forma.setSize({ 250, 50 }); 
		b.forma.setOrigin(125, 25); 
		b.forma.setPosition(pos);
		b.forma.setFillColor(sf::Color(70, 70, 70)); 
		b.forma.setOutlineThickness(2);
		b.texto.setFont(fuente); 
		b.texto.setString(txt); 
		b.texto.setCharacterSize(20);
		sf::FloatRect tr = b.texto.getLocalBounds();
		b.texto.setOrigin(tr.left + tr.width / 2.0f, tr.top + tr.height / 2.0f);
		b.texto.setPosition(pos);
		};

	Config(btnNiveles, "SELECCIONAR NIVELES", { 400, 250 });
	Config(btnGuia, "GUIA DE RAGDOLLS", { 400, 320 });
	Config(btnCerrar, "SALIR", { 400, 390 });
	Config(btnLvl1, "NIVEL 1 (FACIL)", { 400, 200 });
	Config(btnLvl2, "NIVEL 2 (MEDIO)", { 400, 270 });
	Config(btnLvl3, "NIVEL 3 (DIFICIL)", { 400, 340 });
	Config(btnVolver, "VOLVER", { 400, 450 });
	Config(btnVolverMenu, "VOLVER AL MENU", { 400, 350 });

	// Botón pequeño de menú durante el juego
	Config(btnMenuPrincipal, "MENU", { 740, 30 });
	btnMenuPrincipal.forma.setSize({ 80, 40 }); 
	btnMenuPrincipal.forma.setOrigin(40, 20);
	btnMenuPrincipal.texto.setCharacterSize(15);
}

// ========================================================
// 7. GESTIÓN DE NIVELES Y LIMPIEZA
// ========================================================
void Game::LimpiarMundo() {
	for (auto& rag : ragdolls) {
		physicsWorld->DestroyBody(rag->cabeza);
		physicsWorld->DestroyBody(rag->torso);
		physicsWorld->DestroyBody(rag->brazoIzq);
		physicsWorld->DestroyBody(rag->brazoDer);
		physicsWorld->DestroyBody(rag->piernaIzq);
		physicsWorld->DestroyBody(rag->piernaDer);
	}
	ragdolls.clear();
	for (auto& obs : obstaculos) physicsWorld->DestroyBody(obs->body);
	obstaculos.clear();
}

void Game::CargarNivel(int num) {
	LimpiarMundo();
	nivelSeleccionado = num;

	if (num == 1) {
		// --- NIVEL 1: RUEDAS CON ASPAS ---

		//Puntos en los que se van a anclar las ruedas, definidos como cuerpos estáticos sin fixtures
		b2BodyDef a1, a2;
		a1.position.Set(10.0f, 13.0f); 
		a2.position.Set(17.0f, 10.0f);
		b2Body* e1 = physicsWorld->CreateBody(&a1);
		b2Body* e2 = physicsWorld->CreateBody(&a2);	

		//Crea los obstaculos moviles
		auto r1 = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(10, 13), 4, 4, false, texturaMetalRueda);
		auto b1H = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(7, 13), 3, 0.8f, false, texturaMetal);
		auto b1V = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(10, 16), 0.8f, 3, false, texturaMetal);
		auto r2 = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(17, 10), 4, 4, false, texturaMetalRueda);
		auto b2H = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(14, 10), 3, 0.8f, false, texturaMetal);

		//Les quita la gravedad
		r1->body->SetGravityScale(0); b1H->body->SetGravityScale(0); b1V->body->SetGravityScale(0);
		r2->body->SetGravityScale(0); b2H->body->SetGravityScale(0);

		//Los ata a los ejes con joints de tipo weld (soldadura) para que giren juntos como un solo cuerpo, pero sin perder la capacidad de rotar libremente
		b2WeldJointDef w;
		            //cuerpo 1 //cuerpo 2   //punto de anclaje (en este caso el centro del cuerpo)
		w.Initialize(r1->body, b1H->body, r1->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);
		w.Initialize(r1->body, b1V->body, r1->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);
		w.Initialize(r2->body, b2H->body, r2->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);

		//Luego los ata a los ejes con joints de tipo revolute (revolución) para que puedan girar libremente alrededor de esos puntos, como si fueran ruedas
		b2RevoluteJointDef rv;	
		//cuerpo 1 //cuerpo 2   //punto de anclaje (en este caso el centro del cuerpo)
		rv.Initialize(e1, r1->body, e1->GetWorldCenter()); physicsWorld->CreateJoint(&rv);
		rv.Initialize(e2, r2->body, e2->GetWorldCenter()); physicsWorld->CreateJoint(&rv);


		//Controla los obstaculos con un vector y no con variables locales para poder iterar sobre ellos después en la lógica de victoria, y para que se dibujen automáticamente sin tener que escribir código extra
		obstaculos.push_back(std::move(r1)); obstaculos.push_back(std::move(b1H)); obstaculos.push_back(std::move(b1V));
		obstaculos.push_back(std::move(r2)); obstaculos.push_back(std::move(b2H));

		//Aquí lo mismo con los obstaculos estáticos y objetivos, en este caso no necesito variables ya que no los voy a manipular directamente
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(13.5, 3), 1.5, 6, true, texturaMetalPlat));
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(20, 6), 2, 2, false, texturaCaja));
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(18, 15), 2, 2, false, texturaCaja));
	}
	else if (num == 2) {
		// --- NIVEL 2: BALANZA ---
		b2BodyDef a;
		a.position.Set(0, 0);
		b2Body* sf = physicsWorld->CreateBody(&a);


		//Crea los dos bloques móviles que van a actuar como platos de la balanza
		auto oA = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(7, 10), 4, 1.4f, false, texturaMetal);
		auto oB = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19, 10), 4, 1.4f, false, texturaMetal);


		//Los ata al suelo con joints prismaticos, que permiten movimiento lineal a lo largo de un eje pero no rotación, para que se comporten como platos de una balanza
		b2PrismaticJointDef pA, pB;
		//Punto de aclaje al mundo   //Cuerpo a conectar   //Punto de anclaje en el cuerpo   //Eje de movimiento (en este caso vertical)	
		pA.Initialize(sf, oA->body, oA->body->GetWorldCenter(), { 0, 1 });
		//Establezco límites de movimiento para que no se caigan al suelo ni suban demasiado, y activo esos límites
		pA.lowerTranslation = -4; pA.upperTranslation = 4; pA.enableLimit = true;

		//Crea el joint y lo guarda en una variable para poder referenciarlo después en el joint de engranaje
		b2PrismaticJoint* jI = (b2PrismaticJoint*)physicsWorld->CreateJoint(&pA);

		//Hace lo mismo con el otro plato
		pB.Initialize(sf, oB->body, oB->body->GetWorldCenter(), { 0, 1 });
		pB.lowerTranslation = -4; pB.upperTranslation = 4; pB.enableLimit = true;
		b2PrismaticJoint* jD = (b2PrismaticJoint*)physicsWorld->CreateJoint(&pB);


		//Este join conecta las plataformas con railes, haciendo que se muevan en direcciones opuestas pero con la misma magnitud
		b2GearJointDef g; 

		g.bodyA = oA->body; //plato izquierdo conectado al cuerpo A del engranaje 
		g.bodyB = oB->body; //plato derecho conectado al cuerpo B del engranaje
		g.joint1 = jI;  //plato izquierdo conectado al joint prismatico 1
		g.joint2 = jD;  //plato derecho conectado al joint prismatico 2
		g.ratio = 1.0f; //relacion de transmición 1 a 1
		physicsWorld->CreateJoint(&g);


		//Agrega los obstaculos a un vector igual que con el nivel anterior
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(13, 10), 2, 16, true, texturaMetalPlat));
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19, 5), 2, 2, false, texturaCaja));
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19, 15), 2, 2, false, texturaCaja));
		obstaculos.push_back(std::move(oA)); obstaculos.push_back(std::move(oB));
	}
	else if (num == 3) {
		// --- NIVEL 3: PLATAFORMA XY ---

		// 1. CREACIÓN DE LOS EJES ESTÁTICOS (Los "clavos" invisibles en la pared)
		b2BodyDef aw1, aw2, arx;
		aw1.position.Set(8, 15); // Eje para la rueda superior
		aw2.position.Set(8, 5);  // Eje para la rueda inferior
		arx.position.Set(21, 10);// Eje ancla para el riel horizontal del carro

		// Al no ponerles tipo, nacen como b2_staticBody (inamovibles).
		b2Body* er1 = physicsWorld->CreateBody(&aw1);
		b2Body* er2 = physicsWorld->CreateBody(&aw2);
		b2Body* erX = physicsWorld->CreateBody(&arx);

		// 2. CREACIÓN DE LAS PIEZAS FÍSICAS (Los objetos que el jugador ve)
		// Rueda de arriba y sus dos aspas
		auto rd1 = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(8, 15), 4, 4, false, texturaMetalRueda);
		auto b1H = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(5.5, 15), 3, 0.8f, false, texturaMetal);
		auto b1V = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(8, 17.5), 0.8f, 3, false, texturaMetal);
		// Rueda de abajo y sus dos aspas
		auto rd2 = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(8, 5), 4, 4, false, texturaMetalRueda);
		auto b2H = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(5.5, 5), 3, 0.8f, false, texturaMetal);
		auto b2V = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(8, 7.5), 0.8f, 3, false, texturaMetal);

		// 3. AISLAMIENTO GRAVITACIONAL (Apagamos la gravedad para estas piezas)
		// Evita que las ruedas giren solas por el peso desparejo de las aspas.
		rd1->body->SetGravityScale(0); b1H->body->SetGravityScale(0); b1V->body->SetGravityScale(0);
		rd2->body->SetGravityScale(0); b2H->body->SetGravityScale(0); b2V->body->SetGravityScale(0);

		// 4. SOLDADURA (Fusionando las aspas a las ruedas)
		b2WeldJointDef w;
		// Suelda las aspas de la rueda superior
		w.Initialize(rd1->body, b1H->body, rd1->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);
		w.Initialize(rd1->body, b1V->body, rd1->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);
		// Suelda las aspas de la rueda inferior
		w.Initialize(rd2->body, b2H->body, rd2->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);
		w.Initialize(rd2->body, b2V->body, rd2->body->GetWorldCenter()); physicsWorld->CreateJoint(&w);

		// 5. BISAGRAS (Clavando las ruedas a la pared para que solo puedan rotar)
		b2RevoluteJointDef rv1, rv2;
		// Rueda superior anclada al eje er1. Guardamos el joint en jRX.
		rv1.Initialize(er1, rd1->body, er1->GetWorldCenter());
		b2RevoluteJoint* jRX = (b2RevoluteJoint*)physicsWorld->CreateJoint(&rv1);
		// Rueda inferior anclada al eje er2. Guardamos el joint en jRY.
		rv2.Initialize(er2, rd2->body, er2->GetWorldCenter());
		b2RevoluteJoint* jRY = (b2RevoluteJoint*)physicsWorld->CreateJoint(&rv2);

		// 6. EL CARRO INVISIBLE (Eje X)
		// Crea una cajita (0.1x0.1) que no se dibuja, pero existe en la física.
		b2BodyDef cDef;
		cDef.type = b2_dynamicBody;
		cDef.position.Set(21, 10);
		b2Body* carro = physicsWorld->CreateBody(&cDef);
		carro->SetGravityScale(0); // Tampoco tiene gravedad
		b2PolygonShape cs;
		cs.SetAsBox(0.1, 0.1);
		carro->CreateFixture(&cs, 1.0f);

		// 7. LA PLATAFORMA REAL (La que el jugador ve, Eje Y)
		auto pFinal = std::make_unique<Obstaculo>(physicsWorld, b2Vec2(21, 10), 2, 2, false, texturaMetalPlat);
		pFinal->body->SetGravityScale(0);
			
		// 8. LOS RIELES (Prismatic Joints)
		b2PrismaticJointDef pX, pY;
		// Riel X: Conecta el ancla estática de la pared (erX) con el carro invisible.
		// Vector {1, 0} permite movimiento SOLO de izquierda a derecha. Límites de -6 a 6 metros.
		pX.Initialize(erX, carro, carro->GetWorldCenter(), { 1, 0 });
		pX.lowerTranslation = -6; pX.upperTranslation = 6; pX.enableLimit = true;
		b2PrismaticJoint* jPX = (b2PrismaticJoint*)physicsWorld->CreateJoint(&pX);

		// Riel Y: Conecta el carro invisible con la plataforma real (pFinal).
		// Vector {0, 1} permite movimiento SOLO de arriba a abajo. Límites de -4 a 4 metros.
		pY.Initialize(carro, pFinal->body, pFinal->body->GetWorldCenter(), { 0, 1 });
		pY.lowerTranslation = -4; pY.upperTranslation = 4; pY.enableLimit = true;
		b2PrismaticJoint* jPY = (b2PrismaticJoint*)physicsWorld->CreateJoint(&pY);

		// 9. LOS ENGRANAJES (Conectando la rotación de las ruedas con el deslizamiento de los rieles)
		b2GearJointDef gX, gY;

		// Engranaje X: Conecta la rueda superior (rd1) con el carro invisible (carro).
		gX.bodyA = rd1->body;
		gX.bodyB = carro;
		gX.joint1 = jRX; // El giro de la rueda superior
		gX.joint2 = jPX; // El riel horizontal del carro
		gX.ratio = 2.0f; // Por cada 1 radián de giro, el carro se mueve 2 metros.
		physicsWorld->CreateJoint(&gX);

		// Engranaje Y: Conecta la rueda inferior (rd2) con la plataforma real (pFinal).
		gY.bodyA = rd2->body;
		gY.bodyB = pFinal->body;
		gY.joint1 = jRY; // El giro de la rueda inferior
		gY.joint2 = jPY; // El riel vertical de la plataforma
		gY.ratio = 2.0f; // Por cada 1 radián de giro, la plataforma sube/baja 2 metros.
		physicsWorld->CreateJoint(&gY);

		// 10. DECORACIÓN Y GUARDADO DE OBJETOS
		// Se agregan paredes, cajas flotantes y se guardan todos los unique_ptr en el vector de obstáculos.
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(13, 10), 1, 16, true, texturaMetalPlat));
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(19.0f, 15.0f), 2.0f, 2.0f, false, texturaCaja));
		obstaculos.push_back(std::make_unique<Obstaculo>(physicsWorld, b2Vec2(23.0f, 5.0f), 2.0f, 2.0f, false, texturaCaja));
		obstaculos.push_back(std::move(rd1)); obstaculos.push_back(std::move(b1H)); obstaculos.push_back(std::move(b1V));
		obstaculos.push_back(std::move(rd2)); obstaculos.push_back(std::move(b2H)); obstaculos.push_back(std::move(b2V));
		obstaculos.push_back(std::move(pFinal));
	}
}

// Función para dibujar formas de Box2D manualmente 
void Game::DrawBody(b2Body* body, const sf::Color& color) {

	//Box2D guarda estas formas en una estructura de datos llamada Lista Enlazada
	b2Fixture* fixture = body->GetFixtureList();

	//El bucle while evalúa esa pieza y la dibuja
	while (fixture) {

		//Primero, preguntas si la pieza actual es un círculo
		if (fixture->GetType() == b2Shape::e_circle) {

			//extrae su forma genérica con GetShape() con casteo explicito
			b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();

			//Crea un círculo gráfico de SFML. Como Box2D guarda el radio en metros (m_radius), lo multiplica por tu constante (30.0f) para convertirlo a píxeles
			sf::CircleShape sfCircle(circle->m_radius * PIXELS_TO_METERS);
			sfCircle.setOrigin(sfCircle.getRadius(), sfCircle.getRadius());

			//Obtiene la posición central del cuerpo en metros, la convierte a píxeles multiplicando por X e Y, le aplica el color gris pasado como parámetro, y finalmente estampa el círculo en la pantalla
			b2Vec2 pos = body->GetPosition();
			sfCircle.setPosition(pos.x * PIXELS_TO_METERS, pos.y * PIXELS_TO_METERS);
			sfCircle.setFillColor(color);
			window->draw(sfCircle);
		}

		//Pregunta si es un poligono (que en realidad es un rectángulo, pero Box2D lo guarda como un polígono de 4 vértices)
		else if (fixture->GetType() == b2Shape::e_polygon) {

			//castea el poligono
			b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();

			// Crea un sf::ConvexShape en SFML. Esta forma es como una "banda elástica virtual" a la que se le puede poner tantos clavos (vértices) como se quiera
			//Le indica a SFML cuántos vértices tiene exactamente leyendo poly->m_count (que para un rectángulo siempre será 4)
			sf::ConvexShape convex(poly->m_count);
			
			//Este bucle recorre las 4 esquinas del rectángulo matemático de Box2D
			for (int i = 0; i < poly->m_count; i++) {

				//Toma la posición x e y de cada esquina(que están medidas en metros desde el centro del cuerpo)
				b2Vec2 vertex = poly->m_vertices[i];
				//las convierte a píxeles y le dice a SFML: "Pon el punto gráfico número i en esta coordenada"
				convex.setPoint(i, sf::Vector2f(vertex.x * PIXELS_TO_METERS, vertex.y * PIXELS_TO_METERS));
			}

			//La figura de arriba se construyó asumiendo que el centro era 0,0 (Posición Local)
			b2Vec2 pos = body->GetPosition();
			//Usa setPosition para mover todo el dibujo a la posición real del muñeco en la pantalla
			convex.setPosition(pos.x * PIXELS_TO_METERS, pos.y * PIXELS_TO_METERS);
			//Extrae el ángulo del cuerpo de Box2D en radiales (GetAngle()), aplica la fórmula trigonométrica para pasarlo a grados (180/pi)
			convex.setRotation(body->GetAngle() * 180.0f / b2_pi);

			//Aplica el color y dibujo
			convex.setFillColor(color);
			window->draw(convex);
		}

		//La instrucción GetNext() salta a la siguiente pieza
		fixture = fixture->GetNext();
	}
}