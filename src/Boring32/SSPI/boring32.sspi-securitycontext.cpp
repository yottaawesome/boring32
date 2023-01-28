module boring32.sspi:securitycontext;

namespace Boring32::SSPI
{
	void SecurityContext::Init()
	{
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
}