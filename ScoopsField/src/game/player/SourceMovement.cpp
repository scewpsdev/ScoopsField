
#define JUMP_BUFFER 0.3f


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

static vec3 friction(const vec3& velocity)
{
	float entityFriction = 1.0f;
	float edgeFriction = 1.0f;
#define FRICTION 6.0f
	float fric = FRICTION * entityFriction * edgeFriction; // sv_friction * ke * ef

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

static vec3 updateVelocityGround(vec3& velocity, const vec3& wishdir, float maxSpeed, const vec3& forward, const vec3& right, const vec3& up)
{
#define GRAVITY -20
	velocity.y = velocity.y + 0.5f * GRAVITY * deltaTime;

	vec3 accel = wishdir.x * right + wishdir.y * up + wishdir.z * forward;
	float accelMag = accel.length();
	vec3 accelDir = accelMag > 0.0f ? accel / accelMag : vec3::Zero;

	float entityFriction = 1.0f;

	velocity = friction(velocity);
	float m = min(maxSpeed, wishdir.length());
	float currentSpeed = dot(velocity, accelDir);
	float l = m;
#define ACCELERATION 10.0f
	float addSpeed = clamp(l - currentSpeed, 0.0f, entityFriction * deltaTime * m * ACCELERATION);

	velocity = velocity + accelDir * addSpeed;

	velocity.y = velocity.y + 0.5f * GRAVITY * deltaTime;

	return velocity;
}

vec3 updateVelocityAir(vec3& velocity, const vec3& wishdir, float maxSpeed, const vec3& forward, const vec3& right, const vec3& up)
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

static void SourceMovement(Player* player)
{
	Action* currentAction = GetCurrentAction(player);

	player->sprinting = false;
	if (player->cameraMode == CAMERA_MODE_FIRST_PERSON)
	{
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
			updateVelocityGround(player->velocity, fsu, speed, forward, right, up);
		else
			updateVelocityAir(player->velocity, fsu, speed, forward, right, up);

		vec3 displacement = player->velocity * deltaTime;

		if (player->grounded)
			player->distanceWalked += displacement.length();

		int stepIdx = (int)(player->distanceWalked * STEP_FREQUENCY);
		if (stepIdx != player->lastStepIdx)
		{
			OnStep(player);
			player->lastStepIdx = stepIdx;
		}

		ControllerCollisionFlags collisionFlags = MoveCharacterController(&player->controller, displacement, ENTITY_FILTER_DEFAULT);
		if (collisionFlags & CONTROLLER_COLLISION_DOWN)
		{
			if (player->velocity.y < -1)
			{
				player->lastLandedTime = gameTime;
				OnLand(player);
			}
			player->velocity.y = 0; //max(player->velocity.y, -player->velocity.xz().length());
			player->grounded = true;
		}
		else
		{
			player->grounded = false;
		}

		player->moving = player->velocity.lengthSquared() > 1;

		player->position = GetCharacterControllerPosition(&player->controller);
		SetRigidBodyTransform(&player->kinematicBody, player->position, quat::Identity);

		game->cameraPosition = player->position + vec3::Up * 1.5f;
	}
	else
	{
		quat cameraRotation = quat::FromAxisAngle(vec3::Up, game->cameraYaw) * quat::FromAxisAngle(vec3::Right, game->cameraPitch);

		vec3 delta = vec3::Zero;
		if (GetKey(SDL_SCANCODE_A)) delta += cameraRotation.left();
		if (GetKey(SDL_SCANCODE_D)) delta += cameraRotation.right();
		if (GetKey(SDL_SCANCODE_S)) delta += cameraRotation.back();
		if (GetKey(SDL_SCANCODE_W)) delta += cameraRotation.forward();
		if (GetKey(SDL_SCANCODE_SPACE)) delta += vec3::Up;
		if (GetKey(SDL_SCANCODE_LCTRL)) delta += vec3::Down;

		if (delta.lengthSquared() > 0)
		{
			float speed = (GetKey(SDL_SCANCODE_LSHIFT) ? 2 : GetKey(SDL_SCANCODE_LALT) ? 0.25f : 1) * player->walkSpeed;
			vec3 velocity = delta.normalized() * speed;
			vec3 displacement = velocity * deltaTime;
			game->cameraPosition += displacement;
		}

		player->moving = delta.lengthSquared() > 0;
	}
}
