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

protected:
	PhysBody* body;
	Module* listener;
};

class Kart : public PhysicEntity
{
public:
	Kart(ModulePhysics* physics, int _x, int _y, Module* _listener, Texture2D _texture)
		: PhysicEntity(physics->CreateRectangle(_x, _y, 30, 25), _listener)
		, texture(_texture)
	{
	}
	const float forceMagnitude = 1.0f;
	const float maxSpeed = 10.0f;
	const float maxRotation = 10.0f;

	const float inicialAngularSpeed = 0.15f;
	const float angularAcceleration = 0.1f;
	float forceRotation = 0.0f;

	Timer rotationTimer;

	void Update() override
	{
		int x, y;
		body->GetPhysicPosition(x, y);

		if (IsKeyDown(KEY_W)) // Adelante
		{
			b2Vec2 force();
			body->body->ApplyForceToCenter(body->body->GetTransform().q.GetXAxis(), true);
			//body->body->GetAngle()
		}
		if (IsKeyDown(KEY_S)) // Atrás
		{
			//force.y += forceMagnitude / 5;
		}

		body->body->SetAngularVelocity(forceRotation);
		
		if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D))
		{
			rotationTimer.Start();
		}

		if (IsKeyDown(KEY_A) && forceRotation > -maxRotation) // Izquierda
		{
			forceRotation -= (inicialAngularSpeed + angularAcceleration * rotationTimer.ReadSec());
		}
		if (IsKeyDown(KEY_D) && forceRotation < maxRotation) // Derecha
		{
			forceRotation += (inicialAngularSpeed + angularAcceleration * rotationTimer.ReadSec());
		}
		if (IsKeyUp(KEY_A) && IsKeyUp(KEY_D))
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

		// Aplicar la fuerza al cuerpo físico
		//body->body->ApplyForceToCenter(force, true);
		
		Vector2 position{ (float)x, (float)y };
		float scale = 0.3f;
		Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
		Rectangle dest = { position.x, position.y, (float)texture.width * scale, (float)texture.height * scale };
		Vector2 origin = { (float)texture.width / 6.2f, (float)texture.height / 6.3f };
		float rotation = body->GetRotation() * RAD2DEG;
		DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
	}

	int RayHit(vec2<int> ray, vec2<int> mouse, vec2<float>& normal) override
	{
		return body->RayCast(ray.x, ray.y, mouse.x, mouse.y, normal.x, normal.y);;
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
	sensed = false;
}

ModuleGame::~ModuleGame()
{}

// Load assets
bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	circuit = LoadTexture("Assets/circuit.png");
	mario = LoadTexture("Assets/mario.png");
	
	bonus_fx = App->audio->LoadFx("Assets/bonus.wav");

	//load sensors
	sensor = App->physics->CreateRectangleSensor(915, 555, 210, 9);

	//load karts
	entities.emplace_back(DBG_NEW Kart(App->physics, 500, 550, this, mario));
	kart = dynamic_cast<Kart*>(entities[0]);

	//load walls
	entities.emplace_back(DBG_NEW InteriorWall(App->physics, 0, 0, this, NoTexture));
	entities.emplace_back(DBG_NEW ExteriorWall(App->physics, 0, 0, this, NoTexture));
	interior = dynamic_cast<InteriorWall*>(entities[1]);
	exterior = dynamic_cast<ExteriorWall*>(entities[2]);
	App->renderer->camera.x = App->renderer->camera.y = 0;
	return ret;
}

// Load assets
bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");
	delete kart;
	delete interior;
	delete exterior;
	delete sensor;
	return true;
}

// Update: draw background
update_status ModuleGame::Update()
{
	float scaleX = (float)GetScreenWidth() / circuit.width;
	float scaleY = (float)GetScreenHeight() / circuit.height;
	DrawTextureEx(circuit, { 0, 0 }, 0.0f, fmax(scaleX, scaleY), WHITE);

	// Actualizar todas las entidades
	for (PhysicEntity* entity : entities)
	{
		entity->Update();
	}

	if(IsKeyPressed(KEY_SPACE))
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
	if(ray_on == true)
	{
		vec2f destination((float)(mouse.x-ray.x), (float)(mouse.y-ray.y));
		destination.Normalize();
		destination *= (float)ray_hit;

		DrawLine(ray.x, ray.y, (int)(ray.x + destination.x), (int)(ray.y + destination.y), RED);

		if (normal.x != 0.0f)
		{
			DrawLine((int)(ray.x + destination.x), (int)(ray.y + destination.y), (int)(ray.x + destination.x + normal.x * 25.0f), (int)(ray.y + destination.y + normal.y * 25.0f), Color{ 100, 255, 100, 255 });
		}
	}

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	App->audio->PlayFx(bonus_fx);
}
