module sspiclient.ntlm:functions;
import <string>;

namespace SSPIClient::NTLM
{
    constexpr unsigned cbMaxMessage = 12000;

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

    BOOL DoAuthentication(SOCKET s)
    {
        BOOL        fDone = FALSE;
        DWORD       cbOut = 0;
        DWORD       cbIn = 0;
        PBYTE       pInBuf;
        PBYTE       pOutBuf;


        if (!(pInBuf = (PBYTE)malloc(cbMaxMessage)))
        {
            MyHandleError("Memory allocation ");
        }

        if (!(pOutBuf = (PBYTE)malloc(cbMaxMessage)))
        {
            MyHandleError("Memory allocation ");
        }

        cbOut = cbMaxMessage;
        /*if (!GenClientContext(
            NULL,
            0,
            pOutBuf,
            &cbOut,
            &fDone,
            (SEC_WCHAR*)TargetName,
            &hCred,
            &hcText
        ))
        {
            return(FALSE);
        }*/

        if (!SendMsg(s, pOutBuf, cbOut))
        {
            MyHandleError("Send message failed ");
        }

        while (!fDone)
        {
            if (!ReceiveMsg(
                s,
                pInBuf,
                cbMaxMessage,
                &cbIn))
            {
                MyHandleError("Receive message failed ");
            }

            cbOut = cbMaxMessage;

            /*if (!GenClientContext(
                pInBuf,
                cbIn,
                pOutBuf,
                &cbOut,
                &fDone,
                (SEC_WCHAR*)TargetName,
                &hCred,
                &hcText))
            {
                MyHandleError("GenClientContext failed");
            }*/
            if (!SendMsg(
                s,
                pOutBuf,
                cbOut))
            {
                MyHandleError("Send message 2  failed ");
            }
        }

        free(pInBuf);
        free(pOutBuf);
        return(TRUE);
    }

    BOOL SendMsg(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf)
    {
        if (0 == cbBuf)
            return(TRUE);

        //----------------------------------------------------------
        //  Send the size of the message.

        if (!SendBytes(s, (PBYTE)&cbBuf, sizeof(cbBuf)))
            return(FALSE);

        //----------------------------------------------------------
        //  Send the body of the message.

        if (!SendBytes(
            s,
            pBuf,
            cbBuf))
        {
            return(FALSE);
        }

        return(TRUE);
    }

    BOOL ReceiveMsg(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf,
        DWORD* pcbRead)

    {
        DWORD cbRead;
        DWORD cbData;

        //----------------------------------------------------------
        //  Receive the number of bytes in the message.

        if (!ReceiveBytes(
            s,
            (PBYTE)&cbData,
            sizeof(cbData),
            &cbRead))
        {
            return(FALSE);
        }

        if (sizeof(cbData) != cbRead)
            return(FALSE);
        //----------------------------------------------------------
        //  Read the full message.

        if (!ReceiveBytes(
            s,
            pBuf,
            cbData,
            &cbRead))
        {
            return(FALSE);
        }

        if (cbRead != cbData)
            return(FALSE);

        *pcbRead = cbRead;
        return(TRUE);
    }  // end ReceiveMessage  

    BOOL SendBytes(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf)
    {
        PBYTE pTemp = pBuf;
        int   cbSent;
        int   cbRemaining = cbBuf;

        if (0 == cbBuf)
            return(TRUE);

        while (cbRemaining)
        {
            cbSent = send(
                s,
                (const char*)pTemp,
                cbRemaining,
                0);
            if (SOCKET_ERROR == cbSent)
            {
                fprintf(stderr, "send failed: %u\n", GetLastError());
                return FALSE;
            }

            pTemp += cbSent;
            cbRemaining -= cbSent;
        }

        return TRUE;
    }

    BOOL ReceiveBytes(
        SOCKET  s,
        PBYTE   pBuf,
        DWORD   cbBuf,
        DWORD* pcbRead)
    {
        PBYTE pTemp = pBuf;
        int cbRead, cbRemaining = cbBuf;

        while (cbRemaining)
        {
            cbRead = recv(
                s,
                (char*)pTemp,
                cbRemaining,
                0);
            if (0 == cbRead)
                break;
            if (SOCKET_ERROR == cbRead)
            {
                fprintf(stderr, "recv failed: %u\n", GetLastError());
                return FALSE;
            }

            cbRemaining -= cbRead;
            pTemp += cbRead;
        }

        *pcbRead = cbBuf - cbRemaining;

        return TRUE;
    }  // end ReceiveBytes
}
