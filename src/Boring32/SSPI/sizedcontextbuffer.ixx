export module boring32:sspi.sizedcontextbuffer;
import std;
import :win32;

export namespace Boring32::SSPI
{
	struct SizedContextBuffer final
	{
		~SizedContextBuffer()
		{
			Destroy();
		}
		// Needa a type
		SizedContextBuffer() {}

		void** GetAddress() noexcept
		{
			return reinterpret_cast<void**>(&m_buffer[0]);
		}

		void* Get() const noexcept
		{
			return reinterpret_cast<void*>(const_cast<std::byte*>(&m_buffer[0]));
		}

		void Destroy()
		{
			m_buffer.clear();
		}

		private:
		std::vector<std::byte> m_buffer;
	};
}