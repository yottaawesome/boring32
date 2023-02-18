module boring32.sspi:sizedcontextbuffer;

namespace Boring32::SSPI
{
	SizedContextBuffer::~SizedContextBuffer()
	{
		Destroy();
	}

	SizedContextBuffer::SizedContextBuffer()
	{

	}

	void** SizedContextBuffer::GetAddress() noexcept
	{
		return reinterpret_cast<void**>(&m_buffer[0]);
	}

	void* SizedContextBuffer::Get() const noexcept
	{
		return reinterpret_cast<void*>(const_cast<std::byte*>(&m_buffer[0]));
	}

	void SizedContextBuffer::Destroy()
	{
		m_buffer.clear();
	}
}