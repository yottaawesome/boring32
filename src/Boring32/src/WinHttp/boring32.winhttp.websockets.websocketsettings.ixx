module;

#include <string>
#include "include/WinHttp/Session.hpp"

export module boring32.winhttp.websockets.websocketsettings;
import boring32.crypto.certificate;

export namespace Boring32::WinHttp::WebSockets
{
	struct WebSocketSettings
	{
		std::wstring UserAgent;
		std::wstring Server;
		unsigned int Port = 0;
		bool IgnoreSslErrors = false;
		std::wstring ConnectionHeaders;
		Session WinHttpSession;
		Crypto::Certificate ClientCert;
		UINT BufferBlockSize = 2048;
	};
}