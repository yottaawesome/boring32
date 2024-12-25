export module boring32:winhttp_requestresult;
import std;

export namespace Boring32::WinHttp
{
	struct HttpRequestResult
	{
		unsigned StatusCode;
		std::string ResponseBody;
	};
}