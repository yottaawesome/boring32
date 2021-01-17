#pragma once
#include <Windows.h>
#include <bcrypt.h>

namespace Boring32::Crypto
{
	class AesEncryption
	{
		public:
			virtual ~AesEncryption();
			AesEncryption();

		public:
			virtual BCRYPT_ALG_HANDLE GetHandle() const noexcept;
			virtual DWORD GetObjectByteSize();
			virtual void SetChainingMode(const std::wstring& mode);
		
		protected:
			BCRYPT_ALG_HANDLE m_algHandle;
	}; 
}