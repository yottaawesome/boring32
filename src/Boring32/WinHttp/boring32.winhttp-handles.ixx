export module boring32.winhttp:handles;
import std;
import std.compat;
import boring32.win32;

namespace Boring32::WinHttp
{
	using SharedWinHttpSession = std::shared_ptr<std::remove_pointer<Win32::WinHttp::HINTERNET>::type>;
}