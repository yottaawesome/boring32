export module boring32:sspi_sizedcontextbuffer;
import boring32.shared;

export namespace Boring32::SSPI
{
	class SizedContextBuffer
	{
		public:
			virtual ~SizedContextBuffer()
			{
				Destroy();
			}
			// Needa a type
			SizedContextBuffer() {}

		public:
			virtual void** GetAddress() noexcept
			{
				return reinterpret_cast<void**>(&m_buffer[0]);
			}

			virtual void* Get() const noexcept
			{
				return reinterpret_cast<void*>(const_cast<std::byte*>(&m_buffer[0]));
			}

			virtual void Destroy()
			{
				m_buffer.clear();
			}

		protected:
			std::vector<std::byte> m_buffer;
	};
}