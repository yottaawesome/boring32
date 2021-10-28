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

	std::vector<std::byte> StringToByteVector(const std::wstring_view str);
	std::vector<std::byte> StringToByteVector(const std::string_view str);

	// based on https://stackoverflow.com/questions/45172052/correct-way-to-initialize-a-container-of-stdbyte
	template<typename... Ts>
	std::vector<std::byte> ToByteVector(Ts&&... args) noexcept 
	{
		return{ std::byte(std::forward<Ts>(args))... };
	}

	template<typename T>
	T ByteVectorToString(const std::vector<std::byte>& vector)
	{
		static_assert(false, "Cannot use this template");
	}

	template<>
	std::wstring ByteVectorToString(const std::vector<std::byte>& vector);

	template<>
	std::string ByteVectorToString(const std::vector<std::byte>& vector);

	typedef std::string (*blah)(const std::vector<std::byte>& vector);// = ByteVectorToString<std::string>;
	static blah m = ByteVectorToString<std::string>;

	using x = std::string(*)(const std::vector<std::byte>& vector);
}
