export module boring32:winhttp_handles;
import std;
import boring32.win32;

namespace Boring32::WinHttp
{
	using SharedWinHttpSession = std::shared_ptr<std::remove_pointer_t<Win32::WinHttp::HINTERNET>>;
}