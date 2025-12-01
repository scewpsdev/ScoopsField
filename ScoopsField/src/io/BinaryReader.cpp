#include "BinaryReader.h"

#include <SDL3/SDL_assert.h>

#include <string.h>
#include <string>
#include <locale>
#include <codecvt>


BinaryReader::BinaryReader(unsigned char* buffer, int length)
{
	this->buffer = buffer;
	this->length = length;
	this->pos = 0;
}

BinaryReader::~BinaryReader()
{
	if (this->ownsBuffer)
		delete this->buffer;
}

char BinaryReader::ConsumeChar()
{
	return this->buffer[this->pos++];
}

void BinaryReader::Skip(int size)
{
	if (this->pos + size > this->length)
		__debugbreak();
	this->pos += size;
}

unsigned char* BinaryReader::CurrentPtr()
{
	return &this->buffer[this->pos];
}

StringView BinaryReader::ReadASCII(int offset, int length)
{
	return { (char*)&this->buffer[offset], length };
}

StringView BinaryReader::ReadASCII(int length)
{
	StringView result = ReadASCII(this->pos, length);
	this->pos += length;
	return result;
}

StringView BinaryReader::ReadShiftJIS(int offset)
{
	const char* str = (const char*)&this->buffer[offset];
	return { str, (int)strlen(str) };
}

StringView BinaryReader::ReadShiftJIS()
{
	StringView value = ReadShiftJIS(this->pos);
	this->pos += value.length + 1;
	return value;
}

static char* UTF16ToString(const char* ptr, int length)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
	std::string str = conversion.to_bytes((const char16_t*)ptr, (const char16_t*)(ptr)+length);
	int len = (int)str.length();
	char* result = new char[len + 1];
	SDL_memcpy(result, str.c_str(), len);
	result[len] = 0;
	return result;
}

char* BinaryReader::ReadUTF16(int offset)
{
	const char* start = (char*)&this->buffer[offset];
	int length = 0;

	int lastPos = this->pos;
	this->pos = offset;

	unsigned char pair0 = ReadByte();
	unsigned char pair1 = ReadByte();
	while (pair0 != 0 || pair1 != 0)
	{
		length++;
		pair0 = ReadByte();
		pair1 = ReadByte();
	}

	this->pos = lastPos;

	return UTF16ToString(start, length);
}

long long BinaryReader::ReadInt64(int offset)
{
	long long b0 = (long long)this->buffer[offset];
	long long b1 = (long long)this->buffer[offset + 1];
	long long b2 = (long long)this->buffer[offset + 2];
	long long b3 = (long long)this->buffer[offset + 3];
	long long b4 = (long long)this->buffer[offset + 4];
	long long b5 = (long long)this->buffer[offset + 5];
	long long b6 = (long long)this->buffer[offset + 6];
	long long b7 = (long long)this->buffer[offset + 7];

	if (this->bigEndian)
		return (b0 << 56) | (b1 << 48) | (b2 << 40) | (b3 << 32) | (b4 << 24) | (b5 << 16) | (b6 << 8) | (b7 << 0);
	else
		return (b7 << 56) | (b6 << 48) | (b5 << 40) | (b4 << 32) | (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0 << 0);
}

long long BinaryReader::ReadInt64()
{
	long long i = ReadInt64(this->pos);
	this->pos += sizeof(long long);
	return i;
}

int BinaryReader::ReadInt32(int offset)
{
	int b0 = (int)this->buffer[offset];
	int b1 = (int)this->buffer[offset + 1];
	int b2 = (int)this->buffer[offset + 2];
	int b3 = (int)this->buffer[offset + 3];

	if (this->bigEndian)
		return (b0 << 24) | (b1 << 16) | (b2 << 8) | (b3 << 0);
	else
		return (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0 << 0);
}

int BinaryReader::ReadInt32()
{
	int i = ReadInt32(this->pos);
	this->pos += sizeof(int);
	return i;
}

unsigned int BinaryReader::ReadUInt32(int offset)
{
	return (unsigned int)ReadInt32(offset);
}

unsigned int BinaryReader::ReadUInt32()
{
	unsigned int i = ReadUInt32(this->pos);
	this->pos += sizeof(unsigned int);
	return i;
}

short BinaryReader::ReadInt16(int offset)
{
	short b0 = (short)this->buffer[offset];
	short b1 = (short)this->buffer[offset + 1];

	if (this->bigEndian)
		return (b0 << 8) | (b1 << 0);
	else
		return (b1 << 8) | (b0 << 0);
}

short BinaryReader::ReadInt16()
{
	short s = ReadInt16(this->pos);
	this->pos += sizeof(short);
	return s;
}

unsigned short BinaryReader::ReadUInt16()
{
	short s = ReadInt16(this->pos);
	this->pos += sizeof(unsigned short);
	return (unsigned short)s;
}

unsigned char BinaryReader::ReadByte()
{
	return ConsumeChar();
}

unsigned char BinaryReader::ReadByte(int offset)
{
	int lastPos = this->pos;
	this->pos = offset;
	unsigned char c = ReadByte();
	this->pos = lastPos;
	return c;
}

char BinaryReader::ReadSByte()
{
	return ConsumeChar();
}

float BinaryReader::ReadFloat()
{
	float f = *(float*)(&this->buffer[this->pos]);
	this->pos += sizeof(float);
	return f;
}

float BinaryReader::ReadByteNorm()
{
	return ((int)ReadByte() - 127) / 127.0f;
}

float BinaryReader::ReadSByteNorm()
{
	return ReadSByte() / 127.0f;
}

float BinaryReader::ReadShortNorm()
{
	return ReadInt16() / 32767.0f;
}

/*
vec2 BinaryReader::ReadVector2()
{
	float x = ReadFloat();
	float y = ReadFloat();
	return vec2(x, y);
}

vec3 BinaryReader::ReadVector3()
{
	float x = ReadFloat();
	float y = ReadFloat();
	float z = ReadFloat();
	return vec3(x, y, z);
}

vec4 BinaryReader::ReadVector4()
{
	float x = ReadFloat();
	float y = ReadFloat();
	float z = ReadFloat();
	float w = ReadFloat();
	return vec4(x, y, z, w);
}

vec3 BinaryReader::ReadByteNormXYZ()
{
	float x = ReadByteNorm();
	float y = ReadByteNorm();
	float z = ReadByteNorm();
	return vec3(x, y, z);
}

vec4 BinaryReader::ReadByteNormXYZW()
{
	float x = ReadByteNorm();
	float y = ReadByteNorm();
	float z = ReadByteNorm();
	float w = ReadByteNorm();
	return vec4(x, y, z, w);
}

vec3 BinaryReader::ReadSByteNormZYX()
{
	float x = ReadSByteNorm();
	float y = ReadSByteNorm();
	float z = ReadSByteNorm();
	return vec3(x, y, z);
}

vec3 BinaryReader::ReadShortNormXYZ()
{
	float x = ReadShortNorm();
	float y = ReadShortNorm();
	float z = ReadShortNorm();
	return vec3(x, y, z);
}

vec4 BinaryReader::ReadShortNormXYZW()
{
	float x = ReadShortNorm();
	float y = ReadShortNorm();
	float z = ReadShortNorm();
	float w = ReadShortNorm();
	return vec4(x, y, z, w);
}

Color BinaryReader::ReadBGRA()
{
	char b = ReadByte();
	char g = ReadByte();
	char r = ReadByte();
	char a = ReadByte();
	return Color(r, g, b, a);
}

Color BinaryReader::ReadARGB()
{
	char a = ReadByte();
	char r = ReadByte();
	char g = ReadByte();
	char b = ReadByte();
	return Color(r, g, b, a);
}

VertexColor BinaryReader::ReadFloatRGBA()
{
	float r = ReadFloat();
	float g = ReadFloat();
	float b = ReadFloat();
	float a = ReadFloat();
	return VertexColor(r, g, b, a);
}

VertexColor BinaryReader::ReadByteRGBA()
{
	unsigned char r = ReadByte();
	unsigned char g = ReadByte();
	unsigned char b = ReadByte();
	unsigned char a = ReadByte();
	return VertexColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

List<int> BinaryReader::ReadInt32List(int offset, int count)
{
	int lastPos = this->pos;
	this->pos = offset;

	List<int> list = CreateList<int>(count);
	for (int i = 0; i < count; i++)
	{
		list.add(ReadInt32());
	}

	this->pos = lastPos;

	return list;
}

List<int> BinaryReader::ReadInt32List(int count)
{
	List<int> list = CreateList<int>(count);
	for (int i = 0; i < count; i++)
	{
		list.add(ReadInt32());
	}
	return list;
}

List<unsigned int> BinaryReader::ReadUInt32List(int count)
{
	List<unsigned int> list = CreateList<unsigned int>(count);
	for (int i = 0; i < count; i++)
	{
		list.add(ReadUInt32());
	}
	return list;
}

List<short> BinaryReader::ReadInt16List(int count)
{
	List<short> list = CreateList<short>(count);
	for (int i = 0; i < count; i++)
	{
		list.add(ReadInt16());
	}
	return list;
}

List<unsigned short> BinaryReader::ReadUInt16List(int offset, int count)
{
	int lastPos = this->pos;
	this->pos = offset;

	List<unsigned short> list = CreateList<unsigned short>(count);
	for (int i = 0; i < count; i++)
	{
		list.add(ReadUInt16());
	}

	this->pos = lastPos;

	return list;
}

List<unsigned char> BinaryReader::ReadBytes(int offset, int count)
{
	Assert(offset + count <= this->length);
	List<unsigned char> list = CreateList<unsigned char>(count);
	for (int i = offset; i < offset + count; i++)
	{
		list.add(this->buffer[i]);
	}
	return list;
}

List<unsigned char> BinaryReader::ReadBytes(int count)
{
	List<unsigned char> bytes = ReadBytes(this->pos, count);
	this->pos += count;
	return bytes;
}
*/

void BinaryReader::ReadBytes(char* dst, int count)
{
	memcpy(dst, &this->buffer[this->pos], count);
	this->pos += count;
}

bool BinaryReader::ReadBoolean()
{
	unsigned char c = ReadByte();
	SDL_assert(c <= 1);
	return c == 1;
}

bool BinaryReader::ReadBoolean(int offset)
{
	int lastPos = this->pos;
	this->pos = offset;
	bool b = ReadBoolean();
	this->pos = lastPos;
	return b;
}

char BinaryReader::AssertChar(char c)
{
	char value = ConsumeChar();
	SDL_assert(value == c);
	return value;
}

std::string BinaryReader::AssertASCII(const std::string& str)
{
	int len = (int)str.length();
	for (int i = 0; i < len; i++)
	{
		char c = this->ConsumeChar();
		SDL_assert(c == str[i]);
	}
	return str;
}

int BinaryReader::AssertInt32(int i)
{
	int value = ReadInt32();
	SDL_assert(value == i);
	return value;
}

int BinaryReader::AssertInt32(int* selection, int num, int& value)
{
	value = ReadInt32();
	for (int i = 0; i < num; i++)
	{
		if (selection[i] == value)
			return value;
	}
	SDL_assert(false);
	return 0;
}

int BinaryReader::AssertInt32(const std::vector<int>& selection)
{
	int value = ReadInt32();
	for (int i = 0; i < (int)selection.size(); i++)
	{
		if (selection[i] == value)
			return value;
	}
	SDL_assert(false);
	return 0;
}

unsigned int BinaryReader::AssertUInt32(unsigned int i)
{
	int value = ReadUInt32();
	SDL_assert(value == i);
	return value;
}

short BinaryReader::AssertInt16(short s)
{
	short value = ReadInt16();
	SDL_assert(value == s);
	return value;
}

unsigned char BinaryReader::AssertByte(unsigned char c)
{
	unsigned char value = ReadByte();
	SDL_assert(value == c);
	return value;
}

unsigned char BinaryReader::AssertByte(unsigned char* selection, int num, unsigned char& value)
{
	value = ReadByte();
	for (int i = 0; i < num; i++)
	{
		if (selection[i] == value)
			return value;
	}
	SDL_assert(false);
	return 0;
}

unsigned char BinaryReader::AssertByte(const std::vector<unsigned char>& selection)
{
	unsigned char value = ReadByte();
	for (int i = 0; i < (int)selection.size(); i++)
	{
		if (selection[i] == value)
			return value;
	}
	SDL_assert(false);
	return 0;
}

float BinaryReader::AssertFloat(float f)
{
	float value = ReadFloat();
	SDL_assert(value == f);
	return value;
}

bool BinaryReader::AssertBoolean(bool b)
{
	bool value = ReadBoolean();
	SDL_assert(value == b);
	return value;
}

char BinaryReader::AssertPattern(int length, char pattern)
{
	for (int i = 0; i < length; i++)
	{
		char c = ReadByte();
		SDL_assert(c == pattern);
	}
	return pattern;
}
