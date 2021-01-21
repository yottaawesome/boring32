#pragma once
#include <string>
#include <stdexcept>
#include <vector>
#include <Windows.h>

#define colconc(str1,str2) #str1 ": " #str2

namespace Boring32::Util
{
	std::wstring GetCurrentExecutableDirectory();
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li);
	size_t GetUnixTime();
	DWORD GetMillisToMinuteBoundary(const SYSTEMTIME& time, const UINT boundary);
	// based on https://stackoverflow.com/questions/45172052/correct-way-to-initialize-a-container-of-stdbyte
	template<typename... Ts>
	std::vector<std::byte> ToByteVector(Ts&&... args) noexcept 
	{
		return{ std::byte(std::forward<Ts>(args))... };
	}
}
