export module boring32.winhttp:winhttphandle;
import <memory>;
import <win32.hpp>;

export namespace Boring32::WinHttp
{
	class WinHttpHandle
	{
		public:
			virtual ~WinHttpHandle() = default;
			WinHttpHandle() = default;
			WinHttpHandle(WinHttpHandle&& other) noexcept = default;
			WinHttpHandle(const WinHttpHandle& other) = default;
			WinHttpHandle(HINTERNET handle);

		public:
			virtual operator bool() const noexcept;
			virtual bool operator==(const HINTERNET other) const noexcept;
			virtual WinHttpHandle& operator=(const HINTERNET handle);
			virtual WinHttpHandle& operator=(WinHttpHandle&& other) noexcept = default;
			virtual WinHttpHandle& operator=(const WinHttpHandle&) = default;
			
		public:
			virtual HINTERNET Get() const noexcept;
			virtual void Close();

		protected:
			std::shared_ptr<void> m_handle;
	};
}