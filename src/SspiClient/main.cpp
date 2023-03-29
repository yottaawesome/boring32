#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Secur32.lib")

import sspiclient.ntlm;

int main()
{
    SSPIClient::NTLM::Main();
    return 0;
}
