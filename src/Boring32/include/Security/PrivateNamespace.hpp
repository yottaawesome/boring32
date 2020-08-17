#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace Boring32::Security
{
	class PrivateNamespace
	{
		public:
			virtual ~PrivateNamespace();
			PrivateNamespace(
				const bool create,
				const std::wstring& namespaceName,
				const std::wstring& boundaryName,
				const std::wstring& sid
			);

		public:
			virtual void Close();

		protected:
			std::wstring m_namespaceName;
			std::wstring m_boundaryName;
			std::wstring m_namespaceSid;
			HANDLE m_boundaryDescriptor;
			HANDLE m_namespace;
	};
}