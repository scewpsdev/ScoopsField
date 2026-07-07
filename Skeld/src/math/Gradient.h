#pragma once

#include <string.h>
#include <math.h>


template<typename T, int capacity>
struct Gradient
{
	struct Value
	{
		T value;
		float position;
	};

	Value values[capacity];
	int count = 0;

	Gradient()
	{
		count = 0;
	}

	Gradient(T value)
	{
		values[0] = value;
		count = 1;
	}

	Gradient(T value0, T value1)
	{
		values[0] = value0;
		values[1] = value1;
		count = 2;
	}

	void insertValue(float position, Value value)
	{
		if (count == capacity)
		{
			__debugbreak();
			return;
		}

		for (int i = 0; i < count; i++)
		{
			if (position < values[i].position)
			{
				count++;
				for (int j = count - 1; j > i; j--)
					values[j] = values[j - 1];
				values[i] = value;
				return;
			}
		}
		values[count] = value;
		count++;
	}

	void setValue(float position, T value)
	{
		for (int i = 0; i < count; i++)
		{
			if (values[i].position == position)
			{
				values[i].value = value;
				return;
			}
		}

		insertValue(position, { value, position });
	}

	T getValue(float position)
	{
		if (count == 0)
		{
			__debugbreak();
			return {};
		}

		for (int i = 0; i < count; i++)
		{
			Value v1 = values[i];
			if (position < v1.position)
			{
				T value1 = v1.value;
				if (i == 0)
					return value1;
				else
				{
					Value v0 = values[i - 1];
					T value0 = v0.value;
					float progress = fminf(fmaxf((position - v0.position) / (v1.position - v0.position), 0.0f), 1.0f);
					return value0 * (1 - progress) + value1 * progress;
				}
			}
		}

		return values[count - 1].value;
	}
};
