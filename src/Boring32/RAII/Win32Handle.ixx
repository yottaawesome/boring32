export module boring32:raii.win32handle;
import :raii.sharedhandle;

export namespace Boring32::RAII
{
	/// Deprecated: use UniqueHandle or SharedHandle directly.
	using Win32Handle = SharedHandle;
}