module;

#include <source_location>;

export module boring32.winhttp:requestresult;
import <string>;

export namespace Boring32::WinHttp
{
	struct HttpRequestResult
	{
		unsigned StatusCode;
		std::string ResponseBody;
	};
}