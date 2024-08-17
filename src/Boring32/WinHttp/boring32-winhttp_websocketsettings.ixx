export module boring32:winhttp_websocketsettings;
import :crypto;
import boring32.shared;
import :winhttp_session;

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