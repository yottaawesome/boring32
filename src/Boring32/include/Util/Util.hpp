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
	size_t GetUnixTime() noexcept;
	size_t GetMillisToMinuteBoundary(const SYSTEMTIME& time, const size_t boundary) noexcept;
	size_t GetMillisToSecondBoundary(const SYSTEMTIME& time, const size_t secondBoundary) noexcept;

	void ByteVectorToString(const std::vector<std::byte>& vector, std::wstring& result);
	void ByteVectorToString(const std::vector<std::byte>& vector, std::string& result);
	std::vector<std::byte> StringToByteVector(const std::wstring& str);
	std::vector<std::byte> StringToByteVector(const std::string& str);

	// based on https://stackoverflow.com/questions/45172052/correct-way-to-initialize-a-container-of-stdbyte
	template<typename... Ts>
	std::vector<std::byte> ToByteVector(Ts&&... args) noexcept 
	{
		return{ std::byte(std::forward<Ts>(args))... };
	}
}
