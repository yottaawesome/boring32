#pragma once

namespace Boring32::WinHttp
{
	enum class ProxyType : unsigned long
	{
		NoProxy = 1,
		NamedProxy = 3,
		AutoProxy = 4,
	};
}
