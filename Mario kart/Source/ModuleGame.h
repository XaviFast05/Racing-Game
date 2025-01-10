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
	RESULTS,

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
	void CleanEntities();
	void DrawPodium();

public:

	Screen currentScreen;
	std::vector<PhysicEntity*> entities;
	
	PhysBody* winLine;
	//sensors
	PhysBody* sensor1;
	PhysBody* sensor2;
	PhysBody* sensor3;
	//paths
	PhysBody* path1;
	PhysBody* path2;
	PhysBody* path3;
	PhysBody* path4;
	PhysBody* path5;
	PhysBody* path6;
	//tracks
	PhysBody* track1;
	PhysBody* track2;
	PhysBody* track3;
	PhysBody* track4;
	PhysBody* track5;
	PhysBody* track6;
	PhysBody* track7;

	//karts
	Kart* kart;
	Kart* kart2;
	Kart* kart3;
	//walls
	InteriorWall* interior;
	ExteriorWall* exterior;

	// Checks for mario
	bool canWin = false;
	bool newLap = true;
	bool firstCheck = false;
	bool secondCheck = false;
	bool goThroughWinLine = false;

	// Checks for luigi
	bool canWinL = false;
	bool newLapL = true;
	bool firstCheckL = false;
	bool secondCheckL = false;
	bool goThroughWinLineL = false;

	// Checks for peach
	bool canWinP = false;
	bool newLapP = true;
	bool firstCheckP = false;
	bool secondCheckP = false;
	bool goThroughWinLineP = false;

	bool entitiesLoaded = false;
	bool finalLap = false;

	int Laps = 1;
	int LapsM = 1;
	int LapsL = 1;
	int LapsP = 1;

	int PositionPointsM = 0;
	int PositionPointsL = 0;
	int PositionPointsP = 0;

	Texture2D circuit;
	Texture2D title;
	Texture2D controls;
	Texture2D results;
	Texture2D ui;
	Texture2D mario;
	Texture2D luigi;
	Texture2D peach;
	Texture2D NoTexture;

	uint32 bonus_fx;
	uint32 lap_fx;
	uint32 finalLap_fx;
	uint32 engine_fx;
	uint32 turbo_fx;
	uint32 screenPass_fx;

	Music titleMusic;
	Music controlsMusic;
	Music circuitMusic;	
	Music circuitMusicFL;
	Music resultsMusic;

	float bestTime = 1000000.0f;

	vec2<int> ray;
	bool ray_on;
};
