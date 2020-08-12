#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "../../Raii/Raii.hpp"

namespace Boring32::Async
{
	class AnonymousPipe
	{
		// Constructors
		public:
			virtual ~AnonymousPipe();
			AnonymousPipe();
			AnonymousPipe(
				const bool inheritable, 
				const DWORD size,
				const std::wstring& delimiter
			);
			AnonymousPipe(
				const DWORD size, 
				const std::wstring& delimiter,
				const HANDLE readHandle,
				const HANDLE writeHandle
			);

			AnonymousPipe(const AnonymousPipe& other);
			virtual void operator=(const AnonymousPipe& other);

			AnonymousPipe(AnonymousPipe&& other) noexcept;
			virtual void operator=(AnonymousPipe&& other) noexcept;
		
		// API
		public:
			virtual void Write(const std::wstring& msg);
			virtual void DelimitedWrite(const std::wstring& msg);
			virtual std::wstring Read();
			virtual std::vector<std::wstring> DelimitedRead();
			virtual void CloseRead();
			virtual void CloseWrite();
			virtual void SetMode(const DWORD mode);
			virtual HANDLE GetRead();
			virtual HANDLE GetWrite();
			virtual std::wstring GetDelimiter() const;
			virtual DWORD GetSize() const;
			virtual DWORD GetUsedSize() const;
			virtual DWORD GetRemainingSize() const;

		// Internal methods
		protected:
			virtual void Cleanup();
			virtual void Move(AnonymousPipe& other) noexcept;
			virtual void Copy(const AnonymousPipe& other);

		// Internal variables
		protected:
			std::wstring m_delimiter;
			DWORD m_size;
			DWORD m_mode;
			Raii::Win32Handle m_readHandle;
			Raii::Win32Handle m_writeHandle;
	};
}