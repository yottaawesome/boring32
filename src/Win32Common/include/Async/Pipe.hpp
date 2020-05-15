#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace Win32Utils::Async
{
	class Pipe
	{
		// Constructors
		public:
			virtual ~Pipe();
			Pipe();
			Pipe(const Pipe& other);
			Pipe(Pipe&& other) noexcept;
			Pipe(const bool inheritable, const DWORD size, const std::wstring& delimiter);
			Pipe(
				const bool inheritable,
				const DWORD size, 
				const bool duplicate,
				const std::wstring& delimiter,
				const HANDLE readHandle, 
				const HANDLE writeHandle
			);
		
		// API
		public:
			virtual void Write(const std::wstring& msg);
			virtual std::wstring Read();
			virtual std::vector<std::wstring> DelimitedRead();
			virtual void CloseRead();
			virtual void CloseWrite();
			virtual HANDLE GetRead();
			virtual HANDLE GetWrite();

		// Overloaded operators
		public:
			virtual void operator=(const Pipe& other);
			virtual void operator=(Pipe&& other) noexcept;

		// Internal methods
		protected:
			virtual void Duplicate(const Pipe& other);
			virtual void Duplicate(const HANDLE readHandle, const HANDLE writeHandle);
			virtual void Cleanup();

		// Internal variables
		protected:
			std::wstring m_delimiter;
			DWORD m_size;
			bool m_inheritable;
			HANDLE m_readHandle;
			HANDLE m_writeHandle;
	};
}