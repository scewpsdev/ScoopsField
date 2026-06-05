#include "TokenReader.h"

#include <SDL3/SDL.h>


void InitTokenReader(TokenReader* reader, char* data, int size)
{
	reader->data = data;
	reader->size = size;
	reader->pos = 0;
}

bool HasNext(TokenReader* reader)
{
	return reader->pos < reader->size;
}

static bool NextIsWhitespace(TokenReader* reader)
{
	char c = reader->data[reader->pos];
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\b';
}

static void SkipWhitespace(TokenReader* reader)
{
	while (NextIsWhitespace(reader))
	{
		reader->pos++;
	}
}

static bool ReadString(TokenReader* reader, Token* token)
{
	if (reader->data[reader->pos] != '"')
		return false;

	token->type = TOKEN_TYPE_STRING;
	token->start = reader->pos;

	reader->pos++; // "

	while (reader->pos < reader->size && reader->data[reader->pos] != '"')
	{
		reader->pos++;
	}

	reader->pos++; // "

	token->end = reader->pos;

	return true;
}

static bool ReadNumber(TokenReader* reader, Token* token)
{
	char c = reader->data[reader->pos];
	char c2 = reader->data[reader->pos + 1];

	bool isInteger = SDL_isdigit(c) || c == '-' && SDL_isdigit(c2);
	if (!isInteger)
		return false;

	token->type = TOKEN_TYPE_INTEGER;
	token->start = reader->pos;

	while (true)
	{
		char c = SDL_tolower(reader->data[reader->pos]);
		if (SDL_isdigit(c) || c == 'x' || c == 'b' || c == 'o' || c >= 'a' && c <= 'f' || c == 'e' || c == 'u' || c == 'l' || c == '-' || c == '_' || c == '.')
		{
			reader->pos++;
			if (c == '.')
				token->type = TOKEN_TYPE_FLOAT;
		}
		else
		{
			break;
		}
	}

	token->end = reader->pos;

	return true;
}

static bool ReadSymbol(TokenReader* reader, Token* token)
{
	char c = reader->data[reader->pos];
	if (c == '.'
		|| c == ','
		|| c == ':'
		|| c == ';'
		|| c == '#'
		|| c == '('
		|| c == ')'
		|| c == '{'
		|| c == '}'
		|| c == '['
		|| c == ']'
		|| c == '+'
		|| c == '-'
		|| c == '*'
		|| c == '/'
		|| c == '%'
		|| c == '&'
		|| c == '|'
		|| c == '^'
		|| c == '<'
		|| c == '>'
		|| c == '!'
		|| c == '?'
		|| c == '$'
		|| c == '=')
	{
		token->type = TOKEN_TYPE_SYMBOL;
		token->start = reader->pos;

		reader->pos++;

		token->end = reader->pos;

		return true;
	}

	return false;
}

static bool ReadIdentifier(TokenReader* reader, Token* token)
{
	char c = reader->data[reader->pos];
	bool isIdentifier = SDL_isalpha(c) || c == '_';
	if (!isIdentifier)
		return false;

	token->type = TOKEN_TYPE_IDENTIFIER;
	token->start = reader->pos;

	while (true)
	{
		char c = reader->data[reader->pos];
		if (SDL_isalpha(c) || SDL_isdigit(c) || c == '_')
		{
			reader->pos++;
		}
		else
		{
			break;
		}
	}

	token->end = reader->pos;

	return true;
}

Token Next(TokenReader* reader)
{
	SkipWhitespace(reader);

	if (reader->pos < reader->size)
	{
		Token token;
		if (ReadString(reader, &token))
			return token;
		if (ReadNumber(reader, &token))
			return token;
		if (ReadSymbol(reader, &token))
			return token;
		if (ReadIdentifier(reader, &token))
			return token;

		SDL_assert(false);
		reader->pos++;
		return {};
	}

	return {};
}

Token Next(TokenReader* reader, TokenType type)
{
	Token token = Next(reader);
	SDL_assert(token.type == type);
	return token;
}

Token Next(TokenReader* reader, const char* value)
{
	Token token = Next(reader);
	SDL_assert(CheckTokenValue(reader, &token, value));
	return token;
}

Token Next(TokenReader* reader, char value)
{
	Token token = Next(reader);
	SDL_assert(CheckTokenValue(reader, &token, value));
	return token;
}

Token Peek(TokenReader* reader)
{
	int pos = reader->pos;
	Token token = Next(reader);
	reader->pos = pos;
	return token;
}

bool CheckTokenValue(TokenReader* reader, Token* token, const char* value)
{
	int length = (int)SDL_strlen(value);
	if (length != token->end - token->start)
		return false;

	char* tokenString = &reader->data[token->start];
	return SDL_strncmp(tokenString, value, length) == 0;
}

bool CheckTokenValue(TokenReader* reader, Token* token, char value)
{
	return token->end - token->start == 1 && reader->data[token->start] == value;
}

bool NextIsValue(TokenReader* reader, const char* value)
{
	Token token = Peek(reader);
	return CheckTokenValue(reader, &token, value);
}

bool NextIsValue(TokenReader* reader, char value)
{
	Token token = Peek(reader);
	return CheckTokenValue(reader, &token, value);
}


void ReadValue(TokenReader* reader);

void ReadArray(TokenReader* reader)
{
	Next(reader); // [

	bool hasNext = !NextIsValue(reader, ']');
	while (hasNext)
	{
		ReadValue(reader);
		hasNext = NextIsValue(reader, ',');
		if (hasNext)
			Next(reader); // ,
	}

	Next(reader); // ]
}

void ReadObject(TokenReader* reader)
{
	Next(reader); // {

	while (!NextIsValue(reader, '}'))
	{
		Next(reader, TOKEN_TYPE_IDENTIFIER);
		Next(reader, TOKEN_TYPE_SYMBOL); // =

		ReadValue(reader);
	}

	Next(reader); // }
}

void ReadValue(TokenReader* reader)
{
	Token token = Peek(reader);
	if (token.type == TOKEN_TYPE_STRING || token.type == TOKEN_TYPE_INTEGER || token.type == TOKEN_TYPE_FLOAT || token.type == TOKEN_TYPE_IDENTIFIER)
	{
		Next(reader);
	}
	else if (token.type == TOKEN_TYPE_SYMBOL)
	{
		if (CheckTokenValue(reader, &token, '['))
		{
			ReadArray(reader);
		}
		else if (CheckTokenValue(reader, &token, '{'))
		{
			ReadObject(reader);
		}
	}
	else
	{
		SDL_assert(false);
	}
}

static int64_t ParseInteger(char* str, int size)
{
	int64_t value = 0;
	bool negative = false;

	for (int i = 0; i < size; i++)
	{
		char c = str[i];
		if (c == '-')
			negative = true;
		else if (SDL_isdigit(c))
			value = value * 10 + (c - '0');
		else
		{
			SDL_assert(false);
		}
	}

	if (negative)
		value = -value;

	return value;
}

static double ParseFloat(char* str, int size)
{
	SDL_assert(size < 32);

	char processed[32];
	SDL_memset(processed, 0, sizeof(processed));
	SDL_memcpy(processed, str, size);

	return SDL_atof(processed);
}

void ReadFloat(TokenReader* reader, float* value)
{
	Token token = Next(reader);
	SDL_assert(token.type == TOKEN_TYPE_FLOAT || token.type == TOKEN_TYPE_INTEGER);

	if (token.type == TOKEN_TYPE_FLOAT)
		*value = (float)ParseFloat(&reader->data[token.start], token.end - token.start);
	else if (token.type == TOKEN_TYPE_INTEGER)
		*value = (float)ParseInteger(&reader->data[token.start], token.end - token.start);
}

void ReadInteger(TokenReader* reader, int* value)
{
	Token token = Next(reader);
	SDL_assert(token.type == TOKEN_TYPE_INTEGER);
	*value = (int)ParseInteger(&reader->data[token.start], token.end - token.start);
}

void ReadBool(TokenReader* reader, bool* value)
{
	int i;
	ReadInteger(reader, &i);
	*value = i;
}

void ReadIVec2(TokenReader* reader, ivec2* value)
{
	Next(reader, '['); // [

	ReadInteger(reader, &value->x);
	Next(reader, ','); // ,
	ReadInteger(reader, &value->y);

	Next(reader, ']'); // ]
}

void ReadVec3(TokenReader* reader, vec3* value)
{
	Next(reader, '['); // [

	ReadFloat(reader, &value->x);
	Next(reader, ','); // ,
	ReadFloat(reader, &value->y);
	Next(reader, ','); // ,
	ReadFloat(reader, &value->z);

	Next(reader, ']'); // ]
}

void ReadVec4(TokenReader* reader, vec4* value)
{
	Next(reader, '['); // [

	ReadFloat(reader, &value->x);
	Next(reader, ','); // ,
	ReadFloat(reader, &value->y);
	Next(reader, ','); // ,
	ReadFloat(reader, &value->z);
	Next(reader, ','); // ,
	ReadFloat(reader, &value->w);

	Next(reader, ']'); // ]
}

void ReadString(TokenReader* reader, char* str, int maxLen)
{
	Token token = Next(reader);
	SDL_assert(token.type == TOKEN_TYPE_STRING);
	const char* tokenString = &reader->data[token.start + 1];
	int size = token.end - token.start - 2;
	SDL_assert(size < maxLen - 1);
	SDL_memcpy(str, tokenString, size);
	str[size] = 0;
}
