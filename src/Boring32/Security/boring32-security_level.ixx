export module boring32:security_saferlevel;
import boring32.shared;
import :error;

export namespace Boring32::Security
{
	export class SaferLevel final
	{
		public:
			~SaferLevel()
			{
				if (not m_handle)
					return;
				// https://learn.microsoft.com/en-us/windows/win32/api/winsafer/nf-winsafer-safercloselevel
				Win32::WinSafer::SaferCloseLevel(m_handle);
			}

			SaferLevel(const SaferLevel&) = delete;
			SaferLevel& operator=(const SaferLevel&) = delete;

			SaferLevel(
				Win32::WinSafer::Scope scope, 
				Win32::WinSafer::Level level, 
				Win32::WinSafer::Flags flags
			) : m_scope(scope), 
				m_level(level), 
				m_flags(flags)
			{ }

		public:
			Win32::HANDLE ComputeToken(HANDLE token, Win32::WinSafer::TokenFlags flags)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/winsafer/nf-winsafer-safercomputetokenfromlevel
				HANDLE out = nullptr;
				bool successful = Win32::WinSafer::SaferComputeTokenFromLevel(
					m_handle,
					token,
					&out,
					flags,
					0
				);
				if (not successful)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("SaferComputeTokenFromLevel() failed", lastError);
				}
				return out;
			}

		private:
			Win32::WinSafer::Scope m_scope = Win32::WinSafer::Scope::Machine;
			Win32::WinSafer::Level m_level = Win32::WinSafer::Level::Constrained;
			Win32::WinSafer::Flags m_flags = Win32::WinSafer::Flags::Open;

			Win32::WinSafer::SAFER_LEVEL_HANDLE m_handle = 
				[](Win32::WinSafer::Scope scope, Win32::WinSafer::Level level, Win32::WinSafer::Flags flags)
				{
					Win32::WinSafer::SAFER_LEVEL_HANDLE handle = nullptr;
					// https://learn.microsoft.com/en-us/windows/win32/api/winsafer/nf-winsafer-safercreatelevel
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
				}(m_scope, m_level, m_flags);
	};
}