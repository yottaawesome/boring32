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
			BCRYPT_ALG_HANDLE GetHandle() const noexcept;
		
		protected:
			BCRYPT_ALG_HANDLE m_algHandle;
	}; 
}