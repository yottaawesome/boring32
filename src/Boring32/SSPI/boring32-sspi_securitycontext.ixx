export module boring32:sspi_securitycontext;
import boring32.shared;
import :error;
import :com;
import :sspi_buffertype;

export namespace Boring32::SSPI
{
	// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--general
	// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-initializesecuritycontextw
	// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--schannel
	// Should this be SchannelSecurityContext, or should we subclass it?
	// This should really be split up -- the security context function's
	// accpetable parameters are different for the different SSPI providers.
	// See also: https://learn.microsoft.com/en-us/windows-server/security/windows-authentication/security-support-provider-interface-architecture
	class SecurityContext
	{
		public:
			virtual ~SecurityContext() = default;
			SecurityContext() = default;
			SecurityContext(const SecurityContext&) = delete;
			SecurityContext(SecurityContext&&) noexcept = delete;
			SecurityContext(
				const bool sspiAllocatedBuffers,
				const unsigned flags,
				std::wstring target
			)
				: m_sspiAllocatedBuffers(sspiAllocatedBuffers),
				m_flags(flags),
				m_target(std::move(target))
			{ }

		public:
			virtual void Init(Win32::PCredHandle credHandle)
			{
				Win32::SecBufferDesc outBufferDesc{
					.ulVersion = 0,
					.cBuffers = static_cast<unsigned>(m_outBuffers.size()),
					.pBuffers = &m_outBuffers[0]
				};
				Win32::SecBufferDesc inBufferDesc{
					.ulVersion = 0,
					.cBuffers = static_cast<unsigned>(m_inBuffers.size()),
					.pBuffers = &m_inBuffers[0]
				};

				m_flags |= Win32::_ISC_REQ_CONFIDENTIALITY;
				if (m_sspiAllocatedBuffers)
					m_flags |= Win32::_ISC_REQ_ALLOCATE_MEMORY;

				Win32::TimeStamp lifetime;
				Win32::ULONG contextAttributes;
				// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--general
				// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--schannel
				// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-initializesecuritycontextw
				const Win32::SECURITY_STATUS status = Win32::InitializeSecurityContextW(
					credHandle,
					&m_ctxHandle,
					&m_target[0],
					m_flags,
					0,
					0, // SECURITY_NATIVE_DREP ... not used with Schannel
					&inBufferDesc,
					0,
					&m_ctxHandle,
					&outBufferDesc,
					&contextAttributes,
					&lifetime
				);
				if (COM::Failed(status))
				{
					// Need to throw some kind of exception here
					// https://learn.microsoft.com/en-us/windows/win32/secauthn/schannel-error-codes-for-tls-and-ssl-alerts
					// These look like HRESULTs, but not sure
				}
			}

			virtual void AddInBuffer(
				const BufferType type,
				const unsigned size,
				void* const ptr
			)
			{
				m_inBuffers.push_back({
					.cbBuffer = 0,
					.BufferType = static_cast<unsigned>(type),
					.pvBuffer = ptr
					});
			}

			virtual void AddOutBuffer(
				const BufferType type,
				const unsigned size,
				void* const ptr
			)
			{
				m_outBuffers.push_back({
					.cbBuffer = 0,
					.BufferType = static_cast<unsigned>(type),
					.pvBuffer = ptr
					});
			}

		protected:
			Win32::SecHandle m_context { 0 };
			bool m_initialised = false;
			std::vector<Win32::SecBuffer> m_inBuffers;
			std::vector<Win32::SecBuffer> m_outBuffers;
			unsigned m_flags = 0;
			bool m_sspiAllocatedBuffers = false;
			Win32::CtxtHandle m_ctxHandle { 0 };
			std::wstring m_target;
	};
}