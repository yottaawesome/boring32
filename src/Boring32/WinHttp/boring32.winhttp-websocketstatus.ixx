module;

#include <source_location>;

export module boring32.winhttp:websocketstatus;

export namespace Boring32::WinHttp::WebSockets
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