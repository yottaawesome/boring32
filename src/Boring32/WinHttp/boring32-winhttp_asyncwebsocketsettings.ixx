export module boring32:winhttp_asyncwebsocketsettings;
import std;
import :crypto;
import :winhttp_session;

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