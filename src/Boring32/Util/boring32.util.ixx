module;

#include <string>
#include <stdexcept>
#include <vector>
#include <Windows.h>

export module boring32.util;
export import :switcher;

//#define colconc(str1,str2) #str1 ": " #str2

export namespace Boring32::Util
{
	std::wstring GetCurrentExecutableDirectory();
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li);
	size_t GetUnixTime() noexcept;
	size_t GetMillisToMinuteBoundary(const SYSTEMTIME& time, const size_t boundary) noexcept;
	size_t GetMillisToSecondBoundary(const SYSTEMTIME& time, const size_t secondBoundary) noexcept;

	//std::vector<std::byte> StringToByteVector(const std::wstring_view str);
	//std::vector<std::byte> StringToByteVector(const std::string_view str);

	// based on https://stackoverflow.com/questions/45172052/correct-way-to-initialize-a-container-of-stdbyte
	template<typename... Ts>
	std::vector<std::byte> ToByteVector(Ts&&... args) noexcept
	{
		return{ std::byte(std::forward<Ts>(args))... };
	}

	template<typename T> requires std::is_same<std::wstring, T>::value || std::is_same<std::string, T>::value
	T ByteVectorToString(const std::vector<std::byte>& vector)
	{
		return { reinterpret_cast<T::const_pointer>(&vector[0]), vector.size() / sizeof(T::value_type) };
	}

	template<typename T> requires std::is_same<std::wstring, T>::value || std::is_same<std::string, T>::value
	std::vector<std::byte> StringToByteVector(const T& str)
	{
		return {
			reinterpret_cast<const std::byte*>(&str[0]),
			reinterpret_cast<const std::byte*>(&str[0]) + str.size() * sizeof(T::value_type)
		};
	}

	typedef std::string(*blah)(const std::vector<std::byte>& vector);// = ByteVectorToString<std::string>;
	//static blah m = ByteVectorToString<std::string>;

	using x = std::string(*)(const std::vector<std::byte>& vector);
}
