export module boring32.sspi:sizedcontextbuffer;
import <vector>;

export namespace Boring32::SSPI
{
	class SizedContextBuffer
	{
		public:
			virtual ~SizedContextBuffer();
			// Needa a type
			SizedContextBuffer();

		public:
			virtual void** GetAddress() noexcept;
			virtual void* Get() const noexcept;
			virtual void Destroy();

		protected:
			std::vector<std::byte> m_buffer;
	};
}