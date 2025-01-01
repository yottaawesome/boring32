export module boring32:async_overlapped;
import std;
import boring32.win32;
import :error;
import :concepts;

export namespace Boring32::Async
{
	struct [[nodiscard("This must remain alive while the IO operation is in progress.")]] 
	Overlapped : Win32::OVERLAPPED
	{
		~Overlapped() { Close(); }

		Overlapped(const Overlapped&) = delete;
		Overlapped& operator=(const Overlapped&) = delete;

		Overlapped(Overlapped&& other) { swap(other); };
		Overlapped& operator=(Overlapped&& other) { swap(other); };

		Overlapped() : Win32::OVERLAPPED{}
		{
			hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
			if (not hEvent)
				throw Error::Win32Error(Win32::GetLastError(), "CreateEventW() failed");
		}

		bool Wait(Concepts::Duration auto waitTime, std::string_view msg)
		{
			while (not Wait(waitTime))
				std::println("{}", msg);
			return true;
		}

		bool Wait()
		{
			return Wait(std::chrono::milliseconds{ Win32::Infinite });
		}

		bool Wait(Concepts::Duration auto millis)
		{
			if (not hEvent)
				throw std::runtime_error("No event");

			auto count = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(millis).count());
			return Win32::WaitForSingleObject(hEvent, static_cast<Win32::DWORD>(count)) == Win32::WaitObject0;
		}

		bool IsPartial() const noexcept
		{
			return this->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
		}

		std::uint64_t GetBytesRead() const noexcept
		{
			return this->InternalHigh;
		}

		void swap(Overlapped& other) noexcept
		{
			Win32::HANDLE temp = hEvent;
			hEvent = other.hEvent;
			other.hEvent = temp;
		}

		void Close() noexcept
		{
			if (hEvent)
			{
				Win32::CloseHandle(hEvent);
				hEvent = nullptr;
			}
		}

		void reset() noexcept
		{
			Close();
		}
	};
	static_assert(std::movable<Overlapped>);
	static_assert(not std::copyable<Overlapped>);
	static_assert(std::swappable<Overlapped>);
}
