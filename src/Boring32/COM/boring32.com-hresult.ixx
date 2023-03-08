export module boring32.com:hresult;
import <win32.hpp>;

export namespace Boring32::COM
{
	class HResult
	{
		public:
			virtual ~HResult() = default;
			HResult() = default;
			HResult(HResult&&) noexcept = default;
			HResult(const HResult&) = default;
			HResult(const HRESULT hr);

		public:
			virtual operator HRESULT() const noexcept;
			virtual HResult& operator=(const HRESULT hr) noexcept;

		protected:
			HRESULT m_hr = 0x0;
	};
}
