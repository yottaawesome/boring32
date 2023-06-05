module;

#include <source_location>;

export module boring32.raii:win32handle;
import <functional>;
import <iostream>;
import <stdexcept>;
import <format>;
import <memory>;
import <win32.hpp>;
import boring32.error;

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
			Win32Handle(const HANDLE handle) : m_handle(CreateHandlePtr(handle)) { }
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
			virtual bool operator==(const HANDLE other) const noexcept
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
			virtual Win32Handle& operator=(const HANDLE other)
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
			virtual HANDLE* operator&()
			{
				if (m_handle == nullptr)
					m_handle = CreateHandlePtr(nullptr);
				return m_handle.get();
			}

			/// <summary>
			///		Returns the underlying native handle, which may be null.
			/// </summary>
			/// <returns>The underlying native handle</returns>
			virtual HANDLE operator*() const noexcept
			{
				return m_handle ? *m_handle : nullptr;
			}

			/// <summary>
			///		Cast to HANDLE operator.
			/// </summary>
			/// <returns>The underlying native handle or nullptr</returns>
			virtual operator HANDLE() const noexcept
			{
				return m_handle ? *m_handle : nullptr;
			}

			// API
		public:
			virtual HANDLE GetHandle() const noexcept
			{
				return m_handle ? *m_handle : nullptr;
			}

			virtual HANDLE DuplicateCurrentHandle() const
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
				if (!SetHandleInformation(*m_handle, HANDLE_FLAG_INHERIT, isInheritable))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("SetHandleInformation() failed", lastError);
				}
			}

			virtual HANDLE Detach() noexcept
			{
				if (!m_handle || !*m_handle)
					return nullptr;
				HANDLE temp = *m_handle;
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

			virtual bool IsValidValue(const std::shared_ptr<HANDLE>& handle) const noexcept
			{
				return handle && IsValidValue(*handle);
			}

			virtual bool IsValidValue(HANDLE handle) const noexcept
			{
				return handle && handle != INVALID_HANDLE_VALUE;
			}

		public:
			static bool HandleIsInheritable(const HANDLE handle)
			{
				if (!handle)
					return false;
				if (handle == INVALID_HANDLE_VALUE)
					return false;

				DWORD flags = 0;
				if (!GetHandleInformation(handle, &flags))
				{
					const DWORD lastError = GetLastError();
					throw Error::Win32Error("GetHandleInformation() failed", lastError);
				}
				return flags & HANDLE_FLAG_INHERIT;
			}

			static HANDLE DuplicatePassedHandle(const HANDLE handle, const bool isInheritable)
			{
				if (handle == nullptr)
					return nullptr;
				if (handle == INVALID_HANDLE_VALUE)
					return INVALID_HANDLE_VALUE;

				HANDLE duplicateHandle = nullptr;
				const bool succeeded = DuplicateHandle(
					GetCurrentProcess(),
					handle,
					GetCurrentProcess(),
					&duplicateHandle,
					0,
					isInheritable,
					DUPLICATE_SAME_ACCESS
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

			virtual std::shared_ptr<HANDLE> CreateHandlePtr(HANDLE handle)
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

			static void CloseHandleAndFreeMemory(HANDLE* pHandle)
			{
				if (!pHandle)
					return;

				HANDLE wrappedHandle = *pHandle;
				if (wrappedHandle && wrappedHandle != INVALID_HANDLE_VALUE)
					if (!CloseHandle(wrappedHandle))
						std::wcerr << L"Failed to close handle\n";

				*pHandle = 0;
				delete pHandle;
			}

		protected:
			std::shared_ptr<HANDLE> m_handle;
	};
}