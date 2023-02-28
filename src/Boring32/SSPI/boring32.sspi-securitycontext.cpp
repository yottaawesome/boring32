module boring32.sspi:securitycontext;

namespace Boring32::SSPI
{
	SecurityContext::SecurityContext(
		const bool sspiAllocatedBuffers,
		const unsigned flags
	) : m_sspiAllocatedBuffers(sspiAllocatedBuffers),
		m_flags(flags)
	{ }

	void SecurityContext::Init()
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

		if (m_sspiAllocatedBuffers)
			m_flags |= ISC_REQ_ALLOCATE_MEMORY;

		// https://learn.microsoft.com/en-us/windows/win32/secauthn/initializesecuritycontext--schannel
		/*SECURITY_STATUS SEC_Entry = InitializeSecurityContext(
			_In_opt_    PCredHandle    phCredential,
			_In_opt_    PCtxtHandle    phContext,
			_In_opt_    SEC_CHAR * pszTargetName,
			_In_        ULONG          fContextReq,
			_In_        ULONG          Reserved1,
			_In_        ULONG          TargetDataRep,
			_In_opt_    PSecBufferDesc pInput,
			_In_        ULONG          Reserved2,
			_Inout_opt_ PCtxtHandle    phNewContext,
			_Inout_opt_ PSecBufferDesc pOutput,
			_Out_       PULONG         pfContextAttr,
			_Out_opt_   PTimeStamp     ptsExpiry
		);*/
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