export module boring32:winhttp_enums;

export namespace Boring32::WinHttp
{
	enum class WebSocketStatus
	{
		NotInitialised,
		Connected,
		Closing,
		Closed,
		Error
	};

	enum class ProxyType : unsigned long
	{
		NoProxy = 1,
		NamedProxy = 3,
		AutoProxy = 4,
	};
}
