export module boring32:ui;
import boring32.win32;
import :strings;
import :error;
import :raii;

// todo: add win32 UI components, referencing win32-ui project in win32-experiments.
export namespace Boring32::UI
{
	using HwndUniquePtr = RAII::IndirectUniquePtr<Win32::HWND, Win32::DestroyWindow>;
	using FontUniquePtr = RAII::IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;

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

	struct Window
	{
		auto GetHandle(this auto&& self) noexcept { return self.m_window.get(); }
		void Destroy(this auto&& self) noexcept { self.m_window.reset(); }
		void SetFont(this auto&& self, Win32::HFONT font)
		{
			if (self.m_window)
				Win32::SendMessageW(self.m_window.get(), Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(font), true);
		}
		//
		// Shows or hide the window, if present.
		auto Show(this auto&& self) noexcept -> decltype(auto)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::ShowNormal);
			return std::forward<decltype(self)>(self);
		}

		auto Hide(this auto&& self) noexcept -> decltype(auto)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::Hide);
			return std::forward<decltype(self)>(self);
		}

		auto TakeFocus(this auto&& self) -> decltype(auto)
		{
			if (self.m_window)
				Win32::SetFocus(self.m_window.get());
			return std::forward<decltype(self)>(self);
		}
	protected:
		HwndUniquePtr m_window = nullptr;
	};
}
