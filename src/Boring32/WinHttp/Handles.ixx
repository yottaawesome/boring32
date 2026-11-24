export module Boring32:WinHttp.Handles;
import std;
import :Win32;

namespace Boring32::WinHttp
{
	using SharedWinHttpSession = std::shared_ptr<std::remove_pointer_t<Win32::WinHttp::HINTERNET>>;
}