export module boring32:security_encodedpointer;
import boring32.shared;

export namespace Boring32::Security
{
	template<typename T>
	class EncodedPointer final
	{
		EncodedPointer(const T* ptr) noexcept
			: m_encoded(Win32::EncodePointer(ptr))
		{ }

		public:
			T* operator->() noexcept
			{
				return Win32::DecodePointer(m_encoded);
			}

			const T* operator->() const noexcept
			{
				return Win32::DecodePointer(m_encoded);
			}

			operator T* () noexcept
			{
				return Win32::DecodePointer(m_encoded);
			}

			operator const T* () const noexcept
			{
				return Win32::DecodePointer(m_encoded);
			}

			EncodedPointer& operator=(const T* other) noexcept
			{
				m_encoded = other;
				return *this;
			}

		private:
			void* m_encoded = nullptr;
	};
}
