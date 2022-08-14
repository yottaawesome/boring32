module;

#include <Windows.h>
#include <memory>

export module boring32.raii:win32handle;

export namespace Boring32::RAII
{
	class Win32Handle
	{
		// Constructors
		public:
			virtual ~Win32Handle();
			Win32Handle();
			Win32Handle(const HANDLE handle);
			Win32Handle(const Win32Handle& otherHandle);
			Win32Handle(Win32Handle&& otherHandle) noexcept;

			// Operators
		public:
			/// <summary>
			/// 
			/// </summary>
			/// <param name="other"></param>
			virtual void operator=(const Win32Handle& other);
			
			virtual Win32Handle& operator=(Win32Handle&& other) noexcept;
			/// <summary>
			///		Compares the current handle to the specified handle.
			///		For the purposes of comparison, a nullptr handle is
			///		considered equivalent to INVALID_HANDLE_VALUE.
			/// </summary>
			/// <param name="other">The handle to compare against.</param>
			/// <returns>Whether the handles are equivalent.</returns>
			virtual bool operator==(const HANDLE other) const noexcept;

			/// <summary>
			///		Closes the current handle, if valid, and assumes
			///		ownership of handle in the parameter.
			/// </summary>
			/// <param name="other">The handle to assume ownership of. This paremeter can be nullptr.</param>
			virtual Win32Handle& operator=(const HANDLE other);

			/// <summary>
			///		Compares the current handle to the specified handle.
			///		For the purposes of comparison, a nullptr handle is
			///		considered equivalent to INVALID_HANDLE_VALUE.
			/// </summary>
			/// <param name="other">The handle to compare against.</param>
			/// <returns>Whether the handles are equivalent.</returns>
			virtual bool operator==(const Win32Handle& other) const;

			/// <summary>
			///		Tests whether this handle is null or INVALID_HANDLE_VALUE.
			/// </summary>
			/// <returns>True if the handle is null or INVALID_HANDLE_VALUE, false otherwise.</returns>
			virtual operator bool() const noexcept;

			/// <summary>
			///		Returns the internal HANDLE address.
			/// </summary>
			/// <returns>The internal HANDLE's address.</returns>
			virtual HANDLE* operator&();

			/// <summary>
			///		Returns the underlying native handle, which may be null.
			/// </summary>
			/// <returns>The underlying native handle</returns>
			virtual HANDLE operator*() const noexcept;

			/// <summary>
			///		Cast to HANDLE operator.
			/// </summary>
			/// <returns>The underlying native handle or nullptr</returns>
			virtual operator HANDLE() const noexcept;

			// API
		public:
			virtual HANDLE GetHandle() const noexcept;
			virtual HANDLE DuplicateCurrentHandle() const;
			virtual bool IsInheritable() const;
			virtual void Close() noexcept;
			virtual void SetInheritability(const bool isInheritable);
			virtual HANDLE Detach() noexcept;

			/// <summary>
			///		Returns whether the internal HANDLE's value is not 
			///		nullptr or INVALID_HANDLE_VALUE. Note: this does not mean
			///		the value is an actual current and valid HANDLE.
			/// </summary>
			/// <returns>
			///		True if the internal HANDLE is not nullptr and 
			///		INVALID_HANDLE_VALUE, false otherwise.
			/// </returns>
			virtual bool IsValidValue() const noexcept;
			virtual bool IsValidValue(const std::shared_ptr<HANDLE>& handle) const noexcept;
			virtual bool IsValidValue(HANDLE handle) const noexcept;

		public:
			static bool HandleIsInheritable(const HANDLE handle);
			static HANDLE DuplicatePassedHandle(const HANDLE handle, const bool isInheritable);

		protected:
			virtual void Copy(const Win32Handle& other);
			virtual void Move(Win32Handle& other) noexcept;
			virtual std::shared_ptr<HANDLE> CreateHandlePtr(HANDLE handle);
			static void CloseHandleAndFreeMemory(HANDLE* pHandle);

		protected:
			std::shared_ptr<HANDLE> m_handle;
	};
}