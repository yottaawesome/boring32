#pragma once
#include <objbase.h>

namespace Boring32::Com
{
	/// <summary>
	///		Represents a COM library lifetime scope for a thread.
	/// </summary>
	class ComThreadScope
	{
		public:
			/// <summary>
			///		Internally calls Uninitialise().
			/// </summary>
			virtual ~ComThreadScope();

			/// <summary>
			///		Default constructor. Does not initialise COM.
			/// </summary>
			ComThreadScope();
			
			/// <summary>
			///		Initialises COM for the creating thread with the specified
			///		apartment threading mode.
			/// </summary>
			/// <param name="apartmentThreadingMode">The threading mode to initialise COM with.</param>
			ComThreadScope(const COINIT apartmentThreadingMode);
			
			/// <summary>
			///		Copy constructor. Copies the ComThreadScope's threading mode
			///		and initialises COM for the current thread if ComThreadScope
			///		is initialised.
			/// </summary>
			/// <param name="other">The ComThreadScope to copy from.</param>
			ComThreadScope(const ComThreadScope& other);
			/// <summary>
			///		Copy assignment. Copies the ComThreadScope's threading mode
			///		and initialises COM for the current thread if ComThreadScope
			///		is initialised.
			/// </summary>
			/// <param name="other">The ComThreadScope to copy from.</param>
			virtual void operator=(const ComThreadScope& other);

			/// <summary>
			///		Move constructor. Assumes the temporary's scope.
			/// </summary>
			ComThreadScope(ComThreadScope&& other) noexcept;
			/// <summary>
			///		Move assignment. Assumes the temporary's scope.
			/// </summary>
			virtual void operator=(ComThreadScope&& other) noexcept;
			
			/// <summary>
			///		Initialises COM for the calling thread with the specified
			///		apartment threading mode. If this object has already been
			///		initialised with no corresponding uninitialisation, this
			///		function has no effect.
			/// </summary>
			/// <exception cref="std::runtime_error">
			///		Thrown if this function fails to initialise COM or set the
			///		COM security level.
			/// </exception>
			virtual void Initialise();

			/// <summary>
			///		Uninitialises COM. If this object does not currently have
			///		an active COM scope, this function has no effect.
			/// </summary>
			/// <exception cref="std::runtime_error">
			///		Thrown if this function is called from a thread different
			///		to the thread that initialised this object.
			/// </exception>
			virtual void Uninitialise();

			/// <summary>
			///		Returns whether this COM scope object has been initialised.
			/// </summary>
			/// <returns>A bool indicating whether this scope is active.</returns>
			virtual bool IsInitialised() const;

			/// <summary>
			///		Returns the thread ID that initialised this COM object scope.
			/// </summary>
			/// <returns>The thread ID that initialsed the COM scope.</returns>
			virtual DWORD GetComInitialisedThreadId() const;

			/// <summary>
			///		Returns the current COM threading apartment mode held by
			///		this object.
			/// </summary>
			/// <returns>The current COM threading apartment mode </returns>
			virtual COINIT GetApartmentThreadingMode() const;

		protected:
			void Copy(const ComThreadScope& other);
			void Move(ComThreadScope& other);

		protected:
			bool m_isInitialised;
			DWORD m_comInitialisedThreadId;
			COINIT m_apartmentThreadingMode;
	};
}