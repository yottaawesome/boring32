export module boring32.winhttp:asyncwebsocketsettings;
import boring32.crypto;
import :session;
import std;
import std.compat;

export namespace Boring32::WinHttp::WebSockets
{
	struct AsyncWebSocketSettings
	{
		std::wstring UserAgent;
		std::wstring Server;
		unsigned Port = 0;
		bool IgnoreSslErrors = false;
		std::wstring ConnectionHeaders;
		Crypto::Certificate ClientCert;
	};
}