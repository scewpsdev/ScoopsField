
#define JUMP_BUFFER 0.3f
#define DUCK_TRANSITION 0.15f


static void OnStep(Player* player)
{
	PlaySound(&game->stepSounds[game->random.next() % 6], (player->lastStepIdx % 2 * 2 - 1) * 0.2f, 0.5f);
}

static void OnLand(Player* player)
{
	PlaySound(&game->landSound, 0.5f);
}

static vec3 GetMovementInputs(Player* player, Action* currentAction, float speed)
{
	vec3 fsu = vec3::Zero;

	bool inputLeft = GetKey(SDL_SCANCODE_A);
	bool inputRight = GetKey(SDL_SCANCODE_D);
	bool inputDown = GetKey(SDL_SCANCODE_S);
	bool inputUp = GetKey(SDL_SCANCODE_W);

	if (inputLeft)
		fsu.x--;
	if (inputRight)
		fsu.x++;
	if (inputDown)
		fsu.z--;
	if (inputUp)
		fsu.z++;

	if (fsu.lengthSquared() > 0.0f)
	{
		fsu = fsu.normalized();
		fsu *= speed;
	}

	return fsu;
}

static vec3 friction(const vec3& velocity, bool ducked)
{
	float entityFriction = 1.0f;
	float edgeFriction = 1.0f;
#define FRICTION 6.0f
	float fric = FRICTION * (ducked ? 2 : 1) * entityFriction * edgeFriction; // sv_friction * ke * ef

	float l = velocity.length();
	vec3 vn = velocity / l;

#define STOP_SPEED 1.0f
	if (l >= STOP_SPEED)
		return (1.0f - deltaTime * fric) * velocity;
	else if (l >= max(0.01f, deltaTime * STOP_SPEED * fric) && l < STOP_SPEED)
		return velocity - deltaTime * STOP_SPEED * fric * vn;
	else // if (l < Mathf.Max(0.1f, frametime * STOP_SPEED * fric)
		return vec3::Zero;
}

static vec3 updateVelocityGround(vec3& velocity, const vec3& wishdir, float maxSpeed, bool ducked, const vec3& forward, const vec3& right, const vec3& up)
{
#define GRAVITY -20
	velocity.y = velocity.y + 0.5f * GRAVITY * deltaTime;

	vec3 accel = wishdir.x * right + wishdir.y * up + wishdir.z * forward;
	float accelMag = accel.length();
	vec3 accelDir = accelMag > 0.0f ? accel / accelMag : vec3::Zero;

	float entityFriction = 1.0f;

	velocity = friction(velocity, ducked);
	float m = min(maxSpeed, wishdir.length());
	float currentSpeed = dot(velocity, accelDir);
	float l = m;
#define ACCELERATION 10.0f
	float addSpeed = clamp(l - currentSpeed, 0.0f, entityFriction * deltaTime * m * ACCELERATION);

	velocity = velocity + accelDir * addSpeed;

	velocity.y = velocity.y + 0.5f * GRAVITY * deltaTime;

	return velocity;
}
static vec3 updateVelocityAir(vec3& velocity, const vec3& wishdir, float maxSpeed, const vec3& forward, const vec3& right, const vec3& up)
{
	velocity.y = velocity.y + 0.5f * GRAVITY * deltaTime;

	vec3 accel = wishdir.x * right + wishdir.y * up + wishdir.z * forward;
	float accelMag = accel.length();
	vec3 accelDir = accelMag > 0.0f ? accel / accelMag : vec3::Zero;

	float entityFriction = 1.0f;

	float m = min(maxSpeed, wishdir.length());
	float currentSpeed = dot(velocity, accelDir);
#define MAX_AIR_SPEED 0.3f
	float l = min(m, MAX_AIR_SPEED);
#define AIR_ACCELERATION 10.0f
	float addSpeed = clamp(l - currentSpeed, 0.0f, entityFriction * deltaTime * m * AIR_ACCELERATION);

	velocity = velocity + accelDir * addSpeed;

	velocity.y = velocity.y + 0.5f * GRAVITY * deltaTime;

	return velocity;
}

static void SetDuckState(Player* player, bool ducked)
{
	if (ducked && !player->ducked)
	{
		player->ducked = true;
		ResizeCharacterController(&player->controller, CONTROLLER_HEIGHT_DUCKED);
	}
	else if (!ducked && player->ducked)
	{
		player->ducked = false;
		ResizeCharacterController(&player->controller, CONTROLLER_HEIGHT);
	}

	player->duckTimer = -1;
}

static void UpdateDuckState(Player* player)
{
	bool inputDuck = GetKey(SDL_SCANCODE_LCTRL);
	if (inputDuck)
	{
		if (player->duckTimer == -1.0f)
		{
			if (!player->ducked)
			{
				if (player->grounded)
					player->duckTimer = 0.0f;
				else
				{
					SetDuckState(player, true);
					MoveCharacterController(&player->controller, vec3::Up * (CONTROLLER_HEIGHT - CONTROLLER_HEIGHT_DUCKED), 1);
				}
			}
		}
		else
		{
			player->duckTimer += deltaTime;
			if (/*!player->grounded ||*/ player->duckTimer >= DUCK_TRANSITION)
			{
				SetDuckState(player, true);
				/*
				if (!player->grounded)
				{
					MoveCharacterController(&player->controller, vec3::Up * (CONTROLLER_HEIGHT - CONTROLLER_HEIGHT_DUCKED), 0);
				}
				*/
				//player->ducked = true;
			}
		}
	}
	else
	{
		if (player->ducked)
		{
			PhysicsHit hits[16];
			int numHits = SweepSphere(physics, player->controller.getRadius(), player->position + vec3(0.0f, player->controller.getHeight() - player->controller.getRadius(), 0.0f), vec3::Up, 0.5f, hits, 16, ENTITY_FILTER_DEFAULT);

			bool headBlocked = false;
			for (int i = 0; i < numHits; i++)
			{
				if (!hits[i].trigger)
				{
					headBlocked = true;
					break;
				}
			}

			if (!headBlocked)
			{
				SetDuckState(player, false);

				if (!player->grounded)
				{
					MoveCharacterController(&player->controller, vec3::Up * (CONTROLLER_HEIGHT_DUCKED - CONTROLLER_HEIGHT), 1);
				}


				/*
				player->duckTimer = -1.0f;

				if (!player->grounded)
				{
				}
				*/
			}
		}
		else if (player->duckTimer != -1)
		{
			SetDuckState(player, false);

			if (!player->grounded)
			{
				MoveCharacterController(&player->controller, vec3::Up * (CONTROLLER_HEIGHT_DUCKED - CONTROLLER_HEIGHT), 0);
			}

			//player->duckTimer = -1;
			//player->ducked = false;
		}
	}

	if (player->duckTimer >= 0.0f)
	{
		/*
		player->duckTimer += deltaTime;
		if (!player->grounded || player->duckTimer >= DUCK_TRANSITION)
		{
			SetDuckState(player, true);
			//player->ducked = true;
		}
		*/
	}

	if (player->ducked)
	{
		//ResizeCharacterController(&player->controller, CONTROLLER_HEIGHT_DUCKED);
		//if (player->grounded)
		//	controller.offset = Vector3.Zero;
		//else
		//	controller.offset = new Vector3(0, CONTROLLER_HEIGHT - CONTROLLER_HEIGHT_DUCKED, 0);
	}
	else
	{
		//ResizeCharacterController(&player->controller, CONTROLLER_HEIGHT);
		//controller.offset = Vector3.Zero;
	}
}

static void SourceMovement(Player* player, vec3 extraDisplacement)
{
	Action* currentAction = GetCurrentAction(player);

	player->sprinting = false;

	quat playerRotation = quat::FromAxisAngle(vec3::Up, player->rotation);
	vec3 forward = playerRotation.forward();
	vec3 right = playerRotation.right();
	vec3 up = playerRotation.up();

	player->sprinting = GetKey(SDL_SCANCODE_LSHIFT) && player->stamina > 0 && !player->exhausted;
	float speed = (player->sprinting ? 1.5f : GetKey(SDL_SCANCODE_LALT) ? 0.25f : 1) * player->walkSpeed;
	if (player->sprinting)
		player->stamina -= 0.15f * deltaTime;
	if (currentAction)
		speed *= currentAction->moveSpeed;

	UpdateDuckState(player);

	vec3 fsu = GetMovementInputs(player, currentAction, speed);

	if (GetKeyDown(SDL_SCANCODE_SPACE))
		player->lastJumpInput = gameTime;
	if (player->grounded && player->lastJumpInput && gameTime - player->lastJumpInput < JUMP_BUFFER)
	{
		const float jumpPower = 7;
		player->velocity.y = jumpPower;
		player->grounded = false;
		player->lastJumpInput = 0;
	}

	if (player->grounded)
		updateVelocityGround(player->velocity, fsu, speed, player->ducked, forward, right, up);
	else
		updateVelocityAir(player->velocity, fsu, speed, forward, right, up);

	vec3 displacement = player->velocity * deltaTime + extraDisplacement;
	float xzSpeed = player->velocity.xz().length();

	if (player->grounded)
		player->distanceWalked += xzSpeed * deltaTime;

	int stepIdx = (int)(player->distanceWalked * STEP_FREQUENCY);
	if (stepIdx != player->lastStepIdx)
	{
		OnStep(player);
		player->lastStepIdx = stepIdx;
	}

	ControllerCollisionFlags collisionFlags = MoveCharacterController(&player->controller, displacement, ENTITY_FILTER_DEFAULT);
	vec3 newPosition = GetCharacterControllerPosition(&player->controller);

	if (collisionFlags & CONTROLLER_COLLISION_DOWN)
	{
		if (player->velocity.y < -5)
		{
			player->lastLandedTime = gameTime;
			OnLand(player);
		}

		if (newPosition.y < player->position.y)
		{
			vec3 movedDisplacement = newPosition - player->position;
			vec3 movedVelocity = movedDisplacement / deltaTime;

			player->velocity.y = movedVelocity.y; //max(player->velocity.y, -player->velocity.xz().length());
		}
		else
		{
			player->velocity.y = 0;
		}

		player->grounded = true;
	}
	else
	{
		player->grounded = false;
	}

	player->moving = xzSpeed > 1;

	player->position = newPosition;
	SetRigidBodyTransform(&player->kinematicBody, player->position, quat::Identity);
}
