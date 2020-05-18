#pragma once

namespace Boring32::WinHttp
{
	enum class WebSocketStatus
	{
		NotInitialised,
		Connected,
		Closed,
		Error
	};
}