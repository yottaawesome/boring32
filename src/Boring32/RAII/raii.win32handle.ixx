export module boring32:raii_win32handle;
import :raii_sharedhandle;

export namespace Boring32::RAII
{
	/// Deprecated: use UniqueHandle or SharedHandle directly.
	using Win32Handle = SharedHandle;
}