module;

#include <Windows.h>

export module boring32.async.waitableaddress;

export namespace Boring32::Async
{
	// TODO: need to support values that can be 1, 2, 4, or 8 bytes long
	class WaitableAddress
	{
		public:
			enum class WakeType
			{
				One,
				All
			};

		public:
			virtual ~WaitableAddress();
			WaitableAddress();
			WaitableAddress(const WaitableAddress& other) = delete;
			WaitableAddress(WaitableAddress&& other) noexcept = delete;
			WaitableAddress(const DWORD defaultValue, const DWORD waitValue);

		public:
			virtual WaitableAddress& operator=(const WaitableAddress& other) = delete;
			virtual WaitableAddress& operator=(WaitableAddress&& other) noexcept = delete;

		public:
			virtual bool Wait() const;
			virtual bool Wait(const DWORD millis) const;
			virtual void SetValue(const DWORD newValue, const WakeType wakeType);
			virtual DWORD GetValue() const noexcept;
			virtual DWORD GetDefaultValue() const noexcept;

		protected:
			DWORD m_defaultValue;
			DWORD m_waitValue;
	};
}