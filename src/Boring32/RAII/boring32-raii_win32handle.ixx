export module boring32:raii_win32handle;
import boring32.shared;
import :error;

export namespace Boring32::RAII
{
	struct Win32Handle final
	{
		~Win32Handle()
		{
			Close();
		}

		Win32Handle() : m_handle(CreateHandlePtr(nullptr)) { }
		Win32Handle(const Win32::HANDLE handle) : m_handle(CreateHandlePtr(handle)) { }
		Win32Handle(const Win32Handle& otherHandle) : m_handle(CreateHandlePtr(nullptr))
		{
			Copy(otherHandle);
		}

		Win32Handle(Win32Handle&& otherHandle) noexcept
			: m_handle(CreateHandlePtr(nullptr))
		{
			Move(otherHandle);
		}

		void operator=(const Win32Handle& other)
		{
			Copy(other);
		}
			
		Win32Handle& operator=(Win32Handle&& other) noexcept
		{
			Move(other);
			return *this;
		}

		///	Compares the current handle to the specified handle.
		///	For the purposes of comparison, a nullptr handle is
		///	considered equivalent to INVALID_HANDLE_VALUE.
		bool operator==(const Win32::HANDLE other) const noexcept
		{
			if (not IsValidValue(other))
				return not IsValidValue(m_handle);
			return m_handle && *m_handle == other;
		}

		///	Closes the current handle, if valid, and assumes
		///	ownership of handle in the parameter.
		Win32Handle& operator=(const Win32::HANDLE other)
		{
			Close();
			m_handle = CreateHandlePtr(other);
			return *this;
		}

		///	Compares the current handle to the specified handle.
		///	For the purposes of comparison, a nullptr handle is
		///	considered equivalent to INVALID_HANDLE_VALUE.
		bool operator==(const Win32Handle& other) const
		{
			// https://devblogs.microsoft.com/oldnewthing/20040302-00/?p=40443
			if (not IsValidValue(other.m_handle))
				return not IsValidValue(m_handle);
			return m_handle == other.m_handle;
		}

		///	Tests whether this handle is null or INVALID_HANDLE_VALUE.
		operator bool() const noexcept
		{
			return IsValidValue();
		}

		///	Returns the internal HANDLE address.
		Win32::HANDLE* operator&()
		{
			if (not m_handle)
				m_handle = CreateHandlePtr(nullptr);
			return m_handle.get();
		}

		///	Returns the underlying native handle, which may be null.
		constexpr Win32::HANDLE operator*() const noexcept
		{
			return m_handle ? *m_handle : nullptr;
		}

		///	Cast to HANDLE operator.
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
				? Win32Handle::DuplicatePassedHandle(*m_handle, IsInheritable())
				: nullptr;
		}
			
		bool IsInheritable() const
		{
			if (not m_handle or not *m_handle)
				return false;
			return Win32Handle::HandleIsInheritable(*m_handle);
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
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error("SetHandleInformation() failed", lastError);
			}
		}

		Win32::HANDLE Detach() noexcept
		{
			if (not m_handle or not *m_handle)
				return nullptr;
			Win32::HANDLE temp = *m_handle;
			*m_handle = nullptr;
			return temp;
		}

		///	Returns whether the internal HANDLE's value is not 
		///	nullptr or INVALID_HANDLE_VALUE. Note: this does not mean
		///	the value is an actual current and valid HANDLE.
		bool IsValidValue() const noexcept
		{
			return m_handle && IsValidValue(*m_handle);
		}

		bool IsValidValue(const std::shared_ptr<Win32::HANDLE>& handle) const noexcept
		{
			return handle && IsValidValue(*handle);
		}

		constexpr bool IsValidValue(Win32::HANDLE handle) const noexcept
		{
			return handle && handle != Win32::InvalidHandleValue;
		}

		static bool HandleIsInheritable(const HANDLE handle)
		{
			if (not handle)
				return false;
			if (handle == Win32::InvalidHandleValue)
				return false;

			Win32::DWORD flags = 0;
			if (not Win32::GetHandleInformation(handle, &flags))
			{
				const Win32::DWORD lastError = GetLastError();
				throw Error::Win32Error("GetHandleInformation() failed", lastError);
			}
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
			if (auto lastError = GetLastError(); not succeeded)
				throw Error::Win32Error("DuplicateHandle() failed", lastError);

			return duplicateHandle;
		}

		private:
		void Copy(const Win32Handle& other)
		{
			Close();
			m_handle = other.m_handle;
			/*Win32Handle::DuplicatePassedHandle(
				other.GetHandle(),
				other.IsInheritable()
			);*/
		}

		void Move(Win32Handle& other) noexcept
		{
			Close();
			m_handle = std::move(other.m_handle);
			other.m_handle = nullptr;
		}

		std::shared_ptr<Win32::HANDLE> CreateHandlePtr(Win32::HANDLE handle)
		{
			return { new void* (handle), &Win32Handle::CloseHandleAndFreeMemory };

			// This doesn't work in cases where a Win32Handle gets copied/moved and the 
			// original gets destroyed. This is because the lambda capture of "this"
			// refers to the initial object, and when the shared_ptr goes to destroy 
			// the memory, it will invoke CloseHandleAndFreeMemory bound to the original
			// destroyed instance, even if the current object is live, causing an access
			// violation. As such, CloseHandleAndFreeMemory() has been made static and 
			// the bind() call has been removed as per above.
			/*return {
				new void* (handle),
				std::bind(&Win32Handle::CloseHandleAndFreeMemory, this, std::placeholders::_1)
			};*/
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