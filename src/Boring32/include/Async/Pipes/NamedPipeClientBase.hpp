#pragma once
#include <string>
#include "../../Raii/Raii.hpp"

namespace Boring32::Async
{
	class NamedPipeClientBase
	{
		public:
			virtual ~NamedPipeClientBase();
			NamedPipeClientBase(const std::wstring& name, const DWORD fileAttributes);

			NamedPipeClientBase(const NamedPipeClientBase& other);
			virtual void operator=(const NamedPipeClientBase& other);

			NamedPipeClientBase(NamedPipeClientBase&& other) noexcept;
			virtual void operator=(NamedPipeClientBase&& other) noexcept;

			virtual void Connect(const DWORD timeout);
			virtual void Close();

		protected:
			virtual void Copy(const NamedPipeClientBase& other);
			virtual void Move(NamedPipeClientBase& other) noexcept;

		protected:
			Raii::Win32Handle m_handle;
			std::wstring m_pipeName;
			DWORD m_fileAttributes;
	};
}