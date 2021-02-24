#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>
#include "include/Error/Win32Error.hpp"

namespace Boring32::Registry
{
	template<typename T>
	void GetValue(
		const HKEY key,
		const std::wstring& valueName,
		const DWORD type,
		T& out
	)
	{
		if (key == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": key is nullptr");

		DWORD sizeInBytes = sizeof(out);
		const LSTATUS status = RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			type,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(
				__FUNCSIG__ ": RegGetValueW() failed",
				status
			);
	}

	template<typename T>
	void WriteValue(
		const HKEY key,
		const std::wstring& valueName,
		const DWORD type,
		const T& value
	)
	{
		if (key == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": key is nullptr");

		const LSTATUS status = RegSetValueExW(
			key,
			valueName.c_str(),
			0,
			type,
			(BYTE*)&value,
			sizeof(value)
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(__FUNCSIG__ ": RegSetValueExW() failed", status);
	}
}