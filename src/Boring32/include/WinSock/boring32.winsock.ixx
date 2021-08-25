module;

#include "pch.hpp"
#include <vector>
#include <string>
#include <stdexcept>

export module boring32.winsock;

export namespace Boring32::WinSock
{
	class WinSockError : public std::runtime_error
	{
		public:
			virtual ~WinSockError();
			WinSockError(const DWORD errorCode);

			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};

	class WinSockInit
	{
		public:
			WinSockInit();
			WinSockInit(const DWORD highVersion, const DWORD lowVersion);
			virtual ~WinSockInit();

		public:
			const WSAData& GetData() const noexcept;

		protected:
			virtual void Initialize(const DWORD highVersion, const DWORD lowVersion);

		protected:
			WSAData m_wsaData;
	};

	struct ResolvedNames
	{
		std::vector<std::wstring> Names;
	};

	ResolvedNames Resolve(const std::wstring& name);
}
