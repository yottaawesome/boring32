export module boring32:winhttp_asyncwebsocketsettings;
import :crypto;
import :winhttp_session;
import boring32.shared;

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