export module boring32.async:waitable;
import <chrono>;
import boring32.win32;
import boring32.error;
import :functions;
import :concepts;

export namespace Boring32::Async
{
	class Waitable
	{
		public:
			Waitable(Win32::HANDLE handle)
				: m_handle(handle)
			{
				if (not m_handle)
					throw Error::Boring32Error("Waitable handle required");
			}

		public:
			operator bool() const noexcept 
			{
				return m_handle != nullptr;
			}

			operator HANDLE() const noexcept
			{
				return m_handle;
			}

		public:
			WaitResult Wait(Duration auto duration, const bool alertable = false)
			{
				return WaitFor(
					m_handle,
					std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(),
					alertable
				);
			}

		private:
			Win32::HANDLE m_handle = nullptr;
	};
}