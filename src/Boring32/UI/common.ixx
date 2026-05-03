export module boring32:ui.common;
import std;
import :raii;
import :win32;

export namespace Boring32::UI
{
	using HwndUniquePtr = RAII::IndirectUniquePtr<Win32::HWND, Win32::DestroyWindow>;
	using FontUniquePtr = RAII::IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;

	template<unsigned VVirtualKeyCode>
	struct KeyBindable
	{
		constexpr auto KeyCode(this auto&&) noexcept -> unsigned { return VVirtualKeyCode; }
	};

	struct Textable
	{
		auto GetText(this const auto& self) -> std::wstring
		{
			auto handle = self.GetHandle();
			if (not handle)
				return {};
			std::wstring buffer(Win32::GetWindowTextLengthW(handle) + 1, '\0');
			Win32::GetWindowTextW(self.GetHandle(), buffer.data(), static_cast<int>(buffer.size()));
			if (not buffer.empty()) // Remove trailing null character.
				buffer.pop_back();
			return buffer;
		}

		auto SetText(this auto&& self, std::wstring_view text)
		{
			// Beware of embedded nulls -- these will cut the displayed text short.
			if (auto handle = self.GetHandle())
				Win32::SetWindowTextW(handle, text.data());
		}

		auto AppendText(this auto&& self, std::wstring_view text)
		{
			self.SetText(std::format(L"{}{}", self.GetText(), text));
		}

		auto ClearText(this auto&& self)
		{
			self.SetText(L"");
		}
	};

	struct CreateWindowArgs
	{
		Win32::PCWSTR WindowName = nullptr;
		Win32::DWORD Style = 0;
		Win32::DWORD ExtendedStyle = 0;
		int X = Win32::CwUseDefault;
		int Y = Win32::CwUseDefault;
		int Width = Win32::CwUseDefault;
		int Height = Win32::CwUseDefault;
		Win32::HWND hWndParent = 0;
		Win32::HMENU Menu = 0;
	};

	//
	// Strongly-typed message.
	template<Win32::DWORD VMsg>
	struct Win32Message
	{
		static constexpr std::uint32_t uMsg = VMsg;
		Win32::HWND Hwnd = nullptr;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
		auto operator==(this const auto&, std::uint32_t msg) noexcept -> bool
		{
			return VMsg == msg;
		}
	};

	struct GenericWin32Message
	{
		Win32::HWND Hwnd = nullptr;
		Win32::UINT uMsg = 0;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};
}
