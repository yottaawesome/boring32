#pragma once
#include "pch.hpp"
#include <algorithm>
#include <string>
#include <Windows.h>
#include "include/Crypto/SensitiveString.hpp"

namespace Boring32::Crypto
{
	SensitiveString::~SensitiveString()
	{
		Clear();
	}

	SensitiveString::SensitiveString() { }

	SensitiveString::SensitiveString(std::wstring&& value)
	:	Value(std::move(value))
	{ }

	void SensitiveString::Clear()
	{
		if (Value.empty() == false)
		{
			std::fill(Value.begin(), Value.end(), '\0');
			Value.resize(0);
		}
	}
}