#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <winhttp.h>

// Simple resource wrappers. See also: https://github.com/microsoft/wil
namespace Win32Utils::Raii
{	
	class Sid
	{
		public:
			Sid(
				PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
				BYTE                      nSubAuthorityCount,
				DWORD                     nSubAuthority0,
				DWORD                     nSubAuthority1,
				DWORD                     nSubAuthority2,
				DWORD                     nSubAuthority3,
				DWORD                     nSubAuthority4,
				DWORD                     nSubAuthority5,
				DWORD                     nSubAuthority6,
				DWORD                     nSubAuthority7
			);
			virtual ~Sid();
			virtual PSID GetSid();

		protected:
			BYTE sidBuffer[256];
			PSID pAdminSID = (PSID)sidBuffer;
	};

	class Win32Handle
	{
		public:
			virtual ~Win32Handle();
			Win32Handle();
			Win32Handle(const HANDLE handle, const bool inheritable);
			
			Win32Handle(const Win32Handle& otherHandle);
			virtual void operator=(const Win32Handle& other);

			Win32Handle(Win32Handle&& otherHandle) noexcept;
			virtual void operator=(Win32Handle&& other) noexcept;

			virtual void operator=(const bool isInheritable);
			virtual void operator=(const HANDLE other);
			virtual bool operator==(const HANDLE other) const;
			virtual bool operator==(const Win32Handle& other) const;
			virtual HANDLE* operator&();

			virtual HANDLE GetHandle() const;
			virtual HANDLE& GetHandleAddress();
			virtual void Close();
			virtual HANDLE Duplicate() const;

			virtual bool IsInheritable() const;

		protected:
			virtual void Duplicate(const HANDLE otherHandle, const bool inheritable);

		protected:
			HANDLE m_handle;
			bool m_inheritable;
	};
}
