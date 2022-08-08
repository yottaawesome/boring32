module;

#include <Windows.h>

export module boring32.security:impersonationcontext;

export namespace Boring32::Security
{
	class ImpersonationContext
	{
		public:
			virtual ~ImpersonationContext();
			ImpersonationContext(const ImpersonationContext&) = delete;
			ImpersonationContext(ImpersonationContext&& other) noexcept;
			ImpersonationContext(HANDLE const token);

		public:
			virtual ImpersonationContext& operator=(const ImpersonationContext&) = delete;
			virtual ImpersonationContext& operator=(ImpersonationContext&&) noexcept = delete;

		public:
			virtual bool Close();
			virtual HKEY GetUserRegistry();

		protected:
			virtual ImpersonationContext& Move(ImpersonationContext& other) noexcept;

		protected:
			HKEY m_registryHive;
	};
}