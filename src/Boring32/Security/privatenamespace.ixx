export module boring32:security.privatenamespace;
import std;
import :win32;
import :raii;
import :error;

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
		auto operator=(const PrivateNamespace& other) -> PrivateNamespace&
		{
			Copy(other);
			return *this;
		}

		PrivateNamespace(PrivateNamespace&& other) noexcept
		{
			Move(other);
		}
		auto operator=(PrivateNamespace&& other) noexcept -> PrivateNamespace&
		{
			Move(other);
			return *this;
		}

		PrivateNamespace(
			bool create,
			bool destroyOnClose,
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
			m_boundaryDescriptor = std::exchange(other.m_boundaryDescriptor, nullptr);
			m_namespace = std::exchange(other.m_namespace, nullptr);
		}

		void CreateOrOpen(const bool create) 
		try
		{
			m_boundaryDescriptor = Win32::CreateBoundaryDescriptorW(m_boundaryName.c_str(), 0);
			if (not m_boundaryDescriptor)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create boundary descriptor"};

			auto localAdminSID = std::array<Win32::BYTE, Win32::_SECURITY_MAX_SID_SIZE>{};
			auto cbSID = static_cast<Win32::DWORD>(localAdminSID.size() * sizeof(Win32::BYTE));
			auto sidCreated = 
				Win32::CreateWellKnownSid(
					Win32::WELL_KNOWN_SID_TYPE::WinBuiltinAdministratorsSid,
					nullptr,
					localAdminSID.data(),
					&cbSID
				);
			if (not sidCreated)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create SID"};
			if (not Win32::AddSIDToBoundaryDescriptor(&m_boundaryDescriptor, localAdminSID.data()))
				throw Error::Win32Error{Win32::GetLastError(), "Failed to add SID to boundary"};

			if (not create)
			{
				m_namespace = Win32::OpenPrivateNamespaceW(m_boundaryDescriptor, m_namespaceName.c_str());
			}
			else 
			{
				// https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format
				// https://docs.microsoft.com/en-us/windows/win32/secauthz/ace-strings
				// https://docs.microsoft.com/en-us/windows/win32/secauthz/sid-strings
				auto sa = Win32::SECURITY_ATTRIBUTES{
					.nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
					.bInheritHandle = false,
				};
				auto converted = 
					Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
						m_namespaceSid.c_str(),
						Win32::SddlRevision1,
						&sa.lpSecurityDescriptor,
						nullptr
					);
				if (not converted)
					throw Error::Win32Error{Win32::GetLastError(), "Failed to convert security descriptor"};
				auto securityDescriptor = RAII::LocalHeapUniquePtr<void>{ sa.lpSecurityDescriptor };

				m_namespace = Win32::CreatePrivateNamespaceW(
					&sa,
					m_boundaryDescriptor,
					m_namespaceName.c_str()
				);
			}

			if (not m_namespace)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create private namespace"};
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