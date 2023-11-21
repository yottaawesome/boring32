export module boring32.winhttp:requestresult;
import std;
import std.compat;

export namespace Boring32::WinHttp
{
	struct HttpRequestResult
	{
		unsigned StatusCode;
		std::string ResponseBody;
	};
}