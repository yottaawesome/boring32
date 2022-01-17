module;

#include <iostream>
#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include <objbase.h>

module boring32.com.comthreadscope;
import boring32.error.comerror;

namespace Boring32::Com
{
	std::atomic<unsigned> ComThreadScope::m_isSecurityInitialised(0);

	ComThreadScope::~ComThreadScope()
	{
		Uninitialise();
	}

	ComThreadScope::ComThreadScope()
	:	m_isInitialised(false),
		m_comInitialisedThreadId(0),
		m_apartmentThreadingMode(COINIT_MULTITHREADED)
	{}

	ComThreadScope::ComThreadScope(const COINIT apartmentThreadingMode)
	:	m_isInitialised(false),
		m_comInitialisedThreadId(0),
		m_apartmentThreadingMode(apartmentThreadingMode)
	{
		Initialise();
	}

	ComThreadScope::ComThreadScope(const ComThreadScope& other)
	:	m_isInitialised(false),
		m_comInitialisedThreadId(0),
		m_apartmentThreadingMode(COINIT_MULTITHREADED)
	{
		Copy(other);
	}

	void ComThreadScope::operator=(const ComThreadScope& other)
	{
		Copy(other);
	}

	void ComThreadScope::Copy(const ComThreadScope& other)
	{
		Uninitialise();
		m_comInitialisedThreadId = 0;
		m_isInitialised = false;
		m_apartmentThreadingMode = other.m_apartmentThreadingMode;
		if (other.m_isInitialised)
			Initialise();
	}

	ComThreadScope::ComThreadScope(ComThreadScope&& other) noexcept
	{
		Move(other);
	}

	void ComThreadScope::operator=(ComThreadScope&& other) noexcept
	{
		Move(other);
	}

	void ComThreadScope::Move(ComThreadScope& other)
	{
		Uninitialise();
		m_comInitialisedThreadId = other.m_comInitialisedThreadId;
		m_isInitialised = other.m_isInitialised;
		m_apartmentThreadingMode = other.m_apartmentThreadingMode;
		// Assume the scope of the temporary
		if (other.m_isInitialised)
			other.m_isInitialised = false;
	}

	void ComThreadScope::Initialise()
	{
		if (m_isInitialised)
			return;

		// Initialise COM for this thread
		HRESULT hr = CoInitializeEx(nullptr, m_apartmentThreadingMode);
		if (FAILED(hr))
			throw Error::ComError(std::source_location::current(), "CoInitializeEx() failed", hr);

		m_isInitialised = true;
		m_comInitialisedThreadId = GetCurrentThreadId();
	}

	void ComThreadScope::InitialiseSecurity()
	{
		m_isSecurityInitialised++;
		if (m_isSecurityInitialised != 1)
		{
			std::wcerr 
				<< L"__FUNCSIG__: "
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
			throw Error::ComError(std::source_location::current(),"CoInitializeSecurity() failed", hr);
	}

	void ComThreadScope::Uninitialise()
	{
		if (m_isInitialised == false)
			return;

		if (m_comInitialisedThreadId != GetCurrentThreadId())
			throw std::runtime_error(__FUNCSIG__ ": Attempt to uninitialise COM by a thread different to initialising one.");

		CoUninitialize();
		m_isInitialised = false;
	}

	bool ComThreadScope::IsInitialised() const
	{
		return m_isInitialised;
	}

	DWORD ComThreadScope::GetComInitialisedThreadId() const
	{
		return m_comInitialisedThreadId;
	}

	COINIT ComThreadScope::GetApartmentThreadingMode() const
	{
		return m_apartmentThreadingMode;
	}
}