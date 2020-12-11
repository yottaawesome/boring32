#include "pch.hpp"
#include <stdexcept>
#include <iostream>
#include "include/Error/Error.hpp"
#include "include/Crypto/CertStore.hpp"

namespace Boring32::Crypto
{
	CertStore::~CertStore()
	{
		Close(std::nothrow);
	}
	
	CertStore::CertStore()
	:	m_certStore(nullptr),
		m_closeOptions(CertStoreCloseOptions::Default)
	{ }

	CertStore::CertStore(std::wstring storeName)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(CertStoreCloseOptions::Default)
	{
		InternalOpen();
	}
	
	CertStore::CertStore(
		std::wstring storeName,
		const CertStoreCloseOptions closeOptions
	)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(closeOptions)
	{
		InternalOpen();
	}

	void CertStore::Close()
	{
		if (m_certStore)
		{
			// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certclosestore
			// for additional resource notes under remarks
			if (CertCloseStore(m_certStore, (DWORD)m_closeOptions) == false)
				throw Error::Win32Error(__FUNCSIG__ ": CertCloseStore() failed", GetLastError());
			m_certStore = nullptr;
		}
	}

	void CertStore::Close(const std::nothrow_t&) noexcept
	{
		Error::TryCatchLogToWCerr([this] { Close(); }, __FUNCSIG__);
	}

	void CertStore::InternalOpen()
	{
		if (m_storeName.empty())
			throw std::invalid_argument(__FUNCSIG__ ": m_storeName is empty");
		// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certopensystemstorew
		// common store names: CA, MY, ROOT, SPC
		m_certStore = CertOpenSystemStoreW(0, m_storeName.c_str());
		if (m_certStore == nullptr)
			throw Boring32::Error::Win32Error(__FUNCSIG__ ": CertOpenSystemStoreW() failed", GetLastError());
	}

	HCERTSTORE CertStore::GetHandle() const noexcept
	{
		return m_certStore;
	}

	const std::wstring& CertStore::GetName() const noexcept
	{
		return m_storeName;
	}
}
