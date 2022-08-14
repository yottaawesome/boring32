module;

#include <string>
#include <source_location>
#include <Windows.h>
#include <sddl.h>

module boring32.security:securitydescriptor;
import boring32.error;

namespace Boring32::Security
{
	SecurityDescriptor::~SecurityDescriptor() { Close(); }

	SecurityDescriptor::SecurityDescriptor(SecurityDescriptor&& other) noexcept
	{
		Move(other);
	}

	SecurityDescriptor::SecurityDescriptor(std::wstring descriptorString)
		: m_descriptorString(std::move(descriptorString))
	{
		Create();
	}

	void SecurityDescriptor::Close()
	{
		m_descriptor.reset();
	}

	PSECURITY_DESCRIPTOR SecurityDescriptor::GetDescriptor() const noexcept
	{
		return m_descriptor.get();
	}

	const std::wstring& SecurityDescriptor::GetDescriptorString() const noexcept
	{
		return m_descriptorString;
	}
	
	void SecurityDescriptor::Create()
	{
		void* sd;
		// https://docs.microsoft.com/en-us/windows/win32/secbp/creating-a-dacl
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsecuritydescriptortosecuritydescriptora
		const bool succeeded = ConvertStringSecurityDescriptorToSecurityDescriptorW(
			m_descriptorString.c_str(),
			SDDL_REVISION_1, // Must be SDDL_REVISION_1
			&sd,
			nullptr
		);
		if (!succeeded)
			throw Error::Win32Error(
				"ConvertStringSecurityDescriptorToSecurityDescriptorW() failed",
				GetLastError()
			);
		m_descriptor = std::move(
			RAII::LocalHeapUniquePtr<std::remove_pointer<PSECURITY_DESCRIPTOR>::type>(sd)
		);
	}
	
	SecurityDescriptor::operator std::wstring() const noexcept
	{
		return m_descriptorString;
	}
	
	SecurityDescriptor::operator PSECURITY_DESCRIPTOR() const noexcept
	{
		return m_descriptor.get();
	}

	SecurityDescriptor& SecurityDescriptor::operator=(SecurityDescriptor&& other) noexcept
	{
		return Move(other);
	}
	
	SecurityDescriptor& SecurityDescriptor::Move(SecurityDescriptor& other) noexcept
	{
		if (&other == this)
			return *this;
		m_descriptorString = std::move(other.m_descriptorString);
		m_descriptor = std::move(other.m_descriptor);
		return *this;
	}
}