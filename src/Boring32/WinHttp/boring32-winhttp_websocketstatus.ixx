export module boring32:winhttp_websocketstatus;

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