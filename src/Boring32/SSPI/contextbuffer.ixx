export module boring32:sspi.contextbuffer;
import :win32;

export namespace Boring32::SSPI
{
	struct AllocatedContextBuffer final
	{
		~AllocatedContextBuffer()
		{
			Destroy();
		}

		void** GetAddress() noexcept
		{
			return reinterpret_cast<void**>(&m_buffer);
		}

		void* Get() const noexcept
		{
			return m_buffer.pvBuffer;
		}

		void Destroy()
		{
			if (m_buffer.pvBuffer)
			{
				Win32::FreeContextBuffer(m_buffer.pvBuffer);
				m_buffer = { 0 };
			}
		}

	private:
		Win32::SecBuffer m_buffer{ 0 };
		Win32::SecBufferDesc m_desc{ 0 };
	};

	

	class ContextBuffers
	{

	};
}