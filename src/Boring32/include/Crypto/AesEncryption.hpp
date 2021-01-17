#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <bcrypt.h>
#include "CryptoKey.hpp"

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
			virtual CryptoKey GenerateSymmetricKey(const std::vector<std::byte>& key);
		
		protected:
			BCRYPT_ALG_HANDLE m_algHandle;
	}; 
}