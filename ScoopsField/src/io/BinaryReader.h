#pragma once

#include "StringView.h"

#include <string>
#include <vector>


struct BinaryReader
{
	unsigned char* buffer;
	int length;
	int pos;

	bool bigEndian = false;
	bool ownsBuffer = false;


	BinaryReader(unsigned char* buffer, int length);
	~BinaryReader();

	char ConsumeChar();
	void Skip(int size);
	unsigned char* CurrentPtr();

	StringView ReadASCII(int offset, int length);
	StringView ReadASCII(int length);
	StringView ReadShiftJIS(int offset);
	StringView ReadShiftJIS();
	char* ReadUTF16(int offset);
	long long ReadInt64(int offset);
	long long ReadInt64();
	int ReadInt32(int offset);
	int ReadInt32();
	unsigned int ReadUInt32(int offset);
	unsigned int ReadUInt32();
	short ReadInt16(int offset);
	short ReadInt16();
	unsigned short ReadUInt16();
	unsigned char ReadByte();
	unsigned char ReadByte(int offset);
	char ReadSByte();
	float ReadFloat();
	float ReadByteNorm();
	float ReadSByteNorm();
	float ReadShortNorm();
	bool ReadBoolean();
	bool ReadBoolean(int offset);
	//vec2 ReadVector2();
	//vec3 ReadVector3();
	//vec4 ReadVector4();
	//vec3 ReadByteNormXYZ();
	//vec4 ReadByteNormXYZW();
	//vec3 ReadSByteNormZYX();
	//vec3 ReadShortNormXYZ();
	//vec4 ReadShortNormXYZW();
	//Color ReadBGRA();
	//Color ReadARGB();
	//VertexColor ReadFloatRGBA();
	//VertexColor ReadByteRGBA();
	//List<int> ReadInt32List(int offset, int count);
	//List<int> ReadInt32List(int count);
	//List<unsigned int> ReadUInt32List(int count);
	//List<short> ReadInt16List(int count);
	//List<unsigned short> ReadUInt16List(int offset, int count);
	//List<unsigned char> ReadBytes(int offset, int count);
	//List<unsigned char> ReadBytes(int count);
	void ReadBytes(char* dst, int count);

	char AssertChar(char c);
	std::string AssertASCII(const std::string& str);
	int AssertInt32(int i);
	int AssertInt32(int* selection, int num, int& value);
	int AssertInt32(const std::vector<int>& selection);
	unsigned int AssertUInt32(unsigned int i);
	short AssertInt16(short s);
	unsigned char AssertByte(unsigned char c);
	unsigned char AssertByte(unsigned char* selection, int num, unsigned char& value);
	unsigned char AssertByte(const std::vector<unsigned char>& selection);
	float AssertFloat(float f);
	bool AssertBoolean(bool b);
	char AssertPattern(int length, char pattern);

	template <typename T>
	T Read()
	{
		T t = *((T*)&this->buffer[this->pos]);
		this->pos += sizeof(T);
		return t;
	}
};
