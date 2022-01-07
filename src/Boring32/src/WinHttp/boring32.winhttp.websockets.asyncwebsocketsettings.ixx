module;

#include <string>
#include <Windows.h>

export module boring32.winhttp.websockets.asyncwebsocketsettings;
import boring32.crypto.certificate;
import boring32.winhttp.session;

export namespace Boring32::WinHttp::WebSockets
{
	struct AsyncWebSocketSettings
	{
		std::wstring UserAgent;
		std::wstring Server;
		UINT Port = 0;
		bool IgnoreSslErrors = false;
		std::wstring ConnectionHeaders;
		Crypto::Certificate ClientCert;
	};
}