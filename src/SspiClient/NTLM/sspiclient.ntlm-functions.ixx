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
    BOOL DoAuthentication(SOCKET s);
    BOOL SendMsg(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf
    );
    BOOL ReceiveMsg(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf,
        DWORD* pcbRead
    );
    BOOL SendBytes(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf
    );
    BOOL ReceiveBytes(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf,
        DWORD* pcbRead
    );
}
