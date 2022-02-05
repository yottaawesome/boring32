module;

#include <string>

export module boring32.winhttp.requestresult;

export namespace Boring32::WinHttp
{
	struct HttpRequestResult
	{
		unsigned StatusCode;
		std::string ResponseBody;
	};
}