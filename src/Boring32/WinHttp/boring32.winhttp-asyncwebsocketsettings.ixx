module;

#include <source_location>;

export module boring32.winhttp:asyncwebsocketsettings;
import boring32.crypto;
import :session;
import <string>;
import <win32.hpp>;

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