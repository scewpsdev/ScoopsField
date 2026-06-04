#pragma once


enum TokenType
{
	TOKEN_TYPE_NULL = 0,

	TOKEN_TYPE_STRING,
	TOKEN_TYPE_INTEGER,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_SYMBOL,
	TOKEN_TYPE_IDENTIFIER,

	TOKEN_TYPE_COUNT,
};

struct Token
{
	TokenType type;
	int start;
	int end;
};

struct TokenReader
{
	char* data;
	int size;
	int pos;
};


void InitTokenReader(TokenReader* reader, char* data, int size);

bool HasNext(TokenReader* reader);
Token Next(TokenReader* reader);
Token Next(TokenReader* reader, TokenType type);
Token Next(TokenReader* reader, const char* value);
Token Next(TokenReader* reader, char value);
Token Peek(TokenReader* reader);
bool CheckTokenValue(TokenReader* reader, Token* token, const char* value);
bool CheckTokenValue(TokenReader* reader, Token* token, char value);
bool NextIsValue(TokenReader* reader, const char* value);
bool NextIsValue(TokenReader* reader, char value);
