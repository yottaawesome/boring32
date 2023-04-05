export module sspiclient.ntlm:functions;
import <win32.hpp>;

export namespace SSPIClient::NTLM
{
    void MyHandleError(const char* s);
    bool ConnectSocket(
        SOCKET* s,
        const char* serverName,
        const unsigned usPort
    );
    bool DoAuthentication(
        SOCKET s,
        CredHandle* hCred,
        SecHandle* hcText,
        const char* targetName
    );
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
    BOOL GenClientContext(
        BYTE* pIn,
        DWORD       cbIn,
        BYTE* pOut,
        DWORD* pcbOut,
        bool* pfDone,
        SEC_WCHAR* pszTarget,
        CredHandle* hCred,
        SecHandle* hcText
    );
    void PrintHexDump(
        DWORD length,
        PBYTE buffer
    );
}
