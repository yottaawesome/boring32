export module boring32:raii_sharedhandle;
import std;
import :win32;
import :error;

export namespace Boring32::RAII
{
	/// Shared-ownership RAII wrapper for Win32 HANDLEs.
	/// Copyable via reference counting; analogous to std::shared_ptr.
	class SharedHandle final
	{
	public:
		~SharedHandle()
		{
			Close();
		}

		SharedHandle() : m_handle(CreateHandlePtr(nullptr)) { }
		SharedHandle(const Win32::HANDLE handle) : m_handle(CreateHandlePtr(handle)) { }
		SharedHandle(const SharedHandle& otherHandle) : m_handle(CreateHandlePtr(nullptr))
		{
			Copy(otherHandle);
		}

		SharedHandle(SharedHandle&& otherHandle) noexcept
			: m_handle(CreateHandlePtr(nullptr))
		{
			Move(otherHandle);
		}

		void operator=(const SharedHandle& other)
		{
			Copy(other);
		}
			
		SharedHandle& operator=(SharedHandle&& other) noexcept
		{
			Move(other);
			return *this;
		}

		/// Compares the current handle to the specified handle.
		/// A nullptr handle is considered equivalent to INVALID_HANDLE_VALUE.
		bool operator==(const Win32::HANDLE other) const noexcept
		{
			if (not IsValidValue(other))
				return not IsValidValue(m_handle);
			return m_handle && *m_handle == other;
		}

		/// Closes the current handle, if valid, and assumes
		/// ownership of handle in the parameter.
		SharedHandle& operator=(const Win32::HANDLE other)
		{
			Close();
			m_handle = CreateHandlePtr(other);
			return *this;
		}

		/// Compares the current handle to the specified handle.
		/// A nullptr handle is considered equivalent to INVALID_HANDLE_VALUE.
		bool operator==(const SharedHandle& other) const
		{
			// https://devblogs.microsoft.com/oldnewthing/20040302-00/?p=40443
			if (not IsValidValue(other.m_handle))
				return not IsValidValue(m_handle);
			return m_handle == other.m_handle;
		}

		operator bool() const noexcept
		{
			return IsValidValue();
		}

		Win32::HANDLE* operator&()
		{
			if (not m_handle)
				m_handle = CreateHandlePtr(nullptr);
			return m_handle.get();
		}

		constexpr Win32::HANDLE operator*() const noexcept
		{
			return m_handle ? *m_handle : nullptr;
		}

		constexpr operator Win32::HANDLE() const noexcept
		{
			return m_handle ? *m_handle : nullptr;
		}

		// API
		constexpr Win32::HANDLE GetHandle() const noexcept
		{
			return m_handle ? *m_handle : nullptr;
		}

		Win32::HANDLE DuplicateCurrentHandle() const
		{
			return IsValidValue(m_handle)
				? SharedHandle::DuplicatePassedHandle(*m_handle, IsInheritable())
				: nullptr;
		}
			
		bool IsInheritable() const
		{
			if (not m_handle or not *m_handle)
				return false;
			return SharedHandle::HandleIsInheritable(*m_handle);
		}

		void Close() noexcept
		{
			m_handle = nullptr;
		}

		void SetInheritability(const bool isInheritable)
		{
			if (not IsValidValue())
				throw Error::Boring32Error("handle is null or invalid.");
			if (not Win32::SetHandleInformation(*m_handle, Win32::HandleFlagInherit, isInheritable))
				throw Error::Win32Error{Win32::GetLastError(), "SetHandleInformation() failed"};
		}

		Win32::HANDLE Detach() noexcept
		{
			if (not m_handle or not *m_handle)
				return nullptr;
			Win32::HANDLE temp = *m_handle;
			*m_handle = nullptr;
			return temp;
		}

		/// Returns whether the internal HANDLE's value is not 
		/// nullptr or INVALID_HANDLE_VALUE. Note: this does not mean
		/// the value is an actual current and valid HANDLE.
		bool IsValidValue() const noexcept
		{
			return m_handle && IsValidValue(*m_handle);
		}

		bool IsValidValue(const std::shared_ptr<Win32::HANDLE>& handle) const noexcept
		{
			return handle && IsValidValue(*handle);
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
		void Copy(const SharedHandle& other)
		{
			Close();
			m_handle = other.m_handle;
		}

		void Move(SharedHandle& other) noexcept
		{
			Close();
			m_handle = std::move(other.m_handle);
			other.m_handle = nullptr;
		}

		std::shared_ptr<Win32::HANDLE> CreateHandlePtr(Win32::HANDLE handle)
		{
			return { new void* (handle), &SharedHandle::CloseHandleAndFreeMemory };
		}

		static void CloseHandleAndFreeMemory(Win32::HANDLE* pHandle)
		{
			if (not pHandle)
				return;

			Win32::HANDLE wrappedHandle = *pHandle;
			if (wrappedHandle and wrappedHandle != Win32::InvalidHandleValue and not Win32::CloseHandle(wrappedHandle))
				std::wcerr << L"Failed to close handle\n";

			*pHandle = 0;
			delete pHandle;
		}

		std::shared_ptr<Win32::HANDLE> m_handle;
	};
}
