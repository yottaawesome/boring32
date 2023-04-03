module sspiclient.ntlm:main;
import <stdexcept>;
import "win32.hpp";
import :functions;

constexpr unsigned BIG_BUFF = 2048;
#define ServerName  "localhost" //"127.0.0.1"
#define TargetName  "Royal"
#define g_usPort 2000

namespace SSPIClient::NTLM
{
	void Main()
	{
        SOCKET            Client_Socket;
        BYTE              Data[BIG_BUFF];
        PCHAR             pMessage;
        CredHandle        hCred;
        struct _SecHandle hCtxt;
        SECURITY_STATUS   ss;
        DWORD             cbRead;
        ULONG             cbMaxSignature;
        ULONG             cbSecurityTrailer;
        SecPkgContext_Sizes            SecPkgContextSizes;
        SecPkgContext_NegotiationInfo  SecPkgNegInfo;

        WSADATA wsaData;
        if (WSAStartup(0x0101, &wsaData))
        {
            throw std::runtime_error("Could not initialize winsock ");
        }

        SecHandle  hcText;

        ConnectSocket(
            &Client_Socket,
            ServerName,
            g_usPort
        );
        //  Authenticate the connection. 
        DoAuthentication(
            Client_Socket,
            &hCred,
            &hcText,
            TargetName
        );
	}
}