module sspiclient.ntlm:functions;
import <string>;

namespace SSPIClient::NTLM
{
    void MyHandleError(const char* s)
    {

        fprintf(stderr, "%s error. Exiting.\n", s);
        exit(EXIT_FAILURE);
    }

    BOOL ConnectAuthSocket(
        SOCKET* s,
        CredHandle* hCred,
        SecHandle* hcText,
        const char* ServerName,
        const unsigned g_usPort
    )
    {
        unsigned long  ulAddress;
        ADDRINFOA* pHost = nullptr;
        SOCKADDR_IN    sin;

        //--------------------------------------------------------------------
        //  Lookup the server's address.
        int returnVal = inet_pton(
            AF_INET,
            ServerName,
            reinterpret_cast<void*>(&ulAddress)
        );
        if (0 == returnVal)
        {
            ADDRINFOA hints{
                .ai_family = AF_INET,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = IPPROTO_TCP
            };
            const int status = GetAddrInfoA(
                ServerName,
                std::to_string(g_usPort).c_str(),
                &hints,
                &pHost
            );
            if (NULL == pHost)
            {
                MyHandleError("Unable to resolve host name ");
                return(FALSE);
            }
            //ulAddress = pHost->ai_addr.S_un.S_addr
            //pHost->
            ;
            /*memcpy(
                (char FAR*)&ulAddress, 
                pHost->ai_addr->sa_data,
                14
            );
            FreeAddrInfoA(pHost);*/
        }

        //--------------------------------------------------------------------
        //  Create the socket.

        *s = socket(
            PF_INET,
            SOCK_STREAM,
            0);

        if (INVALID_SOCKET == *s)
        {
            MyHandleError("Unable to create socket");
        }
        else
        {
            printf("Socket created.\n");
        }

        if (pHost)
        {
            sin = *((sockaddr_in*)pHost->ai_addr);
            FreeAddrInfoA(pHost);
        }
        else
        {
            sin.sin_family = AF_INET;
            sin.sin_addr.S_un.S_addr = ulAddress;
            sin.sin_port = htons(g_usPort);
        }
        
        //--------------------------------------------------------------------
        //  Connect to the server.
        if (connect(*s, (LPSOCKADDR)&sin, sizeof(sin)))
        {
            closesocket(*s);
            MyHandleError("Connect failed ");
        }
        

        //--------------------------------------------------------------------
        //  Authenticate the connection. 
        // TODO
        /*if (!DoAuthentication(*s))
        {
            closesocket(*s);
            MyHandleError("Authentication ");
        }*/

        return(TRUE);
    }
}
