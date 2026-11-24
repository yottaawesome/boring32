export module boring32:raii.sharedhandle;
import std;
import :win32;
import :error;

export namespace Boring32::RAII
{
	// Shared-ownership RAII wrapper for Win32 HANDLEs.
	// Copyable via reference counting; analogous to std::shared_ptr.
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
		auto operator=(const SharedHandle& other) -> SharedHandle&
		{
			Copy(other);
			return *this;
		}

		SharedHandle(SharedHandle&& otherHandle) noexcept
			: m_handle(CreateHandlePtr(nullptr))
		{
			Move(otherHandle);
		}
		auto operator=(SharedHandle&& other) noexcept -> SharedHandle&
		{
			Move(other);
			return *this;
		}

		// Compares the current handle to the specified handle.
		// A nullptr handle is considered equivalent to INVALID_HANDLE_VALUE.
		auto operator==(const Win32::HANDLE other) const noexcept -> bool
		{
			if (not IsValidValue(other))
				return not IsValidValue(m_handle);
			return m_handle && *m_handle == other;
		}

		// Closes the current handle, if valid, and assumes
		// ownership of handle in the parameter.
		auto operator=(const Win32::HANDLE other) -> SharedHandle&
		{
			Close();
			m_handle = CreateHandlePtr(other);
			return *this;
		}

		// Compares the current handle to the specified handle.
		// A nullptr handle is considered equivalent to INVALID_HANDLE_VALUE.
		auto operator==(const SharedHandle& other) const -> bool
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

		auto operator&() -> Win32::HANDLE*
		{
			if (not m_handle)
				m_handle = CreateHandlePtr(nullptr);
			return m_handle.get();
		}

		constexpr auto operator*() const noexcept -> Win32::HANDLE
		{
			return m_handle ? *m_handle : nullptr;
		}

		constexpr operator Win32::HANDLE() const noexcept
		{
			return m_handle ? *m_handle : nullptr;
		}

		// API
		constexpr auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_handle ? *m_handle : nullptr;
		}

		auto DuplicateCurrentHandle() const -> Win32::HANDLE
		{
			return IsValidValue(m_handle)
				? SharedHandle::DuplicatePassedHandle(*m_handle, IsInheritable())
				: nullptr;
		}
			
		auto IsInheritable() const -> bool
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
				throw Error::Boring32Error{ "handle is null or invalid." };
			if (not Win32::SetHandleInformation(*m_handle, Win32::HandleFlagInherit, isInheritable))
				throw Error::Win32Error{Win32::GetLastError(), "SetHandleInformation() failed"};
		}

		auto Detach() noexcept -> Win32::HANDLE
		{
			if (not m_handle or not *m_handle)
				return nullptr;
			auto temp = *m_handle;
			*m_handle = nullptr;
			return temp;
		}

		// Returns whether the internal HANDLE's value is not 
		// nullptr or INVALID_HANDLE_VALUE. Note: this does not mean
		// the value is an actual current and valid HANDLE.
		auto IsValidValue() const noexcept -> bool
		{
			return m_handle and IsValidValue(*m_handle);
		}

		auto IsValidValue(const std::shared_ptr<Win32::HANDLE>& handle) const noexcept -> bool
		{
			return handle and IsValidValue(*handle);
		}

		static constexpr auto IsValidValue(Win32::HANDLE handle) noexcept -> bool
		{
			return handle and handle != Win32::InvalidHandleValue;
		}

		static auto HandleIsInheritable(const Win32::HANDLE handle) -> bool
		{
			if (not handle)
				return false;
			if (handle == Win32::InvalidHandleValue)
				return false;

			auto flags = Win32::DWORD{};
			if (not Win32::GetHandleInformation(handle, &flags))
				throw Error::Win32Error{Win32::GetLastError(), "GetHandleInformation() failed"};
			return flags & Win32::HandleFlagInherit;
		}

		static auto DuplicatePassedHandle(const Win32::HANDLE handle, const bool isInheritable) -> Win32::HANDLE
		{
			if (not handle)
				return nullptr;
			if (handle == Win32::InvalidHandleValue)
				return Win32::InvalidHandleValue;

			auto duplicateHandle = Win32::HANDLE{};
			auto succeeded = Win32::DuplicateHandle(
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

		auto CreateHandlePtr(Win32::HANDLE handle) -> std::shared_ptr<Win32::HANDLE>
		{
			return { new void* (handle), &SharedHandle::CloseHandleAndFreeMemory };
		}

		static void CloseHandleAndFreeMemory(Win32::HANDLE* pHandle)
		{
			if (not pHandle)
				return;

			auto wrappedHandle = *pHandle;
			if (wrappedHandle and wrappedHandle != Win32::InvalidHandleValue and not Win32::CloseHandle(wrappedHandle))
				std::wcerr << L"Failed to close handle\n";

			*pHandle = 0;
			delete pHandle;
		}

		std::shared_ptr<Win32::HANDLE> m_handle;
	};
}
