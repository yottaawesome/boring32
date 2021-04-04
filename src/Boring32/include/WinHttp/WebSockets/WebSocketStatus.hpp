#pragma once

namespace Boring32::WinHttp::WebSockets
{
	enum class WebSocketStatus
	{
		NotInitialised,
		Connected,
		Closing,
		Closed,
		Error
	};
}