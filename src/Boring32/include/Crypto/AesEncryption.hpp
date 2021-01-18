#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <bcrypt.h>
#include "CryptoKey.hpp"
#include "ChainingMode.hpp"

namespace Boring32::Crypto
{
	class AesEncryption
	{
		public:
			virtual ~AesEncryption();
			AesEncryption();

		public:
			virtual void Close() noexcept;
			virtual BCRYPT_ALG_HANDLE GetHandle() const noexcept;
			virtual DWORD GetObjectByteSize() const;
			virtual DWORD GetBlockByteLength() const;
			virtual void SetChainingMode(const ChainingMode mode);
			virtual CryptoKey GenerateSymmetricKey(const std::vector<std::byte>& key);

			// IV will be modified during encryption, so pass a copy if needed
			virtual std::vector<std::byte> Encrypt(
				const CryptoKey& key,
				const std::vector<std::byte>& iv,
				const std::wstring& string
			);

			virtual std::vector<std::byte> Encrypt(
				const CryptoKey& key,
				const std::vector<std::byte>& iv,
				const std::vector<std::byte>& data
			);

			virtual std::vector<std::byte> Decrypt(
				const CryptoKey& key,
				const std::vector<std::byte>& iv,
				const std::vector<std::byte>& cypherText
			);

		protected:
			BCRYPT_ALG_HANDLE m_algHandle;
	}; 
}