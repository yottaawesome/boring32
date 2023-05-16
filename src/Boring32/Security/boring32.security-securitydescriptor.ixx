module;

#include <source_location>;

export module boring32.security:securitydescriptor;
import boring32.raii;
import <string>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Security
{
	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/access-control-components
	class SecurityDescriptor
	{
		public:
			struct Control
			{
				SECURITY_DESCRIPTOR_CONTROL Control;
				DWORD Revision;
			};

		public:
			virtual ~SecurityDescriptor() { Close(); }

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
			virtual SecurityDescriptor& operator=(const SecurityDescriptor&) = delete;

			virtual SecurityDescriptor& operator=(SecurityDescriptor&& other) noexcept
			{
				return Move(other);
			}

		public:
			[[nodiscard]] virtual operator std::wstring() const noexcept
			{
				return m_descriptorString;
			}

			[[nodiscard]] virtual operator PSECURITY_DESCRIPTOR() const noexcept
			{
				return m_descriptor.get();
			}

		public:
			virtual void Close()
			{
				m_descriptor.reset();
			}

			[[nodiscard]] virtual PSECURITY_DESCRIPTOR GetDescriptor() const noexcept
			{
				return m_descriptor.get();
			}
			
			[[nodiscard]] virtual const std::wstring& GetDescriptorString() const noexcept
			{
				return m_descriptorString;
			}

			[[nodiscard]] virtual Control GetControl() const
			{
				SecurityDescriptor::Control c{ 0 };
				// https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorcontrol
				const bool success = GetSecurityDescriptorControl(
					m_descriptor.get(),
					&c.Control,
					&c.Revision
				);
				if (!success)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("GetSecurityDescriptorControl() failed", lastError);
				}
				return c;
			}

		protected:
			virtual void Create()
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
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error(
						"ConvertStringSecurityDescriptorToSecurityDescriptorW() failed",
						lastError
					);
				}
				m_descriptor =
					RAII::LocalHeapUniquePtr<std::remove_pointer<PSECURITY_DESCRIPTOR>::type>(sd);
			}

			virtual SecurityDescriptor& Move(SecurityDescriptor& other) noexcept
			{
				if (&other == this)
					return *this;
				m_descriptorString = std::move(other.m_descriptorString);
				m_descriptor = std::move(other.m_descriptor);
				return *this;
			}

		protected:
			std::wstring m_descriptorString;
			RAII::LocalHeapUniquePtr<std::remove_pointer<PSECURITY_DESCRIPTOR>::type> m_descriptor;
	};
}