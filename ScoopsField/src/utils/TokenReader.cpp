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
