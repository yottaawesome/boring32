#pragma once

namespace Boring32::WinHttp
{
	enum class WinHttpWebSocketStatus
	{
		NotInitialised,
		Connected,
		Closed,
		Error
	};
}