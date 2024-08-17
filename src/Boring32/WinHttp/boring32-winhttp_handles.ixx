export module boring32:winhttp_handles;
import boring32.shared;

namespace Boring32::WinHttp
{
	using SharedWinHttpSession = std::shared_ptr<std::remove_pointer<Win32::WinHttp::HINTERNET>::type>;
}