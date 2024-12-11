#pragma once

#include "Globals.h"
#include "Module.h"

#include "raylib.h"
#include <vector>
#include "Timer.h"

class PhysBody;
class PhysicEntity;

class Circle
{
public:
	Circle(PhysBody* i_body, float i_mass, int i_rad);
	~Circle();

	void Draw();
	void Update();

private:
	PhysBody* m_body = nullptr;
	float m_mass;
	int m_rad;
};


class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();

public:

	
	// TODO 1: Every second, create a circle at the leftmost side of the screen at a random height. These circles should be destroyed after 10 seconds.
	// TIP: You can check (and maybe reuse) some previous Handout...
	Timer m_creationTimer;
	std::vector<Circle> m_circles;



	// TODO 5 & 6:
	std::vector<float> m_staticFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
	std::vector<float> m_dynamicFrictions = { 0.0f, 0.1f, 0.3f, 0.5f };
	int m_currentStaticFriction = 0;
	int m_currentDynamicFriction = 0;
};
