module;

#include <source_location>;

export module boring32.security:privatenamespace;
import <string>;
import <vector>;
import <win32.hpp>;
import boring32.error;
import boring32.raii;

export namespace Boring32::Security
{
	class PrivateNamespace final
	{
		public:
			~PrivateNamespace()
			{
				Close();
			}

			PrivateNamespace() = default;

			PrivateNamespace(const PrivateNamespace& other)
			{
				Copy(other);
			}

			PrivateNamespace& operator=(const PrivateNamespace& other)
			{
				Copy(other);
				return *this;
			}

			PrivateNamespace(PrivateNamespace&& other) noexcept
			{
				Move(other);
			}

			PrivateNamespace& operator=(PrivateNamespace&& other) noexcept
			{
				Move(other);
				return *this;
			}

		public:
			PrivateNamespace(
				const bool create,
				const bool destroyOnClose,
				const std::wstring& namespaceName,
				const std::wstring& boundaryName,
				const std::wstring& sid
			) : m_destroyOnClose(destroyOnClose),
				m_namespaceName(namespaceName),
				m_boundaryName(boundaryName),
				m_namespaceSid(sid)
			{
				CreateOrOpen(create);
			}

		public:
			void Close()
			{
				if (m_boundaryDescriptor)
				{
					DeleteBoundaryDescriptor(m_boundaryDescriptor);
					m_boundaryDescriptor = nullptr;
				}
				if (m_namespace)
				{
					ClosePrivateNamespace(m_namespace, 0);
					m_namespace = nullptr;
				}
			}

		private:
			void Copy(const PrivateNamespace& other)
			{
				Close();
				m_namespaceName = other.m_namespaceName;
				m_boundaryName = other.m_boundaryName;
				m_namespaceSid = other.m_namespaceSid;
				m_destroyOnClose = other.m_destroyOnClose;
				if (other.m_namespace)
					CreateOrOpen(false);
			}

			void Move(PrivateNamespace& other) noexcept
			{
				Close();
				m_namespaceName = std::move(other.m_namespaceName);
				m_boundaryName = std::move(other.m_boundaryName);
				m_namespaceSid = std::move(other.m_namespaceSid);
				m_boundaryDescriptor = other.m_boundaryDescriptor;
				other.m_boundaryDescriptor = nullptr;
				m_namespace = other.m_namespace;
				other.m_namespace = nullptr;
			}

			void CreateOrOpen(const bool create) try
			{
				m_boundaryDescriptor = CreateBoundaryDescriptorW(m_boundaryName.c_str(), 0);
				if (!m_boundaryDescriptor)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create boundary descriptor", lastError);
				}

				BYTE localAdminSID[SECURITY_MAX_SID_SIZE];
				DWORD cbSID = sizeof(localAdminSID);
				bool sidCreated = CreateWellKnownSid(
					WinBuiltinAdministratorsSid,
					nullptr,
					localAdminSID,
					&cbSID);
				if (!sidCreated)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create SID", lastError);
				}
				bool sidAdded = AddSIDToBoundaryDescriptor(&m_boundaryDescriptor, localAdminSID);
				if (!sidAdded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to add SID to boundary", lastError);
				}

				if (create)
				{
					// https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format
					// https://docs.microsoft.com/en-us/windows/win32/secauthz/ace-strings
					// https://docs.microsoft.com/en-us/windows/win32/secauthz/sid-strings
					SECURITY_ATTRIBUTES sa;
					sa.nLength = sizeof(sa);
					sa.bInheritHandle = false;
					bool converted = ConvertStringSecurityDescriptorToSecurityDescriptorW(
						m_namespaceSid.c_str(),
						SDDL_REVISION_1,
						&sa.lpSecurityDescriptor,
						nullptr
					);
					if (!converted)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("Failed to convert security descriptor", lastError);
					}
					RAII::LocalHeapUniquePtr<void> securityDescriptor(sa.lpSecurityDescriptor);

					m_namespace = CreatePrivateNamespaceW(
						&sa,
						m_boundaryDescriptor,
						m_namespaceName.c_str()
					);
				}
				else
					m_namespace = OpenPrivateNamespaceW(m_boundaryDescriptor, m_namespaceName.c_str());

				if (!m_namespace)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create private namespace", lastError);
				}
			}
			catch (...)
			{
				Close();
				throw;
			}

		private:
			std::wstring m_namespaceName;
			std::wstring m_boundaryName;
			std::wstring m_namespaceSid;
			HANDLE m_boundaryDescriptor = nullptr;
			HANDLE m_namespace = nullptr;
			bool m_destroyOnClose = true;
	};
}