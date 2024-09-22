export module boring32:error_win32errorcodes;
import std;
import boring32.win32;

namespace
{
	std::string TranslateError(Boring32::Win32::DWORD code, Boring32::Win32::HMODULE hModule = nullptr) noexcept
	{
		constexpr auto flags =
			Boring32::Win32::FormatMessageAllocateBuffer
			| Boring32::Win32::FormatMessageFromSystem
			| Boring32::Win32::FormatMessageIgnoreInserts;
		constexpr auto flagsWithModule =
			Boring32::Win32::FormatMessageAllocateBuffer
			| Boring32::Win32::FormatMessageFromHModule
			| Boring32::Win32::FormatMessageIgnoreInserts;

		void* buffer = nullptr;
		Boring32::Win32::DWORD count = Boring32::Win32::FormatMessageA(
			hModule ? flagsWithModule : flags,
			nullptr,
			code,
			0,
			reinterpret_cast<Boring32::Win32::LPSTR>(&buffer),
			0,
			nullptr
		);
		if (not buffer)
			return std::format("Failed interpreting {}.", code);

		std::string message(reinterpret_cast<char*>(buffer));
		Boring32::Win32::LocalFree(buffer);
		if (message.ends_with('\n'))
			message.pop_back();
		if (message.ends_with('\r'))
			message.pop_back();

		return message;
	}
}

export namespace Boring32::Error
{
	struct Win32ErrorCode
	{
		constexpr Win32ErrorCode() noexcept = default;

		constexpr Win32ErrorCode(Win32::DWORD value) noexcept
			: m_value(value)
		{ }

		constexpr operator bool() const noexcept
		{
			return m_value == 0; // ERROR_SUCCESS
		}

		constexpr operator Win32::DWORD() const noexcept
		{
			return m_value;
		}

		constexpr bool operator==(Win32::DWORD other) const noexcept
		{
			return m_value == other;
		}

		constexpr bool operator==(const Win32ErrorCode&) const = default;

		std::string ToString() const noexcept
		{
			return TranslateError(m_value);
		}

		constexpr DWORD Code() const noexcept
		{
			return m_value;
		}

		private:
		DWORD m_value = 0;
	};
}