#pragma once
#include <string>

namespace Boring32::WinHttp::WebSockets
{
	struct WebSocketSettings
	{
		std::wstring UserAgent;
		std::wstring Server;
		std::wstring Proxies;
		std::wstring PacUrl;
		unsigned int Port = 0;
		bool IgnoreSslErrors = false;
	};
}