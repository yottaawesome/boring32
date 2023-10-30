export module boring32.async:waitable;
import <chrono>;
import boring32.win32;
import :functions;
import :concepts;

export namespace Boring32::Async
{
	class Waitable
	{
		public:
			Waitable(Win32::HANDLE handle)
				: m_handle(handle)
			{ }

		public:



		private:
			HANDLE m_handle = nullptr;
	};
}