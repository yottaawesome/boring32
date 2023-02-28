module boring32.sspi:securitycontext;
import <string>;
import boring32.error;

namespace Boring32::SSPI
{
	SecurityContext::SecurityContext(
		const bool sspiAllocatedBuffers,
		const unsigned flags,
		std::wstring target
	) : m_sspiAllocatedBuffers(sspiAllocatedBuffers),
		m_flags(flags),
		m_target(std::move(target))
	{ }

	void SecurityContext::Init(PCredHandle credHandle)
	{
		SecBufferDesc outBufferDesc{
			.ulVersion = 0,
			.cBuffers = static_cast<unsigned>(m_outBuffers.size()),
			.pBuffers = &m_outBuffers[0]
		};
		SecBufferDesc inBufferDesc{
			.ulVersion = 0,
			.cBuffers = static_cast<unsigned>(m_inBuffers.size()),
			.pBuffers = &m_inBuffers[0]
		};

		m_flags |= ISC_REQ_CONFIDENTIALITY;
		if (m_sspiAllocatedBuffers)
			m_flags |= ISC_REQ_ALLOCATE_MEMORY;

		TimeStamp lifetime;
		ULONG contextAttributes;
		// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--general
		// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--schannel
		// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-initializesecuritycontextw
		SECURITY_STATUS status = InitializeSecurityContextW(
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
		if (FAILED(status))
		{
			// Need to throw some kind of exception here
			// https://learn.microsoft.com/en-us/windows/win32/secauthn/schannel-error-codes-for-tls-and-ssl-alerts
			// These look like HRESULTs
		}
	}

	void SecurityContext::AddInBuffer(
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

	void SecurityContext::AddOutBuffer(
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
}