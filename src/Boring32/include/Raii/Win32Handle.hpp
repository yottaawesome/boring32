#pragma once
#include <Windows.h>
#include <string>
#include <memory>

namespace Boring32::Raii
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
			virtual void operator=(const Win32Handle& other);
			virtual Win32Handle& operator=(Win32Handle&& other) noexcept;
			/// <summary>
			///		Compares the current handle to the specified handle.
			///		For the purposes of comparison, a nullptr handle is
			///		considered equivalent to INVALID_HANDLE_VALUE.
			/// </summary>
			/// <param name="other">The handle to compare against.</param>
			/// <returns>Whether the handles are equivalent.</returns>
			virtual bool operator==(const HANDLE other) const;

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

			virtual operator bool() const;

			/// <summary>
			///		Returns the internal HANDLE address.
			/// </summary>
			/// <returns>The internal HANDLE's address.</returns>
			virtual HANDLE* operator&();

			virtual HANDLE operator*() const;
		
		// API
		public:
			virtual HANDLE GetHandle() const;
			virtual HANDLE DuplicateCurrentHandle() const;
			virtual bool IsInheritable() const;
			virtual HANDLE& GetHandleAddress();
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
			virtual bool IsValidValue() const;

		public:
			static bool HandleIsInheritable(const HANDLE handle);
			static HANDLE DuplicatePassedHandle(const HANDLE handle, const bool isInheritable);

		protected:
			virtual void Copy(const Win32Handle& other);
			virtual void Move(Win32Handle& other) noexcept;

		protected:
			HANDLE m_handle;
	};
}