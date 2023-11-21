export module boring32.security:securitydescriptor;
import boring32.raii;
import std;
import std.compat;
import boring32.win32;
import boring32.error;

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

		public:
			~SecurityDescriptor() { Close(); }

			SecurityDescriptor(SecurityDescriptor&& other) noexcept
			{
				Move(other);
			}

			SecurityDescriptor(const SecurityDescriptor&) = delete;

			SecurityDescriptor(std::wstring descriptorString)
				: m_descriptorString(std::move(descriptorString))
			{
				Create();
			}

		public:
			SecurityDescriptor& operator=(const SecurityDescriptor&) = delete;

			SecurityDescriptor& operator=(SecurityDescriptor&& other) noexcept
			{
				return Move(other);
			}

		public:
			[[nodiscard]] operator std::wstring() const noexcept
			{
				return m_descriptorString;
			}

			[[nodiscard]] operator Win32::PSECURITY_DESCRIPTOR() const noexcept
			{
				return m_descriptor.get();
			}

		public:
			void Close()
			{
				m_descriptor.reset();
			}

			[[nodiscard]] Win32::PSECURITY_DESCRIPTOR GetDescriptor() const noexcept
			{
				return m_descriptor.get();
			}
			
			[[nodiscard]] const std::wstring& GetDescriptorString() const noexcept
			{
				return m_descriptorString;
			}

			[[nodiscard]] Control GetControl() const
			{
				SecurityDescriptor::Control c{ 0 };
				// https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorcontrol
				const bool success = Win32::GetSecurityDescriptorControl(
					m_descriptor.get(),
					&c.Control,
					&c.Revision
				);
				if (!success)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetSecurityDescriptorControl() failed", lastError);
				}
				return c;
			}

		private:
			void Create()
			{
				void* sd;
				// https://docs.microsoft.com/en-us/windows/win32/secbp/creating-a-dacl
				// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsecuritydescriptortosecuritydescriptora
				const bool succeeded = Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
					m_descriptorString.c_str(),
					Win32::SddlRevision1, // Must be SDDL_REVISION_1
					&sd,
					nullptr
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"ConvertStringSecurityDescriptorToSecurityDescriptorW() failed",
						lastError
					);
				}
				m_descriptor =
					RAII::LocalHeapUniquePtr<std::remove_pointer<Win32::PSECURITY_DESCRIPTOR>::type>(sd);
			}

			SecurityDescriptor& Move(SecurityDescriptor& other) noexcept
			{
				if (&other == this)
					return *this;
				m_descriptorString = std::move(other.m_descriptorString);
				m_descriptor = std::move(other.m_descriptor);
				return *this;
			}

		private:
			std::wstring m_descriptorString;
			RAII::LocalHeapUniquePtr<std::remove_pointer<Win32::PSECURITY_DESCRIPTOR>::type> m_descriptor;
	};
}