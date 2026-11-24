export module boring32:ui.window;
import std;
import :win32;
import :strings;
import :error;
import :raii;
import :ui.common;

export namespace Boring32::UI
{
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
