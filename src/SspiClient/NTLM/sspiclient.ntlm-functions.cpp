module sspiclient.ntlm:functions;
import <string>;

namespace SSPIClient::NTLM
{
    constexpr unsigned cbMaxMessage = 12000;
    constexpr unsigned MessageAttribute = ISC_REQ_CONFIDENTIALITY;

    inline bool SEC_SUCCESS(const DWORD Status) 
    {
        return Status >= 0;
    }

    void MyHandleError(const char* s)
    {
        fprintf(stderr, "%s error. Exiting.\n", s);
        exit(EXIT_FAILURE);
    }

    bool ConnectSocket(
        SOCKET* s,
        const char* serverName,
        const unsigned usPort
    )
    {
        unsigned long ulAddress;
        ADDRINFOA* pHost = nullptr;
        SOCKADDR_IN sin;

        //--------------------------------------------------------------------
        //  Lookup the server's address.
        int returnVal = inet_pton(
            AF_INET,
            serverName,
            reinterpret_cast<void*>(&ulAddress)
        );
        if (returnVal == 0)
        {
            ADDRINFOA hints{
                .ai_family = AF_INET,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = IPPROTO_TCP
            };
            const int status = GetAddrInfoA(
                serverName,
                std::to_string(usPort).c_str(),
                &hints,
                &pHost
            );
            if (pHost == nullptr)
            {
                MyHandleError("Unable to resolve host name ");
                return false;
            }
        }

        //--------------------------------------------------------------------
        //  Create the socket.
        *s = socket(
            PF_INET,
            SOCK_STREAM,
            0);

        if (*s == INVALID_SOCKET)
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
            sin.sin_port = htons(usPort);
        }
        
        //--------------------------------------------------------------------
        //  Connect to the server.
        if (connect(*s, (LPSOCKADDR)&sin, sizeof(sin)))
        {
            closesocket(*s);
            MyHandleError("Connect failed ");
        }

        return true;
    }

    bool DoAuthentication(
        SOCKET s,
        CredHandle* hCred,
        SecHandle* hcText,
        const char* targetName
    )
    {
        PBYTE pOutBuf;
        if (!(pOutBuf = (PBYTE)malloc(cbMaxMessage)))
        {
            MyHandleError("Memory allocation ");
        }

        bool fDone = false;
        DWORD cbOut = cbMaxMessage;
        if (!GenClientContext(
            nullptr,
            0,
            pOutBuf,
            &cbOut,
            &fDone,
            (SEC_WCHAR*)targetName,
            hCred,
            hcText
        ))
        {
            return false;
        }

        if (!SendMsg(s, pOutBuf, cbOut))
        {
            MyHandleError("Send message failed ");
        }

        PBYTE pInBuf;
        if (!(pInBuf = (PBYTE)malloc(cbMaxMessage)))
        {
            MyHandleError("Memory allocation ");
        }
        DWORD cbIn = 0;
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

            if (!GenClientContext(
                pInBuf,
                cbIn,
                pOutBuf,
                &cbOut,
                &fDone,
                (SEC_WCHAR*)targetName,
                hCred,
                hcText))
            {
                MyHandleError("GenClientContext failed");
            }
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
        return true;
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

    BOOL GenClientContext(
        BYTE* pIn,
        DWORD       cbIn,
        BYTE* pOut,
        DWORD* pcbOut,
        bool* pfDone,
        SEC_WCHAR* pszTarget,
        CredHandle* hCred,
        SecHandle* hcText)
    {
        SECURITY_STATUS   ss;
        TimeStamp         Lifetime;
        SecBufferDesc     OutBuffDesc;
        SecBuffer         OutSecBuff;
        SecBufferDesc     InBuffDesc;
        SecBuffer         InSecBuff;
        ULONG             ContextAttributes;
        static PTCHAR     lpPackageName = (PTCHAR)NEGOSSP_NAME;

        if (NULL == pIn)
        {
            ss = AcquireCredentialsHandle(
                NULL,
                lpPackageName,
                SECPKG_CRED_OUTBOUND,
                NULL,
                NULL,
                NULL,
                NULL,
                hCred,
                &Lifetime);

            if (!(SEC_SUCCESS(ss)))
            {
                MyHandleError("AcquireCreds failed ");
            }
        }

        //--------------------------------------------------------------------
        //  Prepare the buffers.

        OutBuffDesc.ulVersion = 0;
        OutBuffDesc.cBuffers = 1;
        OutBuffDesc.pBuffers = &OutSecBuff;

        OutSecBuff.cbBuffer = *pcbOut;
        OutSecBuff.BufferType = SECBUFFER_TOKEN;
        OutSecBuff.pvBuffer = pOut;

        //-------------------------------------------------------------------
        //  The input buffer is created only if a message has been received 
        //  from the server.

        if (pIn)
        {
            InBuffDesc.ulVersion = 0;
            InBuffDesc.cBuffers = 1;
            InBuffDesc.pBuffers = &InSecBuff;

            InSecBuff.cbBuffer = cbIn;
            InSecBuff.BufferType = SECBUFFER_TOKEN;
            InSecBuff.pvBuffer = pIn;

            ss = InitializeSecurityContext(
                hCred,
                hcText,
                (SEC_WCHAR*)pszTarget,
                MessageAttribute,
                0,
                SECURITY_NATIVE_DREP,
                &InBuffDesc,
                0,
                hcText,
                &OutBuffDesc,
                &ContextAttributes,
                &Lifetime);
        }
        else
        {
            ss = InitializeSecurityContext(
                hCred,
                NULL,
                (SEC_WCHAR*)pszTarget,
                MessageAttribute,
                0,
                SECURITY_NATIVE_DREP,
                NULL,
                0,
                hcText,
                &OutBuffDesc,
                &ContextAttributes,
                &Lifetime);
        }

        if (!SEC_SUCCESS(ss))
        {
            MyHandleError("InitializeSecurityContext failed ");
        }

        //-------------------------------------------------------------------
        //  If necessary, complete the token.

        if ((SEC_I_COMPLETE_NEEDED == ss)
            || (SEC_I_COMPLETE_AND_CONTINUE == ss))
        {
            ss = CompleteAuthToken(hcText, &OutBuffDesc);
            if (!SEC_SUCCESS(ss))
            {
                fprintf(stderr, "complete failed: 0x%08x\n", ss);
                return FALSE;
            }
        }

        *pcbOut = OutSecBuff.cbBuffer;

        *pfDone = !((SEC_I_CONTINUE_NEEDED == ss) ||
            (SEC_I_COMPLETE_AND_CONTINUE == ss));

        printf("Token buffer generated (%lu bytes):\n", OutSecBuff.cbBuffer);
        PrintHexDump(OutSecBuff.cbBuffer, (PBYTE)OutSecBuff.pvBuffer);
        return TRUE;

    }

    void PrintHexDump(
        DWORD length,
        PBYTE buffer)
    {
        DWORD i, count, index;
        CHAR rgbDigits[] = "0123456789abcdef";
        CHAR rgbLine[100];
        char cbLine;

        for (index = 0; length;
            length -= count, buffer += count, index += count)
        {
            count = (length > 16) ? 16 : length;

            sprintf_s(rgbLine, 100, "%4.4x  ", index);
            cbLine = 6;

            for (i = 0; i < count; i++)
            {
                rgbLine[cbLine++] = rgbDigits[buffer[i] >> 4];
                rgbLine[cbLine++] = rgbDigits[buffer[i] & 0x0f];
                if (i == 7)
                {
                    rgbLine[cbLine++] = ':';
                }
                else
                {
                    rgbLine[cbLine++] = ' ';
                }
            }
            for (; i < 16; i++)
            {
                rgbLine[cbLine++] = ' ';
                rgbLine[cbLine++] = ' ';
                rgbLine[cbLine++] = ' ';
            }

            rgbLine[cbLine++] = ' ';

            for (i = 0; i < count; i++)
            {
                if (buffer[i] < 32 || buffer[i] > 126)
                {
                    rgbLine[cbLine++] = '.';
                }
                else
                {
                    rgbLine[cbLine++] = buffer[i];
                }
            }

            rgbLine[cbLine++] = 0;
            printf("%s\n", rgbLine);
        }
    }
}
