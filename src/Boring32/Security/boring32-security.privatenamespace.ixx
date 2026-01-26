export module boring32:security.privatenamespace;
import std;
import boring32.win32;
import :raii;
import :error;

export namespace Boring32::Security
{
	struct PrivateNamespace final
	{
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

		void Close()
		{
			if (m_boundaryDescriptor)
			{
				Win32::DeleteBoundaryDescriptor(m_boundaryDescriptor);
				m_boundaryDescriptor = nullptr;
			}
			if (m_namespace)
			{
				Win32::ClosePrivateNamespace(m_namespace, 0);
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
			m_boundaryDescriptor = Win32::CreateBoundaryDescriptorW(m_boundaryName.c_str(), 0);
			if (not m_boundaryDescriptor)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create boundary descriptor");

			Win32::BYTE localAdminSID[Win32::_SECURITY_MAX_SID_SIZE];
			Win32::DWORD cbSID = sizeof(localAdminSID);
			bool sidCreated = Win32::CreateWellKnownSid(
				Win32::WELL_KNOWN_SID_TYPE::WinBuiltinAdministratorsSid,
				nullptr,
				localAdminSID,
				&cbSID);
			if (not sidCreated)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create SID");
			bool sidAdded = Win32::AddSIDToBoundaryDescriptor(&m_boundaryDescriptor, localAdminSID);
			if (not sidAdded)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to add SID to boundary");

			if (create)
			{
				// https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format
				// https://docs.microsoft.com/en-us/windows/win32/secauthz/ace-strings
				// https://docs.microsoft.com/en-us/windows/win32/secauthz/sid-strings
				Win32::SECURITY_ATTRIBUTES sa;
				sa.nLength = sizeof(sa);
				sa.bInheritHandle = false;
				bool converted = Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
					m_namespaceSid.c_str(),
					Win32::SddlRevision1,
					&sa.lpSecurityDescriptor,
					nullptr
				);
				if (not converted)
					throw Error::Win32Error(Win32::GetLastError(), "Failed to convert security descriptor");
				RAII::LocalHeapUniquePtr<void> securityDescriptor(sa.lpSecurityDescriptor);

				m_namespace = Win32::CreatePrivateNamespaceW(
					&sa,
					m_boundaryDescriptor,
					m_namespaceName.c_str()
				);
			}
			else
				m_namespace = Win32::OpenPrivateNamespaceW(m_boundaryDescriptor, m_namespaceName.c_str());

			if (not m_namespace)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create private namespace");
		}
		catch (...)
		{
			Close();
			throw;
		}

		std::wstring m_namespaceName;
		std::wstring m_boundaryName;
		std::wstring m_namespaceSid;
		Win32::HANDLE m_boundaryDescriptor = nullptr;
		Win32::HANDLE m_namespace = nullptr;
		bool m_destroyOnClose = true;
	};
}