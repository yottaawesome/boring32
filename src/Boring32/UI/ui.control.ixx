export module boring32:ui.control;
import std;
import :win32;
import :ui.window;
import :ui.common;

export namespace Boring32::UI
{
	struct ControlProperties
	{
		Win32::DWORD Id;
		std::wstring Class;
		std::wstring Text;
		Win32::DWORD Styles = 0;
		Win32::DWORD ExtendedStyles = 0;
		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;
	};

	struct Control : Window
	{
		constexpr Control() = default;
		Control(ControlProperties properties) : m_properties(properties) {}

		auto Create(this auto&& self, Win32::HWND parent) -> void
		{
			static_assert(
				requires { { self.GetClass().data() } -> std::same_as<const wchar_t*>; },
			"This type needs a GetClass() member that returns a wstring or wstring_view!");

			Win32::HWND window = Win32::CreateWindowExW(
				self.m_properties.ExtendedStyles,
				self.GetClass().data(),
				self.m_properties.Text.empty() ? nullptr : self.m_properties.Text.data(),
				self.m_properties.Styles,
				self.m_properties.X,
				self.m_properties.Y,
				self.m_properties.Width,
				self.m_properties.Height,
				parent,
				(Win32::HMENU)(Win32::UINT_PTR)(self.m_properties.Id),
				Win32::GetModuleHandleW(nullptr),
				nullptr
			);
			if (self.m_window = HwndUniquePtr(window); not self.m_window)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");
			if (not Win32::SetWindowSubclass(self.m_window.get(), SubclassProc<std::remove_cvref_t<decltype(self)>>, self.GetSubclassId(), reinterpret_cast<Win32::DWORD_PTR>(&self)))
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");

			if constexpr (requires { self.Init(); })
				self.Init();
		}

		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR uIdSubclass,
			Win32::DWORD_PTR dwRefData
		) -> Win32::LRESULT
		{
			//Log::Info("Control {:X} {:X}", msg, wParam);
			if (msg == Win32::Messages::LeftButtonUp)
				return self.OnMessage(Win32Message<Win32::Messages::LeftButtonUp>{ hwnd, wParam, lParam });
			if (msg == Win32::Messages::Paint)
				return self.OnMessage(Win32Message<Win32::Messages::Paint>{ hwnd, wParam, lParam });
			return self.OnMessage(GenericWin32Message{ .Hwnd = hwnd, .uMsg = msg, .wParam = wParam, .lParam = lParam });
		}

		auto OnMessage(this Control& self, auto msg) noexcept -> Win32::LRESULT
		{
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		template<typename TControl>
		static auto SubclassProc(
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR idSubclass,
			Win32::DWORD_PTR refData
		) -> Win32::LRESULT
		{
			TControl* pThis = reinterpret_cast<TControl*>(refData);
			return pThis
				? pThis->HandleMessage(hwnd, msg, wParam, lParam, idSubclass, refData)
				: Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}

		auto GetId(this const auto& self) noexcept -> unsigned { return self.m_properties.Id; }

	protected:
		ControlProperties m_properties;
	};

	template<unsigned VId, int VX, int VY, int VWidth, int VHeight>
	struct Output : Control, Textable
	{
		using Control::OnMessage;

		Output() : Control(GetDefaultProperties()) {}

		auto GetSubclassId(this const auto&) noexcept -> unsigned
		{
			return VId;
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = VId,
				.Text = L"", // initial window text
				.Styles = Win32::Styles::Window::Child | Win32::Styles::Window::Visible | Win32::Styles::Window::Border | Win32::Styles::Static::Right,
				.X = VX,
				.Y = VY,
				.Width = VWidth,
				.Height = VHeight
			};
		};

		auto GetClass(this auto&&) noexcept -> std::wstring_view
		{
			return L"Static";
		}
	};

	struct Button : Control
	{
		using Control::OnMessage;

		Button() : Control(GetDefaultProperties()) {}

		Button(ControlProperties properties) : Control(properties) {}

		constexpr auto GetClass(this auto&&) noexcept -> std::wstring_view
		{
			return L"Button";
		}

		auto OnMessage(this auto& self, Win32Message<Win32::Messages::LeftButtonUp> msg) -> Win32::LRESULT
			requires requires { self.OnClick(); }
		{
			self.OnClick();
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = 100,
				.Class = L"Button",
				.Text = L"", // window text
				.Styles = Win32::Styles::Button::PushButton | Win32::Styles::Window::Child | Win32::Styles::Window::Visible,
				.ExtendedStyles = Win32::Styles::Window::WindowEdge,
				.X = 10,
				.Y = 10,
				.Width = 100,
				.Height = 50
			};
		};

		void Click(this auto&& self) noexcept
		{
			if (self.GetHandle())
				Win32::SendMessageW(self.GetHandle(), Win32::Messages::ButtonClick, 0, 0);
		}
	};
}
