export module boring32.raii:win32handle;
import <functional>;
import <iostream>;
import <format>;
import <memory>;
import boring32.error;
import boring32.win32;

export namespace Boring32::RAII
{
	class Win32Handle
	{
		// Constructors
		public:
			virtual ~Win32Handle()
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

			// Operators
		public:
			/// <summary>
			/// 
			/// </summary>
			/// <param name="other"></param>
			virtual void operator=(const Win32Handle& other)
			{
				Copy(other);
			}
			
			virtual Win32Handle& operator=(Win32Handle&& other) noexcept
			{
				Move(other);
				return *this;
			}

			/// <summary>
			///		Compares the current handle to the specified handle.
			///		For the purposes of comparison, a nullptr handle is
			///		considered equivalent to INVALID_HANDLE_VALUE.
			/// </summary>
			/// <param name="other">The handle to compare against.</param>
			/// <returns>Whether the handles are equivalent.</returns>
			virtual bool operator==(const Win32::HANDLE other) const noexcept
			{
				if (!IsValidValue(other))
					return !IsValidValue(m_handle);
				return m_handle && *m_handle == other;
			}

			/// <summary>
			///		Closes the current handle, if valid, and assumes
			///		ownership of handle in the parameter.
			/// </summary>
			/// <param name="other">The handle to assume ownership of. This paremeter can be nullptr.</param>
			virtual Win32Handle& operator=(const Win32::HANDLE other)
			{
				Close();
				m_handle = CreateHandlePtr(other);
				return *this;
			}

			/// <summary>
			///		Compares the current handle to the specified handle.
			///		For the purposes of comparison, a nullptr handle is
			///		considered equivalent to INVALID_HANDLE_VALUE.
			/// </summary>
			/// <param name="other">The handle to compare against.</param>
			/// <returns>Whether the handles are equivalent.</returns>
			virtual bool operator==(const Win32Handle& other) const
			{
				// https://devblogs.microsoft.com/oldnewthing/20040302-00/?p=40443
				if (!IsValidValue(other.m_handle))
					return !IsValidValue(m_handle);
				return m_handle == other.m_handle;
			}

			/// <summary>
			///		Tests whether this handle is null or INVALID_HANDLE_VALUE.
			/// </summary>
			/// <returns>True if the handle is null or INVALID_HANDLE_VALUE, false otherwise.</returns>
			virtual operator bool() const noexcept
			{
				return IsValidValue();
			}

			/// <summary>
			///		Returns the internal HANDLE address.
			/// </summary>
			/// <returns>The internal HANDLE's address.</returns>
			virtual Win32::HANDLE* operator&()
			{
				if (m_handle == nullptr)
					m_handle = CreateHandlePtr(nullptr);
				return m_handle.get();
			}

			/// <summary>
			///		Returns the underlying native handle, which may be null.
			/// </summary>
			/// <returns>The underlying native handle</returns>
			virtual Win32::HANDLE operator*() const noexcept
			{
				return m_handle ? *m_handle : nullptr;
			}

			/// <summary>
			///		Cast to HANDLE operator.
			/// </summary>
			/// <returns>The underlying native handle or nullptr</returns>
			virtual operator Win32::HANDLE() const noexcept
			{
				return m_handle ? *m_handle : nullptr;
			}

			// API
		public:
			virtual Win32::HANDLE GetHandle() const noexcept
			{
				return m_handle ? *m_handle : nullptr;
			}

			virtual Win32::HANDLE DuplicateCurrentHandle() const
			{
				return IsValidValue(m_handle)
					? Win32Handle::DuplicatePassedHandle(*m_handle, IsInheritable())
					: nullptr;
			}
			
			virtual bool IsInheritable() const
			{
				if (!m_handle || *m_handle == nullptr)
					return false;
				return Win32Handle::HandleIsInheritable(*m_handle);
			}

			virtual void Close() noexcept
			{
				m_handle = nullptr;
			}

			virtual void SetInheritability(const bool isInheritable)
			{
				if (!IsValidValue())
					throw Error::Boring32Error("handle is null or invalid.");
				if (!Win32::SetHandleInformation(*m_handle, Win32::HandleFlagInherit, isInheritable))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("SetHandleInformation() failed", lastError);
				}
			}

			virtual Win32::HANDLE Detach() noexcept
			{
				if (!m_handle || !*m_handle)
					return nullptr;
				Win32::HANDLE temp = *m_handle;
				*m_handle = nullptr;
				return temp;
			}

			/// <summary>
			///		Returns whether the internal HANDLE's value is not 
			///		nullptr or INVALID_HANDLE_VALUE. Note: this does not mean
			///		the value is an actual current and valid HANDLE.
			/// </summary>
			/// <returns>
			///		True if the internal HANDLE is not nullptr and 
			///		INVALID_HANDLE_VALUE, false otherwise.
			/// </returns>
			virtual bool IsValidValue() const noexcept
			{
				return m_handle && IsValidValue(*m_handle);
			}

			virtual bool IsValidValue(const std::shared_ptr<Win32::HANDLE>& handle) const noexcept
			{
				return handle && IsValidValue(*handle);
			}

			virtual bool IsValidValue(Win32::HANDLE handle) const noexcept
			{
				return handle && handle != Win32::InvalidHandleValue;
			}

		public:
			static bool HandleIsInheritable(const HANDLE handle)
			{
				if (!handle)
					return false;
				if (handle == Win32::InvalidHandleValue)
					return false;

				Win32::DWORD flags = 0;
				if (!Win32::GetHandleInformation(handle, &flags))
				{
					const Win32::DWORD lastError = GetLastError();
					throw Error::Win32Error("GetHandleInformation() failed", lastError);
				}
				return flags & Win32::HandleFlagInherit;
			}

			static Win32::HANDLE DuplicatePassedHandle(const Win32::HANDLE handle, const bool isInheritable)
			{
				if (handle == nullptr)
					return nullptr;
				if (handle == Win32::InvalidHandleValue)
					return Win32::InvalidHandleValue;

				Win32::HANDLE duplicateHandle = nullptr;
				const bool succeeded = Win32::DuplicateHandle(
					Win32::GetCurrentProcess(),
					handle,
					Win32::GetCurrentProcess(),
					&duplicateHandle,
					0,
					isInheritable,
					Win32::DuplicateSameAccess
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("DuplicateHandle() failed", lastError);
				}

				return duplicateHandle;
			}

		protected:
			virtual void Copy(const Win32Handle& other)
			{
				Close();
				m_handle = other.m_handle;
				/*Win32Handle::DuplicatePassedHandle(
					other.GetHandle(),
					other.IsInheritable()
				);*/
			}

			virtual void Move(Win32Handle& other) noexcept
			{
				Close();
				m_handle = std::move(other.m_handle);
				other.m_handle = nullptr;
			}

			virtual std::shared_ptr<Win32::HANDLE> CreateHandlePtr(Win32::HANDLE handle)
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
				if (!pHandle)
					return;

				Win32::HANDLE wrappedHandle = *pHandle;
				if (wrappedHandle && wrappedHandle != Win32::InvalidHandleValue)
					if (!Win32::CloseHandle(wrappedHandle))
						std::wcerr << L"Failed to close handle\n";

				*pHandle = 0;
				delete pHandle;
			}

		protected:
			std::shared_ptr<Win32::HANDLE> m_handle;
	};
}