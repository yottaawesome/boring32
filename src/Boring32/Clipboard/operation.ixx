export module boring32:clipboard.operation;
import std;
import :win32;
import :error;

export namespace Boring32::Clipboard
{
	struct ClipboardCloser
	{
		constexpr ~ClipboardCloser() noexcept
		{
			if not consteval
			{
				Win32::CloseClipboard();
			}
		}

		constexpr ClipboardCloser() = default;
		ClipboardCloser(const ClipboardCloser&) = delete;
		ClipboardCloser& operator=(const ClipboardCloser&) = delete;

		ClipboardCloser(ClipboardCloser&&) noexcept = default;
		ClipboardCloser& operator=(ClipboardCloser&&) noexcept = default;
	};

	struct Operation
	{
		std::optional<ClipboardCloser> clipboardClose;

		Operation(const Operation&) = delete;
		Operation& operator=(const Operation&) = delete;

		Operation(Operation&&) noexcept = default;
		Operation& operator=(Operation&&) noexcept = default;

		Operation(Win32::HWND window)
		{
			if (not Win32::OpenClipboard(window))
				throw Error::Win32Error{Win32::GetLastError(), "Failed to open clipboard."};
			clipboardClose.emplace();
		}

		// TODO: support more formats, such as CF_BITMAP, etc.
		auto SetData(this auto&& self, const std::wstring& string) 
			-> decltype(auto)
		{
			// Required for null terminator, which is the size of the character type.
			auto size = std::uint64_t{ sizeof(wchar_t) * string.size() + sizeof(wchar_t) };
			auto alloc = Win32::HGLOBAL{
				Win32::GlobalAlloc(
					static_cast<unsigned>(Win32::GlobalAllocType::Moveable),
					size
				)};
			if (not alloc)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to allocate global memory." };

			try
			{
				void* locked = Win32::GlobalLock(alloc);
				if (not locked)
					throw Error::Win32Error{ Win32::GetLastError(), "Failed to lock global memory." };

				std::memcpy(locked, reinterpret_cast<void*>(const_cast<wchar_t*>(string.data())), size);
				Win32::GlobalUnlock(locked);
				// After the data is set to the clipboard, the system owns the memory handle. Do not free it.
				if (not Win32::SetClipboardData(static_cast<Win32::UINT>(Win32::ClipboardFormats::UnicodeText), alloc))
					throw Error::Win32Error{ Win32::GetLastError(), "Failed to set clipboard data." };
				return std::forward<decltype(self)>(self);
			}
			catch (...)
			{
				Win32::GlobalFree(alloc);
				throw;
			}
		}

		// TODO: support more formats, such as CF_BITMAP, etc.
		auto GetUnicodeText(this const Operation&) -> std::wstring
		{
			auto ptr = Win32::GetClipboardData(static_cast<Win32::UINT>(Win32::ClipboardFormats::UnicodeText));
			if (not ptr)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to get clipboard data." };

			try
			{
				auto locked = static_cast<wchar_t*>(Win32::GlobalLock(ptr));
				if (not locked)
					throw Error::Win32Error{ Win32::GetLastError(), "Failed to lock global memory." };

				auto result = std::wstring{ locked };
				return (Win32::GlobalUnlock(ptr), result);
			}
			catch(...)
			{
				Win32::GlobalUnlock(ptr);
				throw;
			}
		}

		auto EmptyClipboard(this auto&& self) -> decltype(auto)
		{
			if (not Win32::EmptyClipboard())
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to empty clipboard." };
			return std::forward<decltype(self)>(self);
		}

		auto IsFormatAvailable(this const Operation&, Win32::ClipboardFormats format) -> bool
		{
			return Win32::IsClipboardFormatAvailable(static_cast<Win32::UINT>(format)) != 0;
		}
	};
}
