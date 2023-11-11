export module boring32.security:encodedpointer;
import boring32.win32;

export namespace Boring32::Security
{
	template<typename T>
	class EncodedPointer final
	{
		EncodedPointer(const T* ptr)
		{
			m_encoded = Win32::EncodePointer(ptr);
		}

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

			EncodedPointer& operator=(const T* other)
			{
				m_encoded = other;
				return *this;
			}

		private:
			void* m_encoded = nullptr;
	};
}
