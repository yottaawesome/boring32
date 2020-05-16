#pragma once
#include <Windows.h>
#include <string>

namespace Boring32::WinHttp
{
	class WinHttpRequestResult
	{
		public:
			UINT StatusCode;
			std::string ResponseBody;
	};
}