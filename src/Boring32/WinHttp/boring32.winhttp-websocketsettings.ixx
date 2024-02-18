export module boring32.winhttp:websocketsettings;
import boring32.crypto;
import boring32.shared;
import :session;

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
		unsigned BufferBlockSize = 2048;
	};
}