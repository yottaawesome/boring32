#pragma once
#include <string>
#include "../Session.hpp"

import boring32.crypto.certificate;

namespace Boring32::WinHttp::WebSockets
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