export module boring32:security.securitydescriptor;
import std;
import :win32;
import :error;
import :raii;

export namespace Boring32::Security
{
	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/access-control-components
	class SecurityDescriptor final
	{
	public:
		struct Control
		{
			Win32::SECURITY_DESCRIPTOR_CONTROL Control;
			Win32::DWORD Revision;
		};

		~SecurityDescriptor() 
		{ 
			Close(); 
		}

		SecurityDescriptor(SecurityDescriptor&& other) noexcept = default;
		auto operator=(SecurityDescriptor&& other) noexcept -> SecurityDescriptor& = default;

		SecurityDescriptor(const SecurityDescriptor&) = delete;
		auto operator=(const SecurityDescriptor&) -> SecurityDescriptor& = delete;

		SecurityDescriptor(std::wstring descriptorString)
			: m_descriptorString(std::move(descriptorString))
		{
			Create();
		}

		[[nodiscard]] 
		operator std::wstring() const noexcept
		{
			return m_descriptorString;
		}

		[[nodiscard]] 
		operator Win32::PSECURITY_DESCRIPTOR() const noexcept
		{
			return m_descriptor.get();
		}

		void Close()
		{
			m_descriptor.reset();
		}

		[[nodiscard]] 
		auto GetDescriptor() const noexcept -> Win32::PSECURITY_DESCRIPTOR
		{
			return m_descriptor.get();
		}
			
		[[nodiscard]] 
		auto GetDescriptorString() const noexcept -> std::wstring
		{
			return m_descriptorString;
		}

		[[nodiscard]] 
		auto GetControl() const -> Control
		{
			auto c = SecurityDescriptor::Control{ 0 };
			// https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorcontrol
			auto success = Win32::GetSecurityDescriptorControl(m_descriptor.get(), &c.Control, &c.Revision);
			if (not success)
				throw Error::Win32Error{Win32::GetLastError(), "GetSecurityDescriptorControl() failed"};
			return c;
		}

	private:
		void Create()
		{
			auto sd = (void*)nullptr;
			// https://docs.microsoft.com/en-us/windows/win32/secbp/creating-a-dacl
			// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsecuritydescriptortosecuritydescriptora
			auto succeeded = Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
				m_descriptorString.c_str(),
				Win32::SddlRevision1, // Must be SDDL_REVISION_1
				&sd,
				nullptr
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "ConvertStringSecurityDescriptorToSecurityDescriptorW() failed"};
			m_descriptor = RAII::LocalHeapUniquePtr<Win32::SECURITY_DESCRIPTOR>(reinterpret_cast<Win32::SECURITY_DESCRIPTOR*>(sd));
		}

		std::wstring m_descriptorString;
		RAII::LocalHeapUniquePtr<Win32::SECURITY_DESCRIPTOR> m_descriptor;
	};
}