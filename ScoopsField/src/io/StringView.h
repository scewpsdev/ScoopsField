#pragma once


struct StringView
{
	const char* buffer;
	int length;


	char operator[](int idx)
	{
		return buffer[idx];
	}
};
