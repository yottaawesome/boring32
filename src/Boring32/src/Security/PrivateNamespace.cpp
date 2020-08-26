#include "pch.hpp"
#include <Sddl.h>          // for SID management
#include "include/Error/Win32Error.hpp"
#include "include/Security/PrivateNamespace.hpp"

namespace Boring32::Security
{
	PrivateNamespace::~PrivateNamespace()
	{
		Close();
	}

	PrivateNamespace::PrivateNamespace()
		: m_boundaryDescriptor(nullptr),
		m_namespace(nullptr)
	{ }

	void PrivateNamespace::Close()
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

	PrivateNamespace::PrivateNamespace(
		const bool create,
		const std::wstring& namespaceName,
		const std::wstring& boundaryName,
		const std::wstring& sid
	)
		: m_boundaryDescriptor(nullptr),
		m_namespace(nullptr),
		m_namespaceName(namespaceName),
		m_boundaryName(boundaryName),
		m_namespaceSid(sid)
	{
		CreateOrOpen(create);
	}

	void PrivateNamespace::CreateOrOpen(const bool create)
	{
		try
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

				m_namespace = CreatePrivateNamespaceW(
					&sa,
					m_boundaryDescriptor,
					m_namespaceName.c_str()
				);
				LocalFree(sa.lpSecurityDescriptor);
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
	}

	PrivateNamespace::PrivateNamespace(const PrivateNamespace& other)
	{
		Copy(other);
	}

	PrivateNamespace& PrivateNamespace::operator=(const PrivateNamespace& other)
	{
		Copy(other);
		return *this;
	}

	void PrivateNamespace::Copy(const PrivateNamespace& other)
	{
		Close();
		if (other.m_namespace)
			CreateOrOpen(false);
	}

	PrivateNamespace::PrivateNamespace(PrivateNamespace&& other) noexcept
	{
		Move(other);
	}

	PrivateNamespace& PrivateNamespace::operator=(PrivateNamespace&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void PrivateNamespace::Move(PrivateNamespace& other) noexcept
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
}