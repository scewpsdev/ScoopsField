

static void ReadCollider(TokenReader* reader, Creature* creature, const char* path)
{
	char bone[64];
	ColliderType type;
	bool trigger;
	vec3 size;
	vec3 offset;
	vec3 rotation;

	while (!NextIsValue(reader, '}'))
	{
		Token name = Next(reader, TOKEN_TYPE_IDENTIFIER);
		Next(reader, '='); // =

		if (CheckTokenValue(reader, &name, "bone"))
			ReadString(reader, bone, 64);
		else if (CheckTokenValue(reader, &name, "type"))
		{
			Token value = Next(reader);
			if (CheckTokenValue(reader, &value, "Box"))
				type = COLLIDER_TYPE_BOX;
			else if (CheckTokenValue(reader, &value, "Sphere"))
				type = COLLIDER_TYPE_SPHERE;
			else if (CheckTokenValue(reader, &value, "Capsule"))
				type = COLLIDER_TYPE_CAPSULE;
			else
			{
				SDL_assert(false);
			}
		}
		else if (CheckTokenValue(reader, &name, "trigger"))
			ReadBool(reader, &trigger);
		else if (CheckTokenValue(reader, &name, "size"))
			ReadVec3(reader, &size);
		else if (CheckTokenValue(reader, &name, "offset"))
			ReadVec3(reader, &offset);
		else if (CheckTokenValue(reader, &name, "rotation"))
			ReadVec3(reader, &rotation);
		else
		{
			const char* tokenString = &reader->data[name.start];
			SDL_assert(false);
			ReadValue(reader);
		}
	}

	if (Node* node = GetNodeByName(creature->model, bone))
	{
		mat4 transform = ModelMatrix((Entity*)creature) * CalculateNodeDefaultWorldTransform(creature->model, node);

		RigidBody* hitbox = HashMapAdd(&creature->hitboxes, hash(bone), {});
		InitRigidBody(hitbox, RIGID_BODY_KINEMATIC, transform.translation(), transform.rotation(), creature);
		if (type == COLLIDER_TYPE_BOX)
			AddBoxCollider(hitbox, size, offset, quat::FromEulers(rotation), ENTITY_FILTER_ENEMY_HITBOX, 0, false);
		else if (type == COLLIDER_TYPE_CAPSULE)
			AddCapsuleCollider(hitbox, 0.5f * size.x, size.y, offset, quat::FromEulers(rotation), ENTITY_FILTER_ENEMY_HITBOX, 0, false);
		else if (type == COLLIDER_TYPE_SPHERE)
			AddSphereCollider(hitbox, 0.5f * size.x, offset, ENTITY_FILTER_ENEMY_HITBOX, 0, false);
		else
		{
			SDL_assert(false);
		}
	}
}

static void ReadEntity(TokenReader* reader, Creature* creature, const char* path)
{
	while (Peek(reader).type == TOKEN_TYPE_IDENTIFIER)
	{
		Token name = Next(reader, TOKEN_TYPE_IDENTIFIER);
		Next(reader, '='); // =
		if (CheckTokenValue(reader, &name, "boneColliders"))
		{
			Next(reader, '['); // [

			bool hasNext = NextIsValue(reader, '{');
			while (hasNext)
			{
				Next(reader, '{'); // {

				ReadCollider(reader, creature, path);

				Next(reader, '}'); // }

				hasNext = NextIsValue(reader, ',');
				if (hasNext)
					Next(reader, ','); // ,
			}

			Next(reader, ']'); // ]
		}
		else
		{
			ReadValue(reader);
		}
	}
}

void LoadCreatureHitbox(Creature* creature, const char* path)
{
	char fullPath[256];
	SDL_snprintf(fullPath, 256, "res/%s.bin", path);

	size_t fileSize;
	if (void* data = SDL_LoadFile(fullPath, &fileSize))
	{
		TokenReader reader = {};
		InitTokenReader(&reader, (char*)data, (int)fileSize);

		Next(&reader, TOKEN_TYPE_IDENTIFIER); // entities
		Next(&reader, '='); // =
		Next(&reader, '['); // [
		Next(&reader, '{'); // {

		ReadEntity(&reader, creature, path);

		Next(&reader, '}'); // }
		Next(&reader, ']'); // ]

		SDL_free(data);
	}
}
