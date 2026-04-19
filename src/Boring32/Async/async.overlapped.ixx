export module boring32:async.overlapped;
import std;
import :win32;
import :error;
import :concepts;

export namespace Boring32::Async
{
	struct [[nodiscard("This must remain alive while the IO operation is in progress.")]] 
	Overlapped final : Win32::OVERLAPPED
	{
		~Overlapped() { Close(); }

		Overlapped(const Overlapped&) = delete;
		auto operator=(const Overlapped&) -> Overlapped& = delete;

		// TODO: OVERLAPPED is not movable either.
		// May need to add a Reset() method.
		Overlapped(Overlapped&& other) { swap(other); };
		auto operator=(Overlapped&& other) -> Overlapped&
		{ 
			swap(other); 
			return *this; 
		};

		Overlapped() : Win32::OVERLAPPED{}
		{
			hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
			if (not hEvent)
				throw Error::Win32Error{ Win32::GetLastError(), "CreateEventW() failed" };
		}

		auto Wait(Concepts::Duration auto waitTime, std::string_view msg) -> bool
		{
			while (not Wait(waitTime))
				std::println("{}", msg);
			return true;
		}

		auto Wait() -> bool
		{
			return Wait(std::chrono::milliseconds{ Win32::Infinite });
		}

		auto Wait(Concepts::Duration auto millis) -> bool
		{
			if (not hEvent)
				throw std::runtime_error("No event");

			auto count = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(millis).count());
			return Win32::WaitForSingleObject(hEvent, static_cast<Win32::DWORD>(count)) == Win32::WaitObject0;
		}

		auto IsPartial() const noexcept -> bool
		{
			return this->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
		}

		auto Pending() const noexcept -> bool
		{
			return Internal == 0x00000103L; // STATUS_IO_PENDING
		}

		auto GetBytesRead() const noexcept -> std::uint64_t
		{
			return this->InternalHigh;
		}

		auto swap(Overlapped& other) noexcept -> void
		{
			Win32::HANDLE temp = hEvent;
			hEvent = other.hEvent;
			other.hEvent = temp;
		}

		auto Close() noexcept -> void
		{
			if (hEvent)
			{
				Win32::CloseHandle(hEvent);
				hEvent = nullptr;
			}
		}

		auto reset() noexcept -> void
		{
			Close();
		}
	};
	static_assert(std::movable<Overlapped>);
	static_assert(not std::copyable<Overlapped>);
	static_assert(std::swappable<Overlapped>);
}
