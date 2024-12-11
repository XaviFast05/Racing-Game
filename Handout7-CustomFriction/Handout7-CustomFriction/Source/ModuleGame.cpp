#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

#include <cmath>
#include <format>

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	
}

ModuleGame::~ModuleGame()
{}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	m_creationTimer.Start();

	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");
	
	return true;
}


// Update: draw background
update_status ModuleGame::Update()
{
	// TODO 1:

	if (m_creationTimer.ReadSec() > 1)
	{
		m_creationTimer.Start();
		
		
		// TODO 3: Assign a random weight to each circle (from 1Kg to 100Kg), and make them bigger (or smaller) depending on their weight.
		int rad = std::rand() % 100;
		float mass = std::rand() % 19 + 1;
		int y = std::rand() % 500;
		PhysBody* circulio = App->physics->CreateCircle(METERS_TO_PIXELS(1), y + 40, rad);
		m_circles.push_back(Circle(circulio, mass, rad));

	}

	// TODO 5: With each left click, increase the STATIC friction coeficient. (At some point, reset it back to zero). Display it at the bottom of the screen.
	// TODO 6: With each right click, increase the DYNAMIC friction coeficient. (At some point, reset it back to zero). Display it at the bottom of the screen.
	
	DrawText(std::format("Static Friction: {}/ Dynamic Friction: {}", m_staticFrictions[m_currentStaticFriction], m_dynamicFrictions[m_currentDynamicFriction]).c_str(), 10, 600, 30, BLACK);

	for (Circle& c : m_circles)
	{
		c.Draw();
		c.Update();
	}

	return UPDATE_CONTINUE;
}


Circle::Circle(PhysBody* i_body, float i_mass, int i_rad)
	: m_body(i_body), m_mass(i_mass), m_rad(i_rad)
{
}


Circle::~Circle()
{
}

void Circle::Draw()
{
	b2Vec2 pos = m_body->body->GetPosition();
	//TODO 3:
	DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), m_rad, Color{ 128, 0, 0, 128 });

}

void Circle::Update(/*...*/)
{
	// TODO 2: Apply an horizontal Force to each circle so it crosses the screen with constant acceleration. (NO GRAVITY to the bottom of the screen)
	float force = 10.0f;
	m_body->body->ApplyForce(b2Vec2(force, 0.f), m_body->body->GetLocalCenter(), true);
	// TODO 4: Apply a friction to these circles (as if the game had a top view) while they cross the screen. 
}

