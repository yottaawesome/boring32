export module boring32.security:saferlevel;
import boring32.win32;
import boring32.error;

export namespace Boring32::Security
{
	export class SaferLevel final
	{
		public:
			~SaferLevel()
			{
				if (m_handle)
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/winsafer/nf-winsafer-safercloselevel
					Win32::WinSafer::SaferCloseLevel(m_handle);
				}
			}

			SaferLevel(const SaferLevel&) = delete;
			SaferLevel& operator=(const SaferLevel&) = delete;

			SaferLevel(Win32::WinSafer::Scope scope, Win32::WinSafer::Level level, Win32::WinSafer::Flags flags)
				: Scope(scope), Level(level), Flags(flags)
			{}

		private:
			Win32::WinSafer::Scope Scope = Win32::WinSafer::Scope::Machine;
			Win32::WinSafer::Level Level = Win32::WinSafer::Level::Constrained;
			Win32::WinSafer::Flags Flags = Win32::WinSafer::Flags::Open;

			Win32::WinSafer::SAFER_LEVEL_HANDLE m_handle = 
				[](Win32::WinSafer::Scope scope, Win32::WinSafer::Level level, Win32::WinSafer::Flags flags)
				{
					Win32::WinSafer::SAFER_LEVEL_HANDLE handle = nullptr;

					bool successful = Win32::WinSafer::SaferCreateLevel(
						(Win32::DWORD)scope,
						(Win32::DWORD)level,
						(Win32::DWORD)flags,
						&handle,
						0
					);
					if (not successful)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error("SaferCreateLevel() failed", lastError);
					}
					return handle;
				}(Scope, Level, Flags);
	};
}