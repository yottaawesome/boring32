export module boring32.sspi:buffertype;
import <win32.hpp>;

export namespace Boring32::SSPI
{
	enum class BufferType
	{
		Token = SECBUFFER_TOKEN,
		Data = SECBUFFER_DATA
	};
}