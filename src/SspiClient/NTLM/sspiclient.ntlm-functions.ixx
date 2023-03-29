export module sspiclient.ntlm:functions;
import <win32.hpp>;

export namespace SSPIClient::NTLM
{
    void MyHandleError(const char* s);
    BOOL ConnectAuthSocket(
        SOCKET* s,
        CredHandle* hCred,
        SecHandle* hcText,
        const char* ServerName,
        const unsigned g_usPort
    );
}
