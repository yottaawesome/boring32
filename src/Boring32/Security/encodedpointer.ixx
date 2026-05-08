export module boring32:security.encodedpointer;
import :win32;

export namespace Boring32::Security
{
	template<typename T>
	class EncodedPointer final
	{
	public:
		EncodedPointer(const T* ptr) noexcept
			: m_encoded(Win32::EncodePointer(ptr))
		{ }

		auto operator->() noexcept -> T*
		{
			return Win32::DecodePointer(m_encoded);
		}

		auto operator->() const noexcept -> const T*
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

		auto operator=(const T* other) noexcept -> EncodedPointer&
		{
			m_encoded = other;
			return *this;
		}

	private:
		void* m_encoded = nullptr;
	};
}
