#include <format>
#include <iostream>

import boring32.strings;





template <>
struct std::formatter<const wchar_t*> : std::formatter<std::string>
{
	auto format(const wchar_t* p, format_context& ctx)
	{
		return formatter<string>::format(std::format("[{}, {}]", "A", "B"), ctx);
	}
};

template <>
struct std::formatter<wchar_t*> : std::formatter<std::string>
{
	auto format(wchar_t* p, format_context& ctx)
	{
		return formatter<string>::format(std::format("[{}, {}]", "A", "B"), ctx);
	}
};

template <>
struct std::formatter<std::wstring_view> : std::formatter<std::string>
{
	auto format(std::wstring_view, format_context& ctx)
	{
		return formatter<string>::format(std::format("[{}, {}]", "A", "B"), ctx);
	}
};

template <>
struct std::formatter<std::wstring> : std::formatter<std::string>
{
	auto format(std::wstring p, format_context& ctx)
	{
		return formatter<string>::format(std::format("[{}, {}]", "A", "B"), ctx);
	}
};

template <class... _Types>
std::string info(const std::string_view _Fmt, const _Types&... _Args) {
	return std::format(_Fmt, _Args...);
}

int main(int argc, char** args)
{
	Boring32::Strings::Utf16String utf16;

	wchar_t x[] = L"";
	std::cout << std::format("{}", (wchar_t*)L"");
	
	std::wstring s;
	std::cout << std::format("{}", s);

	return 0;
}
