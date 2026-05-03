export module boring32:com.comthreadscope;
import std;
import :win32;
import :error;
import :com.functions;

export namespace Boring32::Com
{
	///	Represents a COM library lifetime scope for a thread.
	struct COMThreadScope final
	{
		///	Internally calls Uninitialise().
		~COMThreadScope()
		{
			Uninitialise();
		}

		///	Default constructor. Does not initialise COM.
		COMThreadScope() = default;

		///	Copy constructor. Copies the ComThreadScope's threading mode
		///	and initialises COM for the current thread if ComThreadScope
		///	is initialised.
		COMThreadScope(const COMThreadScope& other)
		{
			Copy(other);
		}

		///	Copy assignment. Copies the ComThreadScope's threading mode
		///	and initialises COM for the current thread if ComThreadScope
		///	is initialised.
		void operator=(const COMThreadScope& other)
		{
			Copy(other);
		}
			
		///	Move constructor. Assumes the temporary's scope.
		COMThreadScope(COMThreadScope&& other) noexcept
		{
			Move(other);
		}

		///	Move assignment. Assumes the temporary's scope.
		void operator=(COMThreadScope&& other) noexcept
		{
			Move(other);
		}

		///	Initialises COM for the creating thread with the specified
		///	apartment threading mode.
		COMThreadScope(const Win32::COINIT apartmentThreadingMode)
			: m_apartmentThreadingMode(apartmentThreadingMode)
		{
			Initialise();
		}

		///	Initialises COM for the calling thread with the specified
		///	apartment threading mode. If this object has already been
		///	initialised with no corresponding uninitialisation, this
		///	function has no effect.
		///	Thrown if this function fails to initialise COM or set the
		///	COM security level.
		void Initialise()
		{
			if (m_isInitialised)
				return;

			// Initialise COM for this thread.
			Win32::HRESULT hr = Win32::CoInitializeEx(
				nullptr,
				// "In addition to the flags already mentioned, it is a good idea 
				// to set the COINIT_DISABLE_OLE1DDE flag in the dwCoInit parameter. 
				// Setting this flag avoids some overhead associated with Object 
				// Linking and Embedding (OLE) 1.0, an obsolete technology."
				// https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library
				m_apartmentThreadingMode | Win32::COINIT::COINIT_DISABLE_OLE1DDE
			);
			if (Failed(hr))
				throw Error::COMError(hr, "CoInitializeEx() failed");

			m_isInitialised = true;
			m_comInitialisedThreadId = Win32::GetCurrentThreadId();
		}

		///	Initialises general COM security levels. This can only
		///	be called once.
		void InitialiseSecurity()
		{
			m_isSecurityInitialised++;
			if (m_isSecurityInitialised != 1)
			{
				std::wcerr << std::format(L"An attempt to initialise COM security more than once for this process occurred. COM security can only be set once for the whole process, and cannot be changed. Ignoring...\n");
				return;
			}

			// Set general COM security levels. This can only be set once per process.
			// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializesecurity
			HRESULT hr = Win32::CoInitializeSecurity(
				nullptr,
				-1,								// COM authentication
				nullptr,                        // Authentication services
				nullptr,                        // Reserved
				(Win32::DWORD)Win32::RPCCAuthLevel::Default,		// Default authentication 
				(Win32::DWORD)Win32::RPCCImpLevel::Impersonate,	// Default Impersonation  
				nullptr,                        // Authentication info
				Win32::EOLE_AUTHENTICATION_CAPABILITIES::EOAC_NONE,						// Additional capabilities 
				nullptr                         // Reserved
			);
			if (Failed(hr))
				throw Error::COMError(hr, "CoInitializeSecurity() failed");
		}

		///	Uninitialises COM. If this object does not currently have
		///	an active COM scope, this function has no effect.
		///	Thrown if this function is called from a thread different
		///	to the thread that initialised this object.
		void Uninitialise()
		{
			if (m_isInitialised == false)
				return;

			if (m_comInitialisedThreadId != Win32::GetCurrentThreadId())
				throw Error::Boring32Error("Attempt to uninitialise COM by a thread different to initialising one.");

			Win32::CoUninitialize();
			m_isInitialised = false;
		}

		///	Returns whether this COM scope object has been initialised.
		auto IsInitialised() const noexcept -> bool
		{
			return m_isInitialised;
		}

		///	Returns the thread ID that initialised this COM object scope.
		auto GetComInitialisedThreadId() const noexcept -> Win32::DWORD
		{
			return m_comInitialisedThreadId;
		}

		///	Returns the current COM threading apartment mode held by
		///	this object.
		auto GetApartmentThreadingMode() const noexcept -> Win32::COINIT
		{
			return m_apartmentThreadingMode;
		}

	private:
		// Not sure if this is something that makes sense.
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

		bool m_isInitialised = false;
		static std::atomic<unsigned> m_isSecurityInitialised;
		Win32::DWORD m_comInitialisedThreadId = 0;
		// https://learn.microsoft.com/en-us/windows/win32/api/objbase/ne-objbase-coinit
		Win32::COINIT m_apartmentThreadingMode = COINIT_MULTITHREADED;
	};

	std::atomic<unsigned> COMThreadScope::m_isSecurityInitialised(0);
}