module;

#include <memory>
#include <Windows.h>
#include <winhttp.h>

export module boring32.winhttp:handles;

namespace Boring32::WinHttp
{
	using SharedWinHttpSession = std::shared_ptr<std::remove_pointer<HINTERNET>::type>;
}