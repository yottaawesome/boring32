module;

#include <string>
#include <Windows.h>

export module boring32.security.securitydescriptor;
import boring32.raii.uniqueptrs;

export namespace Boring32::Security
{
	class SecurityDescriptor
	{
		public:
			virtual ~SecurityDescriptor();

			SecurityDescriptor(std::wstring descriptorString);
			SecurityDescriptor(const SecurityDescriptor&) = delete;

		public:
			virtual SecurityDescriptor& operator=(const SecurityDescriptor&) = delete;
			virtual operator std::wstring() const noexcept;
			virtual operator PSECURITY_DESCRIPTOR() const noexcept;

		public:
			virtual void Close();
			[[nodiscard]] virtual PSECURITY_DESCRIPTOR GetDescriptor() const noexcept;
			[[nodiscard]] virtual const std::wstring& GetDescriptorString() const noexcept;

		protected:
			virtual void Create();

		protected:
			std::wstring m_descriptorString;
			Raii::LocalHeapUniquePtr m_descriptor;
	};
}