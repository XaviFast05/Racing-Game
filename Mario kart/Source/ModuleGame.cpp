#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

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
			if (turboTimer.ReadSec() > turboTime)
			{
				DrawText("TURBO", 250, 500, 50, BLACK);
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
		if (distance < 10.0f) // Umbral de colisi�n
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
	mario = LoadTexture("Assets/mario.png");
	luigi = LoadTexture("Assets/luigi.png");

	// Load sound fx
	lap_fx = App->audio->LoadFx("Assets/SOUND/FX/LapTakenFX.wav");
	finalLap_fx = App->audio->LoadFx("Assets/SOUND/FX/FinalLap.wav");
	engine_fx = App->audio->LoadFx("Assets/SOUND/FX/Engine Sounds/engine.wav");
	screenPass_fx = App->audio->LoadFx("Assets/SOUND/FX/ScreenPass.wav");

	// Load music
	titleMusic = LoadMusicStream("Assets/SOUND/MUSIC/MainMenu.ogg");
	controlsMusic = LoadMusicStream("Assets/SOUND/MUSIC/ControlsMenu.wav");
	circuitMusic = LoadMusicStream("Assets/SOUND/MUSIC/CircuitMusic.wav");
	circuitMusicFL = LoadMusicStream("Assets/SOUND/MUSIC/CircuitMusicFinalLap.wav");
	resultsMusic = LoadMusicStream("Assets/SOUND/MUSIC/Results.wav");
	SetMusicVolume(titleMusic, 0.09f);
	SetMusicVolume(controlsMusic, 0.09f);
	SetMusicVolume(circuitMusic, 0.09f);
	SetMusicVolume(circuitMusicFL, 0.09f);
	SetMusicVolume(resultsMusic, 0.5f);
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
	UnloadTexture(mario);
	UnloadTexture(luigi);

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
		DrawText("TITLE", 250, 500, 50, BLACK);
		
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
		DrawText("CONTROLS", 250, 500, 50, BLACK);
		if (IsKeyPressed(KEY_SPACE))
		{
			App->audio->PlayFx(screenPass_fx);
			currentScreen = GAMEPLAY;
		}
	}
	else if (currentScreen == GAMEPLAY) // Draw gameplay
	{
		
		// Play music
		if ((LapsM == 3 || LapsL == 3) && finalLap == true) UpdateMusicStream(circuitMusicFL);
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

			//load karts -------------------------------------
			entities.emplace_back(DBG_NEW Kart(App->physics, 916, 600, 4.71, this, mario, 0));
			entities.emplace_back(DBG_NEW Kart(App->physics, 952, 630, 4.71, this, luigi, 1));
			entities.emplace_back(DBG_NEW Kart(App->physics, 916, 660, 4.71, this, luigi, 2));
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
		TextDraw();
		engineSound();

		if (IsKeyPressed(KEY_P))
		{
			currentScreen = RESULTS;
			CleanEntities();
		}
	}
	else if (currentScreen == RESULTS) // Draw results
	{
		// Play music
		UpdateMusicStream(resultsMusic);
		// Draw results
		DrawText("RESULTS", 250, 500, 50, BLACK);
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
		LapsM++;
	}
	else if ((bodyA == kart->body && bodyB == winLine) && firstCheck == true) // Reset if lap is not completed
	{
		firstCheck = false;
		secondCheck = false;
		canWin = false;
		newLap = true;
		printf("canWin Reseted\n");
	}
	else if ((bodyA == kart->body && bodyB == winLine) && goThroughWinLine == true) // Reset if lap is not completed
	{
		newLap = true;
		goThroughWinLine = false;
		printf("newLap Reseted\n");
	}

	if ((bodyA == kart->body && bodyB == sensor1) && newLap == true) // If the sensor1 is touched
	{
		newLap = false;
		canWin = false;
		secondCheck = false;
		firstCheck = true;
		printf("First check\n");
	}
	else if ((bodyA == kart->body && bodyB == sensor1) && secondCheck == true) // Reset if sensor1 is touched before winLine
	{
		firstCheck = true;
		secondCheck = false;
		canWin = false;
		newLap = false;
		printf("firstCheck Reseted\n");
	}

	if ((bodyA == kart->body && bodyB == sensor2) && firstCheck == true) // If the sensor2 is touched
	{
		firstCheck = false;
		secondCheck = true;
		newLap = false;
		canWin = false;
		printf("Second check\n");
	}
	else if ((bodyA == kart->body && bodyB == sensor2) && canWin == true) // Reset if sensor2 is touched before sensor1
	{
		firstCheck = false;
		secondCheck = true;
		canWin = false;
		newLap = false;
		printf("secondCheck Reseted\n");
	}

	if ((bodyA == kart->body && bodyB == sensor3) && secondCheck == true) // If the sensor3 is touched
	{
		canWin = true;
		firstCheck = false;
		secondCheck = false;
		newLap = false;
		printf("canWin\n");
	}
	else if ((bodyA == kart->body && bodyB == sensor3) && newLap == true) // Dont let the player take the firstCheck unless it goes through winLine
	{
		goThroughWinLine = true;
		newLap = false;
		printf("Go through winLine\n");
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
	}
	else if ((bodyA == kart2->body && bodyB == winLine) && firstCheckL == true) // Reset if lap is not completed
	{
		firstCheckL = false;
		secondCheckL = false;
		canWinL = false;
		newLapL = true;
		printf("canWin Reseted\n");
	}
	else if ((bodyA == kart2->body && bodyB == winLine) && goThroughWinLineL == true) // Reset if lap is not completed
	{
		newLapL = true;
		goThroughWinLineL = false;
		printf("newLap Reseted\n");
	}

	if ((bodyA == kart2->body && bodyB == sensor1) && newLapL == true) // If the sensor1 is touched
	{
		newLapL = false;
		canWinL = false;
		secondCheckL = false;
		firstCheckL = true;
		printf("First check\n");
	}
	else if ((bodyA == kart2->body && bodyB == sensor1) && secondCheckL == true) // Reset if sensor1 is touched before winLine
	{
		firstCheckL = true;
		secondCheckL = false;
		canWinL = false;
		newLapL = false;
		printf("firstCheck Reseted\n");
	}

	if ((bodyA == kart2->body && bodyB == sensor2) && firstCheckL == true) // If the sensor2 is touched
	{
		firstCheckL = false;
		secondCheckL = true;
		newLapL = false;
		canWinL = false;
		printf("Second check\n");
	}
	else if ((bodyA == kart2->body && bodyB == sensor2) && canWinL == true) // Reset if sensor2 is touched before sensor1
	{
		firstCheckL = false;
		secondCheckL = true;
		canWinL = false;
		newLapL = false;
		printf("secondCheck Reseted\n");
	}

	if ((bodyA == kart2->body && bodyB == sensor3) && secondCheckL == true) // If the sensor3 is touched
	{
		canWinL = true;
		firstCheckL = false;
		secondCheckL = false;
		newLapL = false;
		printf("canWin\n");
	}
	else if ((bodyA == kart2->body && bodyB == sensor3) && newLapL == true) // Dont let the player take the firstCheck unless it goes through winLine
	{
		goThroughWinLineL = true;
		newLapL = false;
		printf("Go through winLine\n");
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
	// Draw text for Mario
	DrawText(TextFormat("Lap: %i / 3\n", LapsM), 10, 900, 50, RED);
	if (bestTime >= 1000000)
	{
		DrawText(TextFormat("Best Time: XX\n"), 10, 850, 50, RED);
	}
	else
	{
		DrawText(TextFormat("Best Time: %.2f\n", bestTime), 10, 850, 50, RED);
	}

	DrawText(TextFormat("Current Time: %.2f\n", kart->currentTime), 10, 800, 50, RED);
	if (LapsM > 3)
	{
		if (kart->currentTime < bestTime)
		{
			bestTime = kart->currentTime;
		}
		kart->gameTimer.Start();
		LapsM = 1;
		LapsL = 1;
	}
	// Draw text for Luigi
	DrawText(TextFormat("Lap: %i / 3\n", LapsL), 10, 950, 50, GREEN);
	if (LapsL > 3)
	{
		if (kart->currentTime < bestTime)
		{
			bestTime = kart->currentTime;
		}
		kart->gameTimer.Start();
		LapsM = 1;
		LapsL = 1;
	}
}

void ModuleGame::engineSound()
{
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
	{
		App->audio->PlayFx(engine_fx);
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
}