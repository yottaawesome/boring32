export module boring32.security:privatenamespace;
import <string>;
import <vector>;
import <win32.hpp>;
import boring32.error;
import boring32.raii;

export namespace Boring32::Security
{
	class PrivateNamespace
	{
		public:
			virtual ~PrivateNamespace()
			{
				Close();
			}

			PrivateNamespace() = default;

			PrivateNamespace(const PrivateNamespace& other)
			{
				Copy(other);
			}

			virtual PrivateNamespace& operator=(const PrivateNamespace& other)
			{
				Copy(other);
				return *this;
			}

			PrivateNamespace(PrivateNamespace&& other) noexcept
			{
				Move(other);
			}

			virtual PrivateNamespace& operator=(PrivateNamespace&& other) noexcept
			{
				Move(other);
				return *this;
			}

			PrivateNamespace(
				const bool create,
				const bool destroyOnClose,
				const std::wstring& namespaceName,
				const std::wstring& boundaryName,
				const std::wstring& sid
			) : m_destroyOnClose(destroyOnClose),
				m_boundaryDescriptor(nullptr),
				m_namespace(nullptr),
				m_namespaceName(namespaceName),
				m_boundaryName(boundaryName),
				m_namespaceSid(sid)
			{
				CreateOrOpen(create);
			}

		public:
			virtual void Close()
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

		protected:
			virtual void Copy(const PrivateNamespace& other)
			{
				Close();
				m_namespaceName = other.m_namespaceName;
				m_boundaryName = other.m_boundaryName;
				m_namespaceSid = other.m_namespaceSid;
				m_destroyOnClose = other.m_destroyOnClose;
				if (other.m_namespace)
					CreateOrOpen(false);
			}

			virtual void Move(PrivateNamespace& other) noexcept
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

			virtual void CreateOrOpen(const bool create) try
			{
				m_boundaryDescriptor = CreateBoundaryDescriptorW(m_boundaryName.c_str(), 0);
				if (m_boundaryDescriptor == nullptr)
					throw Error::Win32Error("Failed to create boundary descriptor", GetLastError());

				BYTE localAdminSID[SECURITY_MAX_SID_SIZE];
				DWORD cbSID = sizeof(localAdminSID);
				bool sidCreated = CreateWellKnownSid(
					WinBuiltinAdministratorsSid,
					nullptr,
					localAdminSID,
					&cbSID);
				if (sidCreated == false)
					throw Error::Win32Error("Failed to create SID", GetLastError());
				bool sidAdded = AddSIDToBoundaryDescriptor(&m_boundaryDescriptor, localAdminSID);
				if (sidAdded == false)
					throw Error::Win32Error("Failed to add SID to boundary", GetLastError());

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
					if (converted == false)
						throw Error::Win32Error("Failed to convert security descriptor", GetLastError());
					RAII::LocalHeapUniquePtr<void> securityDescriptor(sa.lpSecurityDescriptor);

					m_namespace = CreatePrivateNamespaceW(
						&sa,
						m_boundaryDescriptor,
						m_namespaceName.c_str()
					);
				}
				else
					m_namespace = OpenPrivateNamespaceW(m_boundaryDescriptor, m_namespaceName.c_str());

				if (m_namespace == nullptr)
					throw Error::Win32Error("Failed to create private namespace", GetLastError());
			}
			catch (...)
			{
				Close();
				throw;
			}

		protected:
			std::wstring m_namespaceName;
			std::wstring m_boundaryName;
			std::wstring m_namespaceSid;
			HANDLE m_boundaryDescriptor = nullptr;
			HANDLE m_namespace = nullptr;
			bool m_destroyOnClose = true;
	};
}