export module boring32.winhttp:handles;
import <memory>;
import <win32.hpp>;

namespace Boring32::WinHttp
{
	using SharedWinHttpSession = std::shared_ptr<std::remove_pointer<HINTERNET>::type>;
}