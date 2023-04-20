export module boring32.com:comthreadscope;
import <atomic>;
import <win32.hpp>;

export namespace Boring32::COM
{
	/// <summary>
	///		Represents a COM library lifetime scope for a thread.
	/// </summary>
	class COMThreadScope
	{
		public:
			/// <summary>
			///		Internally calls Uninitialise().
			/// </summary>
			virtual ~COMThreadScope();

			/// <summary>
			///		Default constructor. Does not initialise COM.
			/// </summary>
			COMThreadScope() = default;
			
			/// <summary>
			///		Initialises COM for the creating thread with the specified
			///		apartment threading mode.
			/// </summary>
			/// <param name="apartmentThreadingMode">The threading mode to initialise COM with.</param>
			COMThreadScope(const COINIT apartmentThreadingMode);
			
			/// <summary>
			///		Copy constructor. Copies the ComThreadScope's threading mode
			///		and initialises COM for the current thread if ComThreadScope
			///		is initialised.
			/// </summary>
			/// <param name="other">The ComThreadScope to copy from.</param>
			COMThreadScope(const COMThreadScope& other);
			/// <summary>
			///		Copy assignment. Copies the ComThreadScope's threading mode
			///		and initialises COM for the current thread if ComThreadScope
			///		is initialised.
			/// </summary>
			/// <param name="other">The ComThreadScope to copy from.</param>
			virtual void operator=(const COMThreadScope& other);

			/// <summary>
			///		Move constructor. Assumes the temporary's scope.
			/// </summary>
			COMThreadScope(COMThreadScope&& other) noexcept;
			/// <summary>
			///		Move assignment. Assumes the temporary's scope.
			/// </summary>
			virtual void operator=(COMThreadScope&& other) noexcept;
			
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
			///		Initialises general COM security levels. This can only
			///		be called once.
			/// </summary>
			virtual void InitialiseSecurity();

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
			void Copy(const COMThreadScope& other);
			void Move(COMThreadScope& other);

		protected:
			bool m_isInitialised = false;
			static std::atomic<unsigned> m_isSecurityInitialised;
			DWORD m_comInitialisedThreadId = 0;
			COINIT m_apartmentThreadingMode = COINIT_MULTITHREADED;
	};
}