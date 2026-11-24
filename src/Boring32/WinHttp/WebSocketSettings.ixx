export module Boring32:WinHttp.WebSocketSettings;
import std;
import :Crypto;
import :WinHttp.Session;

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