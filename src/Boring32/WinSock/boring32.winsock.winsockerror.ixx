module;

#include <string>
#include <stdexcept>
#include <Windows.h>

export module boring32.winsock.winsockerror;

export namespace Boring32::WinSock
{
	class WinSockError : public std::runtime_error
	{
		public:
			virtual ~WinSockError();
			WinSockError(const DWORD errorCode);
			WinSockError(const std::string& message, const DWORD errorCode);

			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};
}