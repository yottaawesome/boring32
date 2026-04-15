export module boring32:raii.uniquehandle;
import std;
import :win32;
import :error;

export namespace Boring32::RAII
{
	/// Exclusive-ownership RAII wrapper for Win32 HANDLEs.
	/// Move-only; analogous to std::unique_ptr.
	class UniqueHandle final
	{
	public:
		~UniqueHandle()
		{
			Close();
		}

		UniqueHandle() = default;

		UniqueHandle(const Win32::HANDLE handle)
			: m_handle(handle)
		{ }

		UniqueHandle(const UniqueHandle&) = delete;
		UniqueHandle& operator=(const UniqueHandle&) = delete;

		UniqueHandle(UniqueHandle&& other) noexcept
			: m_handle(other.m_handle)
		{
			other.m_handle = nullptr;
		}

		UniqueHandle& operator=(UniqueHandle&& other) noexcept
		{
			if (std::addressof(other) != this)
			{
				Close();
				m_handle = other.m_handle;
				other.m_handle = nullptr;
			}
			return *this;
		}

		/// Closes the current handle, if valid, and assumes
		/// ownership of the handle in the parameter.
		UniqueHandle& operator=(const Win32::HANDLE other)
		{
			Close();
			m_handle = other;
			return *this;
		}

		/// Compares the current handle to the specified handle.
		/// A nullptr handle is considered equivalent to INVALID_HANDLE_VALUE.
		bool operator==(const Win32::HANDLE other) const noexcept
		{
			if (not IsValidValue(other))
				return not IsValidValue(m_handle);
			return m_handle == other;
		}

		bool operator==(const UniqueHandle& other) const noexcept
		{
			if (not IsValidValue(other.m_handle))
				return not IsValidValue(m_handle);
			return m_handle == other.m_handle;
		}

		operator bool() const noexcept
		{
			return IsValidValue(m_handle);
		}

		Win32::HANDLE* operator&() noexcept
		{
			return &m_handle;
		}

		constexpr Win32::HANDLE operator*() const noexcept
		{
			return m_handle;
		}

		constexpr operator Win32::HANDLE() const noexcept
		{
			return m_handle;
		}

		// API
		constexpr Win32::HANDLE GetHandle() const noexcept
		{
			return m_handle;
		}

		Win32::HANDLE DuplicateCurrentHandle() const
		{
			return IsValidValue(m_handle)
				? UniqueHandle::DuplicatePassedHandle(m_handle, IsInheritable())
				: nullptr;
		}

		bool IsInheritable() const
		{
			if (not IsValidValue(m_handle))
				return false;
			return UniqueHandle::HandleIsInheritable(m_handle);
		}

		void Close() noexcept
		{
			if (IsValidValue(m_handle))
			{
				if (not Win32::CloseHandle(m_handle))
					std::wcerr << L"Failed to close handle\n";
			}
			m_handle = nullptr;
		}

		void SetInheritability(const bool isInheritable)
		{
			if (not IsValidValue(m_handle))
				throw Error::Boring32Error("handle is null or invalid.");
			if (not Win32::SetHandleInformation(m_handle, Win32::HandleFlagInherit, isInheritable))
				throw Error::Win32Error{Win32::GetLastError(), "SetHandleInformation() failed"};
		}

		Win32::HANDLE Detach() noexcept
		{
			Win32::HANDLE temp = m_handle;
			m_handle = nullptr;
			return temp;
		}

		bool IsValidValue() const noexcept
		{
			return IsValidValue(m_handle);
		}

		static constexpr bool IsValidValue(Win32::HANDLE handle) noexcept
		{
			return handle && handle != Win32::InvalidHandleValue;
		}

		static bool HandleIsInheritable(const Win32::HANDLE handle)
		{
			if (not handle)
				return false;
			if (handle == Win32::InvalidHandleValue)
				return false;

			Win32::DWORD flags = 0;
			if (not Win32::GetHandleInformation(handle, &flags))
				throw Error::Win32Error(Win32::GetLastError(), "GetHandleInformation() failed");
			return flags & Win32::HandleFlagInherit;
		}

		static Win32::HANDLE DuplicatePassedHandle(const Win32::HANDLE handle, const bool isInheritable)
		{
			if (not handle)
				return nullptr;
			if (handle == Win32::InvalidHandleValue)
				return Win32::InvalidHandleValue;

			Win32::HANDLE duplicateHandle = nullptr;
			bool succeeded = Win32::DuplicateHandle(
				Win32::GetCurrentProcess(),
				handle,
				Win32::GetCurrentProcess(),
				&duplicateHandle,
				0,
				isInheritable,
				Win32::DuplicateSameAccess
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "DuplicateHandle() failed"};

			return duplicateHandle;
		}

	private:
		Win32::HANDLE m_handle = nullptr;
	};
}
