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
			PrivateNamespace();
			PrivateNamespace(const PrivateNamespace& other);
			virtual PrivateNamespace& operator=(const PrivateNamespace& other);
			PrivateNamespace(PrivateNamespace&& other) noexcept;
			virtual PrivateNamespace& operator=(PrivateNamespace&& other) noexcept;

			PrivateNamespace(
				const bool create,
				const bool destroyOnClose,
				const std::wstring& namespaceName,
				const std::wstring& boundaryName,
				const std::wstring& sid
			);

		public:
			virtual void Close();

		protected:
			virtual void Copy(const PrivateNamespace& other);
			virtual void Move(PrivateNamespace& other) noexcept;
			virtual void CreateOrOpen(const bool create);

		protected:
			std::wstring m_namespaceName;
			std::wstring m_boundaryName;
			std::wstring m_namespaceSid;
			HANDLE m_boundaryDescriptor;
			HANDLE m_namespace;
			bool m_destroyOnClose;
	};
}