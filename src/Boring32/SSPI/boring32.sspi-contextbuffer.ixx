export module boring32.sspi:contextbuffer;
import std;

import <win32.hpp>;

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
				FreeContextBuffer(m_buffer.pvBuffer);
				m_buffer = { 0 };
			}
		}

		private:
			SecBuffer m_buffer{ 0 };
			SecBufferDesc m_desc{ 0 };
	};

	

	class ContextBuffers
	{

	};
}