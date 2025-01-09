#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include <algorithm>

class PhysicEntity
{
protected:

	PhysicEntity(PhysBody* _body, Module* _listener)
		: body(_body)
		, listener(_listener)
	{
		body->listener = listener;
	}

public:
	virtual ~PhysicEntity() = default;
	virtual void Update() = 0;

	virtual int RayHit(vec2<int> ray, vec2<int> mouse, vec2<float>& normal)
	{
		return 0;
	}

	PhysBody* body;
protected:

	Module* listener;
};

class Kart : public PhysicEntity
{
public:
	Kart(ModulePhysics* physics, int _x, int _y, float initialRotation, Module* _listener, Texture2D _texture, int playerNum)
		: PhysicEntity(physics->CreateRectangle(_x, _y, 30, 25), _listener)
		, texture(_texture)
	{
		// Player 1 controls
		if (playerNum == 0)
		{
			upKey = KEY_W;
			downKey = KEY_S;
			leftKey = KEY_A;
			rightKey = KEY_D;
			turboKey = KEY_SPACE;
			isPlayer = true;
		}		
		// Player 2 controls
		else if (playerNum == 1)
		{
			upKey = KEY_UP;
			downKey = KEY_DOWN;
			leftKey = KEY_LEFT;
			rightKey = KEY_RIGHT;
			turboKey = KEY_ENTER;
			isPlayer = true;
		}
		else if (playerNum == 2)
			isPlayer = false;
		// Set initial rotation
		body->body->SetTransform(body->body->GetPosition(), initialRotation);
	}

	const float forceMagnitude = 1.0f;
	const float maxSpeed = 7.0f;
	const float maxRotation = 5.0f;

	const float inicialAngularSpeed = 0.05f;
	const float angularAcceleration = 0.05f;
	float forceRotation = 0.0f;

	const float inicialSpeed = 0.1f;
	const float acceleration = 0.1f;

	const float turboTime = 10.0f;
	const float brakeSpeed = 0.5f;

	Timer rotationTimer;
	Timer moveTimer;
	Timer gameTimer;
	float currentTime = 0.f;

	KeyboardKey upKey;
	KeyboardKey downKey;
	KeyboardKey leftKey;
	KeyboardKey rightKey;
	KeyboardKey turboKey;

	bool isPlayer;
	bool turboMReady;
	bool turboLReady;
	int currentPathIndex;

	Timer turboTimer;
	void StartTimer()
	{
		turboTimer.Start(); 
	}

	void Update() override
	{
		int x, y;
		body->GetPhysicPosition(x, y);

		float forceX = body->body->GetTransform().q.GetXAxis().x;
		float forceY = body->body->GetTransform().q.GetXAxis().y;

		b2Vec2 force(forceX, forceY);
		if(isPlayer == true)
		{
			// Forward
			if (IsKeyPressed(upKey))
			{
				moveTimer.Start();
			}
			if (IsKeyDown(upKey) && forceX < maxSpeed && forceY < maxSpeed) 
			{
				forceX += inicialSpeed + acceleration * moveTimer.ReadSec();
				forceY += inicialSpeed + acceleration * moveTimer.ReadSec();
				body->body->ApplyForceToCenter(force, true);
			}

			// Backward
			else if (IsKeyDown(downKey) && forceX > -maxSpeed && forceY > -maxSpeed) 
			{
				forceX += 0.1f;
				forceY += 0.1f;
				body->body->ApplyForceToCenter(-force, true);
			}

			// Brake
			else if (IsKeyUp(upKey) && IsKeyUp(downKey))
			{
				b2Vec2 velocity = body->body->GetLinearVelocity();
				b2Vec2 brakeForce = -brakeSpeed * velocity;
				body->body->ApplyForceToCenter(brakeForce, true);
			}

			// Rotate
			body->body->SetAngularVelocity(forceRotation);
			if (IsKeyPressed(leftKey) || IsKeyPressed(rightKey))
			{
				rotationTimer.Start();
			}

			// Turbo
			if (IsKeyPressed(turboKey) && turboTimer.ReadSec() > turboTime)
			{
				forceX += 2000.0f;
				forceY += 2000.0f;
				body->body->ApplyLinearImpulseToCenter(force, true);
				turboTimer.Start();
			}

			// Turbo text
			if (turboKey == KEY_SPACE && turboTimer.ReadSec() > turboTime)
			{
				DrawText("TURBO", 85, 900, 30, RED);
				turboMReady == true;
			}
			else if (turboKey == KEY_SPACE && turboTimer.ReadSec() < turboTime)
			{
				DrawText("-----", 85, 900, 30, RED);
			}
			
			if (turboKey == KEY_ENTER && turboTimer.ReadSec() > turboTime)
			{
				DrawText("TURBO", 290, 900, 30, GREEN);
				turboLReady == true;
			}
			else if (turboKey == KEY_ENTER && turboTimer.ReadSec() < turboTime)
			{
				DrawText("-----", 290, 900, 30, GREEN);
			}

			// left
			if (IsKeyDown(leftKey) && forceRotation > -maxRotation) 
			{
				forceRotation -= (inicialAngularSpeed + angularAcceleration * rotationTimer.ReadSec());
			}

			// right
			if (IsKeyDown(rightKey) && forceRotation < maxRotation) 
			{
				forceRotation += (inicialAngularSpeed + angularAcceleration * rotationTimer.ReadSec());
			}

			// brake rotation
			if (IsKeyUp(leftKey) && IsKeyUp(rightKey))
			{
				if (forceRotation < 0)
				{
					forceRotation += 0.1;
				}
				if (forceRotation > 0)
				{
					forceRotation -= 0.1;	
				}
			}		
		}
		else
		{
			CpuMovement();
		}

		currentTime = gameTimer.ReadSec();
		Vector2 position{ (float)x, (float)y };
		float scale = 0.3f;
		Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
		Rectangle dest = { position.x, position.y, (float)texture.width * scale, (float)texture.height * scale };
		Vector2 origin = { (float)texture.width / 6.2f, (float)texture.height / 6.3f };
		float rotation = body->GetRotation() * RAD2DEG;
		DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
	}
	void CpuMovement()
	{
		// path coordinates
		std::vector<Vector2> pathSensors = {
			{915, 420},
			{200, 85},
			{87, 670},
			{500, 600},
			{780, 915}
		};

		// reset path index when it reaches the end
		if (currentPathIndex >= pathSensors.size())
		{
			currentPathIndex = 0;
		}

		// obtain kart position and next destination
		int x, y;
		body->GetPhysicPosition(x, y);
		Vector2 position = { (float)x, (float)y };

		Vector2 target = pathSensors[currentPathIndex];

		// Calculate direction and distance to the next path 
		Vector2 direction = { target.x - position.x, target.y - position.y };
		float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
		direction.x /= distance;
		direction.y /= distance;

		// Aply force to the kart
		float forceX = direction.x * (inicialSpeed + acceleration);
		float forceY = direction.y * (inicialSpeed + acceleration);
		b2Vec2 force(forceX, forceY);
		body->body->ApplyForceToCenter(force, true);

		// Chech colision
		if (distance < 10.0f) // Umbral de colisión
		{
			currentPathIndex = (currentPathIndex + 1) % pathSensors.size(); // Cambiar al siguiente sensor
		}

		// Rotate kart to next location 
		float angle = atan2(direction.y, direction.x);
		body->body->SetTransform(body->body->GetPosition(), angle);
	}

private:
	Texture2D texture;
	
};

class InteriorWall : public PhysicEntity
{
public:
	// Pivot 0, 0
	static constexpr int interior_wall[32] = {
			168, 160,
			272, 160,
			272, 376,
			688, 376,
			688, 416,
			816, 416,
			816, 816,
			736, 816,
			736, 496,
			272, 496,
			272, 592,
			176, 592,
			176, 664,
			136, 664,
			136, 552,
			168, 552,
	};

	InteriorWall(ModulePhysics* physics, int _x, int _y, Module* _listener, Texture2D _texture)
		: PhysicEntity(physics->CreateChain(0, 0, interior_wall, 32), _listener)
		, texture(_texture)
	{

	}

	void Update() override
	{
		int x, y;
		body->GetPhysicPosition(x, y);
		DrawTextureEx(texture, Vector2{ (float)x, (float)y }, body->GetRotation() * RAD2DEG, 1.0f, WHITE);
	}

private:
	Texture2D texture;
};

class ExteriorWall : public PhysicEntity
{
public:
	// Pivot 0, 0
	static constexpr int exterior_wall[16] = {
				8, 872,
				480, 872,
				480, 1016,
				1016, 1016,
				1016, 160,
				736, 160,
				736, 8,
				8, 8


	};

	ExteriorWall(ModulePhysics* physics, int _x, int _y, Module* _listener, Texture2D _texture)
		: PhysicEntity(physics->CreateChain(0, 0, exterior_wall, 16), _listener)
		, texture(_texture)
	{

	}

	void Update() override
	{
		int x, y;
		body->GetPhysicPosition(x, y);
		DrawTextureEx(texture, Vector2{ (float)x, (float)y }, body->GetRotation() * RAD2DEG, 1.0f, WHITE);
	}

private:
	Texture2D texture;
};



ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	ray_on = false;
}

ModuleGame::~ModuleGame()
{}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	currentScreen = MAINTITLE;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	// Load textures
	circuit = LoadTexture("Assets/circuit.png");
	title = LoadTexture("Assets/TitleScreen.png");
	controls = LoadTexture("Assets/Controls.png");
	results = LoadTexture("Assets/Results.png");
	ui = LoadTexture("Assets/Cuadradetes.png");
	mario = LoadTexture("Assets/mario.png");
	luigi = LoadTexture("Assets/luigi.png");
	peach = LoadTexture("Assets/peach.png");

	// Load sound fx
	lap_fx = App->audio->LoadFx("Assets/SOUND/FX/LapTakenFX.wav");
	finalLap_fx = App->audio->LoadFx("Assets/SOUND/FX/FinalLap.wav");
	engine_fx = App->audio->LoadFx("Assets/SOUND/FX/Engine Sounds/engine8.wav");
	turbo_fx = App->audio->LoadFx("Assets/SOUND/FX/Turbo.wav");
	screenPass_fx = App->audio->LoadFx("Assets/SOUND/FX/ScreenPass.wav");

	// Load music
	titleMusic = LoadMusicStream("Assets/SOUND/MUSIC/MainMenu.ogg");
	controlsMusic = LoadMusicStream("Assets/SOUND/MUSIC/ControlsMenu.wav");
	circuitMusic = LoadMusicStream("Assets/SOUND/MUSIC/CircuitMusic.wav");
	circuitMusicFL = LoadMusicStream("Assets/SOUND/MUSIC/CircuitMusicFinalLap.wav");
	resultsMusic = LoadMusicStream("Assets/SOUND/MUSIC/Results.wav");
	PlayMusicStream(titleMusic);
	PlayMusicStream(controlsMusic);
	PlayMusicStream(circuitMusic);	
	PlayMusicStream(circuitMusicFL);
	PlayMusicStream(resultsMusic);
	
	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Game scene");
	// Unload bodies
	CleanEntities();	

	// Unload textures
	UnloadTexture(circuit);
	UnloadTexture(title);
	UnloadTexture(controls);
	UnloadTexture(results);
	UnloadTexture(ui);
	UnloadTexture(mario);
	UnloadTexture(luigi);
	UnloadTexture(peach);

	// Stop and unload music
	StopMusicStream(titleMusic);
	StopMusicStream(controlsMusic);
	StopMusicStream(circuitMusic);
	StopMusicStream(circuitMusicFL);	
	UnloadMusicStream(titleMusic);
	UnloadMusicStream(controlsMusic);
	UnloadMusicStream(circuitMusic);
	UnloadMusicStream(circuitMusicFL);
	return true;
}

// Update: draw background
update_status ModuleGame::Update()
{
	
	// Change game state -------------------------------------
	if (currentScreen == MAINTITLE) // Main title
	{
		// Play music
		UpdateMusicStream(titleMusic);
		// Draw main title
		float scaleX = (float)GetScreenWidth() / title.width;
		float scaleY = (float)GetScreenHeight() / title.height;
		DrawTextureEx(title, { 0, 0 }, 0.0f, fmax(scaleX, scaleY), WHITE);

		
		if (IsKeyPressed(KEY_SPACE))
		{
			App->audio->PlayFx(screenPass_fx);
			currentScreen = CONTROLS;
		}
		else if (IsKeyPressed(KEY_ENTER))
		{
			App->audio->PlayFx(screenPass_fx);
			currentScreen = GAMEPLAY;
		}
		entitiesLoaded = false;
	}
	else if (currentScreen == CONTROLS)
	{
		// Play music
		UpdateMusicStream(controlsMusic);
		// Draw controls
		entitiesLoaded = false;
		float scaleX = (float)GetScreenWidth() / controls.width;
		float scaleY = (float)GetScreenHeight() / controls.height;
		DrawTextureEx(controls, { 0, 0 }, 0.0f, fmax(scaleX, scaleY), WHITE);

		if (IsKeyPressed(KEY_SPACE))
		{
			App->audio->PlayFx(screenPass_fx);
			currentScreen = GAMEPLAY;
		}
	}
	else if (currentScreen == GAMEPLAY) // Draw gameplay
	{
		
		// Play music
		if ((LapsM == 3 || LapsL == 3 || LapsP == 3) && finalLap == true) UpdateMusicStream(circuitMusicFL);
		else UpdateMusicStream(circuitMusic);
		
		//load sensors -------------------------------------
		if (entitiesLoaded == false)
		{
			winLine = App->physics->CreateRectangleSensor(915, 555, 180, 9);
			sensor1 = App->physics->CreateRectangleSensor(200, 85, 9, 140);
			sensor2 = App->physics->CreateRectangleSensor(87, 400, 145, 9);
			sensor3 = App->physics->CreateRectangleSensor(800, 915, 9, 160);

			//load paths -------------------------------------
			path1 = App->physics->CreateRectangleSensor(915, 420, 180, 9);
			path2 = App->physics->CreateRectangleSensor(200, 85, 9, 140);
			path3 = App->physics->CreateRectangleSensor(87, 670, 145, 9);
			path4 = App->physics->CreateRectangleSensor(500, 600, 9, 160);
			path5 = App->physics->CreateRectangleSensor(780, 915, 9, 160);

			//load tracks ------------------------------------
			track1 = App->physics->CreateRectangleSensor(935, 641, 82, 510);
			track2 = App->physics->CreateRectangleSensor(571, 229, 830, 82);
			track3 = App->physics->CreateRectangleSensor(150, 90, 208, 92);
			track4 = App->physics->CreateRectangleSensor(80, 447, 82, 609);
			track5 = App->physics->CreateRectangleSensor(306, 662, 492, 82);
			track6 = App->physics->CreateRectangleSensor(629, 740, 442, 82);
			track7 = App->physics->CreateRectangleSensor(840, 900, 220, 96);

			//rotate tracks ----------------------------------
			track2->body->SetTransform(track2->body->GetPosition(), 26.5f * (b2_pi / 180.0f));
			track3->body->SetTransform(track3->body->GetPosition(), 335.0f * (b2_pi / 180.0f));
			track5->body->SetTransform(track5->body->GetPosition(), 337.0f * (b2_pi / 180.0f));
			track6->body->SetTransform(track6->body->GetPosition(), 61.0f * (b2_pi / 180.0f));
			track7->body->SetTransform(track7->body->GetPosition(), 345.0f * (b2_pi / 180.0f));

			//load karts -------------------------------------
			entities.emplace_back(DBG_NEW Kart(App->physics, 916, 600, 4.71, this, mario, 0));
			entities.emplace_back(DBG_NEW Kart(App->physics, 952, 630, 4.71, this, luigi, 1));
			entities.emplace_back(DBG_NEW Kart(App->physics, 916, 660, 4.71, this, peach, 2));
			kart = dynamic_cast<Kart*>(entities[0]);
			kart2 = dynamic_cast<Kart*>(entities[1]);
			kart3 = dynamic_cast<Kart*>(entities[2]);
			kart->StartTimer();
			kart2->StartTimer();
			kart3->StartTimer();

			kart->gameTimer.Start();

			//load walls -------------------------------------
			entities.emplace_back(DBG_NEW InteriorWall(App->physics, 0, 0, this, NoTexture));
			entities.emplace_back(DBG_NEW ExteriorWall(App->physics, 0, 0, this, NoTexture));
			interior = dynamic_cast<InteriorWall*>(entities[3]);
			exterior = dynamic_cast<ExteriorWall*>(entities[4]);
			App->renderer->camera.x = App->renderer->camera.y = 0;
			entitiesLoaded = true;
		}
		
		float scaleX = (float)GetScreenWidth() / circuit.width;
		float scaleY = (float)GetScreenHeight() / circuit.height;
		DrawTextureEx(circuit, { 0, 0 }, 0.0f, fmax(scaleX, scaleY), WHITE);

		DrawTexture(ui, 0, 0, WHITE);
		engineSound();

		if (IsKeyPressed(KEY_P) || LapsL > 3 || LapsM > 3 || LapsP > 3)
		{
			CleanEntities();
			currentScreen = RESULTS;
		}


	}
	else if (currentScreen == RESULTS) // Draw results
	{
		// Play music
		UpdateMusicStream(resultsMusic);
		// Draw results
		float scaleX = (float)GetScreenWidth() / results.width;
		float scaleY = (float)GetScreenHeight() / results.height;
		DrawTextureEx(results, { 0, 0 }, 0.0f, fmax(scaleX, scaleY), WHITE);

		if (IsKeyPressed(KEY_SPACE))
		{
			App->audio->PlayFx(screenPass_fx);
			currentScreen = MAINTITLE;
		}
	}

	if (currentScreen == GAMEPLAY)
	{
		//Update all entities
		for (PhysicEntity* entity : entities)
		{
			entity->Update();
		}

		if (IsKeyPressed(KEY_SPACE))
		{
			ray_on = !ray_on;
			ray.x = GetMouseX();
			ray.y = GetMouseY();
		}

		// Prepare for raycast ------------------------------------------------------

		vec2i mouse;
		mouse.x = GetMouseX();
		mouse.y = GetMouseY();
		int ray_hit = ray.DistanceTo(mouse);

		vec2f normal(0.0f, 0.0f);

		// All draw functions ------------------------------------------------------


		for (PhysicEntity* entity : entities)
		{
			entity->Update();
			if (ray_on)
			{
				int hit = entity->RayHit(ray, mouse, normal);
				if (hit >= 0)
				{
					ray_hit = hit;
				}
			}
		}


		// ray -----------------
		if (ray_on == true)
		{
			vec2f destination((float)(mouse.x - ray.x), (float)(mouse.y - ray.y));
			destination.Normalize();
			destination *= (float)ray_hit;

			DrawLine(ray.x, ray.y, (int)(ray.x + destination.x), (int)(ray.y + destination.y), RED);
		}
	}
	
	// Draw text
	TextDraw();

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	// Mario colissions -------------
	if ((bodyA == kart->body && bodyB == winLine) && canWin == true) // If the winLine is touched
	{
		firstCheck = false;
		secondCheck = false;
		canWin = false;
		newLap = true;

		// Play sound
		if (LapsM == 2 && finalLap == false)
		{
			App->audio->PlayFx(finalLap_fx);
			finalLap = true;
		}
		else App->audio->PlayFx(lap_fx);

		printf("Lap completed\n");

		PositionPointsM++;
		LapsM++;
	}
	else if ((bodyA == kart->body && bodyB == winLine) && firstCheck == true) // Reset if lap is not completed
	{
		firstCheck = false;
		secondCheck = false;
		canWin = false;
		newLap = true;
		printf("canWin Reseted\n");
		PositionPointsM--;
	}
	else if ((bodyA == kart->body && bodyB == winLine) && goThroughWinLine == true) // Reset if lap is not completed
	{
		newLap = true;
		goThroughWinLine = false;
		printf("newLap Reseted\n");
		PositionPointsM--;
	}

	if ((bodyA == kart->body && bodyB == sensor1) && newLap == true) // If the sensor1 is touched
	{
		newLap = false;
		canWin = false;
		secondCheck = false;
		firstCheck = true;
		printf("First check\n");
		PositionPointsM++;
	}
	else if ((bodyA == kart->body && bodyB == sensor1) && secondCheck == true) // Reset if sensor1 is touched before winLine
	{
		firstCheck = true;
		secondCheck = false;
		canWin = false;
		newLap = false;
		printf("firstCheck Reseted\n");
		PositionPointsM--;
	}

	if ((bodyA == kart->body && bodyB == sensor2) && firstCheck == true) // If the sensor2 is touched
	{
		firstCheck = false;
		secondCheck = true;
		newLap = false;
		canWin = false;
		printf("Second check\n");
		PositionPointsM++;
	}
	else if ((bodyA == kart->body && bodyB == sensor2) && canWin == true) // Reset if sensor2 is touched before sensor1
	{
		firstCheck = false;
		secondCheck = true;
		canWin = false;
		newLap = false;
		printf("secondCheck Reseted\n");
		PositionPointsM--;
	}

	if ((bodyA == kart->body && bodyB == sensor3) && secondCheck == true) // If the sensor3 is touched
	{
		canWin = true;
		firstCheck = false;
		secondCheck = false;
		newLap = false;
		printf("canWin\n");
		PositionPointsM++;
	}
	else if ((bodyA == kart->body && bodyB == sensor3) && newLap == true) // Dont let the player take the firstCheck unless it goes through winLine
	{
		goThroughWinLine = true;
		newLap = false;
		printf("Go through winLine\n");
		PositionPointsM--;
	}

	// Luigi colissions -------------
	if ((bodyA == kart2->body && bodyB == winLine) && canWinL == true) // If the winLine is touched
	{
		firstCheckL = false;
		secondCheckL = false;
		canWinL = false;
		newLapL = true;

		// Play sound
		if (LapsL == 2 && finalLap == false)
		{
			App->audio->PlayFx(finalLap_fx);
			finalLap = true;
		}
		else App->audio->PlayFx(lap_fx);

		printf("Lap completed\n");
		LapsL++;
		PositionPointsL++;
	}
	else if ((bodyA == kart2->body && bodyB == winLine) && firstCheckL == true) // Reset if lap is not completed
	{
		firstCheckL = false;
		secondCheckL = false;
		canWinL = false;
		newLapL = true;
		printf("canWin Reseted\n");
		PositionPointsL--;
	}
	else if ((bodyA == kart2->body && bodyB == winLine) && goThroughWinLineL == true) // Reset if lap is not completed
	{
		newLapL = true;
		goThroughWinLineL = false;
		printf("newLap Reseted\n");
		PositionPointsL--;
	}

	if ((bodyA == kart2->body && bodyB == sensor1) && newLapL == true) // If the sensor1 is touched
	{
		newLapL = false;
		canWinL = false;
		secondCheckL = false;
		firstCheckL = true;
		printf("First check\n");
		PositionPointsL++;
	}
	else if ((bodyA == kart2->body && bodyB == sensor1) && secondCheckL == true) // Reset if sensor1 is touched before winLine
	{
		firstCheckL = true;
		secondCheckL = false;
		canWinL = false;
		newLapL = false;
		printf("firstCheck Reseted\n");
		PositionPointsL--;
	}

	if ((bodyA == kart2->body && bodyB == sensor2) && firstCheckL == true) // If the sensor2 is touched
	{
		firstCheckL = false;
		secondCheckL = true;
		newLapL = false;
		canWinL = false;
		printf("Second check\n");
		PositionPointsL++;
	}
	else if ((bodyA == kart2->body && bodyB == sensor2) && canWinL == true) // Reset if sensor2 is touched before sensor1
	{
		firstCheckL = false;
		secondCheckL = true;
		canWinL = false;
		newLapL = false;
		printf("secondCheck Reseted\n");
		PositionPointsL--;
	}

	if ((bodyA == kart2->body && bodyB == sensor3) && secondCheckL == true) // If the sensor3 is touched
	{
		canWinL = true;
		firstCheckL = false;
		secondCheckL = false;
		newLapL = false;
		printf("canWin\n");
		PositionPointsL++;
	}
	else if ((bodyA == kart2->body && bodyB == sensor3) && newLapL == true) // Dont let the player take the firstCheck unless it goes through winLine
	{
		goThroughWinLineL = true;
		newLapL = false;
		printf("Go through winLine\n");
		PositionPointsL--;
	}

	// CPU peach colissions -------------

	if ((bodyA == kart3->body && bodyB == winLine) && canWinP == true) // If the winLine is touched
	{
		firstCheckP = false;
		secondCheckP = false;
		canWinP = false;
		newLapP = true;
		// Play sound
		if (LapsP == 2 && finalLap == false)
		{
			App->audio->PlayFx(finalLap_fx);
			finalLap = true;
		}
		else App->audio->PlayFx(lap_fx);
		printf("Lap completed\n");
		LapsP++;
		PositionPointsP++;
	}
	else if ((bodyA == kart3->body && bodyB == winLine) && firstCheckP == true) // Reset if lap is not completed
	{
		firstCheckP = false;
		secondCheckP = false;
		canWinP = false;
		newLapP = true;
		printf("canWin Reseted\n");
		PositionPointsP--;
	}
	else if ((bodyA == kart3->body && bodyB == winLine) && goThroughWinLineP == true) // Reset if lap is not completed
	{
		newLapP = true;
		goThroughWinLineP = false;
		printf("newLap Reseted\n");
		PositionPointsP--;
	}

	if ((bodyA == kart3->body && bodyB == sensor1) && newLapP == true) // If the sensor1 is touched
	{
		newLapP = false;
		canWinP = false;
		secondCheckP = false;
		firstCheckP = true;
		printf("First check\n");
		PositionPointsP++;
	}
	else if ((bodyA == kart3->body && bodyB == sensor1) && secondCheckP == true) // Reset if sensor1 is touched before winLine
	{
		firstCheckP = true;
		secondCheckP = false;
		canWinP = false;
		newLapP = false;
		printf("firstCheck Reseted\n");
		PositionPointsP--;
	}

	if ((bodyA == kart3->body && bodyB == sensor2) && firstCheckP == true) // If the sensor2 is touched
	{
		firstCheckP = false;
		secondCheckP = true;
		newLapP = false;
		canWinP = false;
		printf("Second check\n");
		PositionPointsP++;
	}
	else if ((bodyA == kart3->body && bodyB == sensor2) && canWinP == true) // Reset if sensor2 is touched before sensor1
	{
		firstCheckP = false;
		secondCheckP = true;
		canWinP = false;
		newLapP = false;
		printf("secondCheck Reseted\n");
		PositionPointsP--;
	}

	if ((bodyA == kart3->body && bodyB == sensor3) && secondCheckP == true) // If the sensor3 is touched
	{
		canWinP = true;
		firstCheckP = false;
		secondCheckP = false;
		newLapP = false;
		printf("canWin\n");
		PositionPointsP++;
	}
	else if ((bodyA == kart3->body && bodyB == sensor3) && newLapP == true) // Dont let the player take the firstCheck unless it goes through winLine
	{
		goThroughWinLineP = true;
		newLapP = false;
		printf("Go through winLine\n");
		PositionPointsP--;
	}

	// path colisions for the cpu karts
	if (bodyA == kart3->body)
	{
		if (bodyB == path1 || bodyB == path2 || bodyB == path3 || bodyB == path4 || bodyB == path5)
		{
			kart3->currentPathIndex = (kart3->currentPathIndex + 1) % 5; // change to next path
		}
	}
}

void ModuleGame::TextDraw()
{
	if (currentScreen == GAMEPLAY)
	{
		for (auto entity : entities)
		{
			if (entity != nullptr)
			{
				if (entity->body != nullptr)
				{
					DrawText(TextFormat("Current Time: %.2f\n", kart->currentTime), 460, 35, 28, RED);
					if (LapsM > 3)
					{
						if (kart->currentTime < bestTime)
						{
							bestTime = kart->currentTime;
						}
						kart->gameTimer.Start();
					}
					// Mario text 
					DrawText(TextFormat("Lap: %i / 3\n", LapsM), 80, 938, 30, RED);
					// Luigi text 
					DrawText(TextFormat("Lap: %i / 3\n", LapsL), 285, 938, 30, GREEN);
					if (LapsL > 3)
					{
						if (kart->currentTime < bestTime)
						{
							bestTime = kart->currentTime;
						}
						kart->gameTimer.Start();
					}
					DrawPodium();
				}
			}
		}
	}
    
	else if (currentScreen == RESULTS)
	{
		// Martí, aqui hauria de sortir totes les coses que vols que surtin a la pantalla de resultats, com el millor temps i podium.
		if (bestTime >= 1000000)
		{
			DrawText("Best Time: XX\n", 150, 400, 70, WHITE);
		}
		else
		{
			DrawText(TextFormat("Best Time: %.2f\n", bestTime), 150, 400, 70, WHITE);
		}
	}
}


void ModuleGame::DrawPodium()
{
	struct KartPosition
	{
		const char* name;
		int x;
		int y;
		int laps;
		int positionPoints;
		float distance;
		Color color;
	};

	KartPosition karts[] = {
		{"Mario", 0, 0, LapsM, PositionPointsM, 0.0f, RED},
		{"Luigi", 0, 0, LapsL, PositionPointsL, 0.0f, GREEN},
		{"Peach", 0, 0, LapsP, PositionPointsP, 0.0f, PINK}
	};

	// Get current karts positions
	kart->body->GetPhysicPosition(karts[0].x, karts[0].y);
	kart2->body->GetPhysicPosition(karts[1].x, karts[1].y);
	kart3->body->GetPhysicPosition(karts[2].x, karts[2].y);

	// Initial position of the circuit
	int startX = 916;
	int startY = 555;

	// Calculate the distance of each kart to the initial circuit position
	auto calculateDistance = [startX, startY](int x, int y) -> float {
		return sqrt((x - startX) * (x - startX) + (y - startY) * (y - startY));
		};

	karts[0].distance = calculateDistance(karts[0].x, karts[0].y);
	karts[1].distance = calculateDistance(karts[1].x, karts[1].y);
	karts[2].distance = calculateDistance(karts[2].x, karts[2].y);

	// Sort karts by laps, position points and distance
	std::sort(std::begin(karts), std::end(karts), [](const KartPosition& a, const KartPosition& b) {
		if (a.laps != b.laps)
		{
			return a.laps > b.laps; // More laps
		}
		if (a.positionPoints != b.positionPoints)
		{
			return a.positionPoints > b.positionPoints; // More position points
		}
		return a.distance > b.distance; // More distance
		});

	// Draw podium positions
	for (int i = 0; i < 3; ++i)
	{
		DrawText(TextFormat("%d. %s", i + 1, karts[i].name), 765, 25 + i * 35, 35, karts[i].color);
	}
}

void ModuleGame::engineSound()
{
	if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
	{
		App->audio->PlayFx(engine_fx);
	}

	if (kart->turboMReady == true)
	{
		App->audio->PlayFx(turbo_fx);
		kart->turboMReady = false;
	}

	if (IsKeyPressed(KEY_ENTER) && kart->turboLReady == true)
	{
		App->audio->PlayFx(turbo_fx);
		kart->turboLReady = false;	
	}
}

void ModuleGame::CleanEntities()
{
	for (auto entity : entities) 
	{
		if (entity != nullptr)
		{
			if (entity->body != nullptr)
			{
				App->physics->world->DestroyBody(entity->body->body);
				entity->body->body = nullptr;
			}
			delete entity;
		}
	}
	entities.clear();

	// Delete each sensor one by one, because they are not in the entities vector
	if (winLine != nullptr)
	{
		if (winLine->body != nullptr)
		{
			App->physics->world->DestroyBody(winLine->body);
			winLine->body = nullptr;
		}
		delete winLine;
		winLine = nullptr;
	}

	if (sensor1 != nullptr)
	{
		if (sensor1->body != nullptr)
		{
			App->physics->world->DestroyBody(sensor1->body);
			sensor1->body = nullptr;
		}
		delete sensor1;
		sensor1 = nullptr;
	}

	if (sensor2 != nullptr)
	{
		if (sensor2->body != nullptr)
		{
			App->physics->world->DestroyBody(sensor2->body);
			sensor2->body = nullptr;
		}
		delete sensor2;
		sensor2 = nullptr;
	}

	if (sensor3 != nullptr)
	{
		if (sensor3->body != nullptr)
		{
			App->physics->world->DestroyBody(sensor3->body);
			sensor3->body = nullptr;
		}
		delete sensor3;
		sensor3 = nullptr;
	}

	if (path1 != nullptr)
	{
		if (path1->body != nullptr)
		{
			App->physics->world->DestroyBody(path1->body);
			path1->body = nullptr;
		}
		delete path1;
		path1 = nullptr;
	}
	
	if (path2 != nullptr)
	{
		if (path2->body != nullptr)
		{
			App->physics->world->DestroyBody(path2->body);
			path2->body = nullptr;
		}
		delete path2;
		path2 = nullptr;
	}

	if (path3 != nullptr)
	{
		if (path3->body != nullptr)
		{
			App->physics->world->DestroyBody(path3->body);
			path3->body = nullptr;
		}
		delete path3;
		path3 = nullptr;
	}

	if (path4 != nullptr)
	{
		if (path4->body != nullptr)
		{
			App->physics->world->DestroyBody(path4->body);
			path4->body = nullptr;
		}
		delete path4;
		path4 = nullptr;
	}

	if (path5 != nullptr)
	{
		if (path5->body != nullptr)
		{
			App->physics->world->DestroyBody(path5->body);
			path5->body = nullptr;
		}
		delete path5;
		path5 = nullptr;
	}
}

