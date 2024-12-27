#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

class PhysBody;
class PhysicEntity;
class Kart;
class InteriorWall;
class ExteriorWall;

enum Screen
{
	MAINTITLE,
	CONTROLS,
	GAMEPLAY,

};

class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);
	void TextDraw();
	void engineSound();


public:

	Screen currentScreen;
	std::vector<PhysicEntity*> entities;
	
	PhysBody* winLine;
	PhysBody* sensor1;
	PhysBody* sensor2;
	PhysBody* sensor3;
	Kart* kart;
	Kart* kart2;
	InteriorWall* interior;
	ExteriorWall* exterior;

	bool canWin = false;
	bool newLap = true;
	bool firstCheck = false;
	bool secondCheck = false;
	bool goThroughWinLine = false;
	bool entitiesLoaded = false;

	int Laps = 1;

	Texture2D circuit;
	Texture2D mario;
	Texture2D NoTexture;

	uint32 bonus_fx;
	uint32 lap_fx;
	uint32 finalLap_fx;
	uint32 engine_fx;
	uint32 screenPass_fx;

	Music titleMusic;
	Music controlsMusic;
	Music circuitMusic;	
	Music circuitMusicFL;

	vec2<int> ray;
	bool ray_on;
};
