#pragma once
#include <Windows.h>
#include <string>

namespace Boring32::WinHttp
{
	class HttpRequestResult
	{
		public:
			UINT StatusCode;
			std::string ResponseBody;
	};
}