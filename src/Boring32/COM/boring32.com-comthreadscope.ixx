export module boring32.com:comthreadscope;
import boring32.error;
import <atomic>;
import <iostream>;
import <win32.hpp>;

export namespace Boring32::COM
{
	/// <summary>
	///		Represents a COM library lifetime scope for a thread.
	/// </summary>
	class COMThreadScope final
	{
		// The Six
		public:
			/// <summary>
			///		Internally calls Uninitialise().
			/// </summary>
			~COMThreadScope()
			{
				Uninitialise();
			}

			/// <summary>
			///		Default constructor. Does not initialise COM.
			/// </summary>
			COMThreadScope() = default;

			/// <summary>
			///		Copy constructor. Copies the ComThreadScope's threading mode
			///		and initialises COM for the current thread if ComThreadScope
			///		is initialised.
			/// </summary>
			/// <param name="other">The ComThreadScope to copy from.</param>
			COMThreadScope(const COMThreadScope& other)
			{
				Copy(other);
			}

			/// <summary>
			///		Copy assignment. Copies the ComThreadScope's threading mode
			///		and initialises COM for the current thread if ComThreadScope
			///		is initialised.
			/// </summary>
			/// <param name="other">The ComThreadScope to copy from.</param>
			void operator=(const COMThreadScope& other)
			{
				Copy(other);
			}
			
			/// <summary>
			///		Move constructor. Assumes the temporary's scope.
			/// </summary>
			COMThreadScope(COMThreadScope&& other) noexcept
			{
				Move(other);
			}

			/// <summary>
			///		Move assignment. Assumes the temporary's scope.
			/// </summary>
			void operator=(COMThreadScope&& other) noexcept
			{
				Move(other);
			}

		// Custom constructors
		public:
			/// <summary>
			///		Initialises COM for the creating thread with the specified
			///		apartment threading mode.
			/// </summary>
			/// <param name="apartmentThreadingMode">The threading mode to initialise COM with.</param>
			COMThreadScope(const COINIT apartmentThreadingMode)
				: m_isInitialised(false),
				m_comInitialisedThreadId(0),
				m_apartmentThreadingMode(apartmentThreadingMode)
			{
				Initialise();
			}

		// API
		public:
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
			void Initialise()
			{
				if (m_isInitialised)
					return;

				// Initialise COM for this thread
				HRESULT hr = CoInitializeEx(nullptr, m_apartmentThreadingMode);
				if (FAILED(hr))
					throw Error::COMError("CoInitializeEx() failed", hr);

				m_isInitialised = true;
				m_comInitialisedThreadId = GetCurrentThreadId();
			}

			/// <summary>
			///		Initialises general COM security levels. This can only
			///		be called once.
			/// </summary>
			void InitialiseSecurity()
			{
				m_isSecurityInitialised++;
				if (m_isSecurityInitialised != 1)
				{
					std::wcerr
						<< L"An attempt to initialise COM security more than once for this process occurred. "
						<< L"COM security can only be set once for the whole process, and cannot be changed. "
						<< L"Ignoring..."
						<< std::endl;
					return;
				}

				// Set general COM security levels. This can only be set once per process.
				// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializesecurity
				HRESULT hr = CoInitializeSecurity(
					nullptr,
					-1,								// COM authentication
					nullptr,                        // Authentication services
					nullptr,                        // Reserved
					RPC_C_AUTHN_LEVEL_DEFAULT,		// Default authentication 
					RPC_C_IMP_LEVEL_IMPERSONATE,	// Default Impersonation  
					nullptr,                        // Authentication info
					EOAC_NONE,						// Additional capabilities 
					nullptr                         // Reserved
				);
				if (FAILED(hr))
					throw Error::COMError("CoInitializeSecurity() failed", hr);
			}

			/// <summary>
			///		Uninitialises COM. If this object does not currently have
			///		an active COM scope, this function has no effect.
			/// </summary>
			/// <exception cref="std::runtime_error">
			///		Thrown if this function is called from a thread different
			///		to the thread that initialised this object.
			/// </exception>
			void Uninitialise()
			{
				if (m_isInitialised == false)
					return;

				if (m_comInitialisedThreadId != GetCurrentThreadId())
					throw Error::Boring32Error("Attempt to uninitialise COM by a thread different to initialising one.");

				CoUninitialize();
				m_isInitialised = false;
			}

			/// <summary>
			///		Returns whether this COM scope object has been initialised.
			/// </summary>
			/// <returns>A bool indicating whether this scope is active.</returns>
			bool IsInitialised() const noexcept
			{
				return m_isInitialised;
			}

			/// <summary>
			///		Returns the thread ID that initialised this COM object scope.
			/// </summary>
			/// <returns>The thread ID that initialsed the COM scope.</returns>
			DWORD GetComInitialisedThreadId() const noexcept
			{
				return m_comInitialisedThreadId;
			}

			/// <summary>
			///		Returns the current COM threading apartment mode held by
			///		this object.
			/// </summary>
			/// <returns>The current COM threading apartment mode </returns>
			COINIT GetApartmentThreadingMode() const noexcept
			{
				return m_apartmentThreadingMode;
			}

		// Internal
		private:
			void Copy(const COMThreadScope& other)
			{
				Uninitialise();
				m_comInitialisedThreadId = 0;
				m_isInitialised = false;
				m_apartmentThreadingMode = other.m_apartmentThreadingMode;
				if (other.m_isInitialised)
					Initialise();
			}

			void Move(COMThreadScope& other)
			{
				Uninitialise();
				m_comInitialisedThreadId = other.m_comInitialisedThreadId;
				m_isInitialised = other.m_isInitialised;
				m_apartmentThreadingMode = other.m_apartmentThreadingMode;
				// Assume the scope of the temporary
				if (other.m_isInitialised)
					other.m_isInitialised = false;
			}

		private:
			bool m_isInitialised = false;
			static std::atomic<unsigned> m_isSecurityInitialised;
			DWORD m_comInitialisedThreadId = 0;
			COINIT m_apartmentThreadingMode = COINIT_MULTITHREADED;
	};

	std::atomic<unsigned> COMThreadScope::m_isSecurityInitialised(0);
}