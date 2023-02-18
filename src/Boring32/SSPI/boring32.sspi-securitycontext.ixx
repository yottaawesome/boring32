export module boring32.sspi:securitycontext;
import <vector>;
import <win32.hpp>;
import :buffertype;

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
			SecurityContext(const SecurityContext&) = delete;
			SecurityContext(SecurityContext&&) noexcept = delete;
			SecurityContext(
				const bool sspiAllocatedBuffers,
				const unsigned flags
			);

		public:
			virtual void Init();
			virtual void AddInBuffer(
				const BufferType type,
				const unsigned size
			);
			virtual void AddOutBuffer(
				const BufferType type,
				const unsigned size
			);

		protected:
			SecHandle m_context = { 0 };
			bool m_initialised = false;
			std::vector<SecBuffer> m_inBuffers;
			std::vector<SecBuffer> m_outBuffers;
			unsigned m_flags = 0;
			bool m_sspiAllocatedBuffers = false;
	};
}