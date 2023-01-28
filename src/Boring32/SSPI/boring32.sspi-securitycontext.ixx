export module boring32.sspi:securitycontext;
import <win32.hpp>;

export namespace Boring32::SSPI
{
	// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--general
	// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-initializesecuritycontextw
	// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--schannel
	// Should this be SchannelSecurityContext, or should we subclass it?
	class SecurityContext
	{
		public:
			virtual ~SecurityContext() = default;
			SecurityContext() = default;

		protected:
			virtual void Init();

		protected:
			SecHandle m_context = { 0 };
			bool m_initialised = false;
	};
}