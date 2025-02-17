export module boring32:ui;
import boring32.win32;
import :strings;
import :error;

// todo: add win32 UI components, referencing win32-ui project in win32-experiments.
export namespace Boring32::UI
{
	template<Strings::FixedString VClassName>
	struct WindowClass
	{

	};

	struct BaseWindow
	{
		
	};

    template<typename T>
    concept ControlLike = requires (T t)
    {
        t.Handle = (Win32::HWND)0;
        { T::Class.data() } -> std::same_as<const wchar_t*>;
        { t.Text.data() } -> std::same_as<const wchar_t*>;
        t.Styles = Win32::DWORD{ 0 };
    };

	template<ControlLike TType>
	struct Control
	{
        void Init(Win32::HWND parent, const Win32::DWORD x, const Win32::DWORD y, const Win32::DWORD width, const Win32::DWORD height)
        {
            m_control.Handle = CreateWindowExW(
                0,
                TType::Class.data(),
                m_control.Text.data(),
                m_control.Styles,
                x,
                y,
                width,
                height,
                parent,
                (Win32::HMENU)(Win32::UINT_PTR)(m_control.Id),
                Win32::GetModuleHandleW(nullptr),
                nullptr
            );
            if (not m_control.Handle)
                throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");
            if (not Win32::SetWindowSubclass(m_control.Handle, SubclassProc, 5, reinterpret_cast<Win32::DWORD_PTR>(this)))
            {
                std::println("Failed subclassing button.");
                std::terminate();
            }
        }

        static Win32::LRESULT __stdcall SubclassProc(
            Win32::HWND hwnd,
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR idSubclass,
            Win32::DWORD_PTR refData
        )
        {
            return 0;
        }

		TType m_control;
	};
}
