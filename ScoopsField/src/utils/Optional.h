#pragma once


template<typename T>
struct Optional
{
	bool hasValue = false;
	T value = {};


	Optional() = default;

	Optional(const T& value)
	{
		hasValue = true;
		this->value = value;
	}

	operator bool() const { return hasValue; }
};


template<typename T>
inline void InitOptional(Optional<T>* optional, const T& value)
{
	optional->hasValue = true;
	optional->value = value;
}
