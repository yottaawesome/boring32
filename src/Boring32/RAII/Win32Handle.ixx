export module Boring32:Raii.Win32Handle;
import :Raii.SharedHandle;

export namespace Boring32::RAII
{
	/// Deprecated: use UniqueHandle or SharedHandle directly.
	using Win32Handle = SharedHandle;
}