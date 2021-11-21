#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <utility>
#include "NtStatusError.hpp"
#include "ComError.hpp"

namespace Boring32::Error
{
	std::stringstream& PrintExceptionToStringStream(
		const std::exception& ex,
		std::stringstream& ss
	);

	std::wstringstream& PrintExceptionToStringStream(
		const std::exception& ex,
		std::wstringstream& ss
	);

	std::string GetNtStatusError(
		const std::string& 
		msg, const NTSTATUS errorCode
	);

	void GetErrorCodeString(
		const DWORD errorCode, 
		HMODULE moduleToReadFrom, 
		std::wstring& stringToHoldMessage
	) noexcept;
	
	void GetErrorCodeString(
		const DWORD errorCode, 
		HMODULE moduleToReadFrom, 
		std::string& stringToHoldMessage
	) noexcept;

	void GetErrorCodeString(
		const DWORD errorCode, 
		std::string& stringToHoldMessage
	) noexcept;

	void GetErrorCodeString(
		const DWORD errorCode, 
		std::wstring& stringToHoldMessage
	) noexcept;

	std::wstring CreateErrorStringFromCode(
		const std::wstring msg, 
		HMODULE moduleToReadFrom, 
		const DWORD errorCode
	) noexcept;

	std::wstring CreateErrorStringFromNtStatus(
		const std::wstring msg,
		const DWORD errorCode
	) noexcept;

	std::string CreateErrorStringFromNtStatus(
		const std::string msg,
		const DWORD errorCode
	) noexcept;

	std::wstring CreateErrorStringFromCode(
		const std::wstring msg, 
		const DWORD errorCode
	) noexcept;

	std::string CreateErrorStringFromCode(
		const std::string msg, 
		HMODULE moduleToReadFrom, 
		const DWORD errorCode
	) noexcept;

	std::string CreateErrorStringFromCode(
		const std::string msg, 
		const DWORD errorCode
	) noexcept;

	std::wstring GetErrorFromHResult(
		const std::wstring& msg, 
		const HRESULT hr
	) noexcept;

	std::string GetErrorFromHResult(
		const std::string& msg, 
		const HRESULT hr
	) noexcept;

	void TranslateErrorCode(const DWORD errorCode, std::wstring& out) noexcept;
	void TranslateErrorCode(const HMODULE moduleToReadFrom, const DWORD errorCode, std::wstring& out) noexcept;
	void TranslateErrorCode(const DWORD errorCode, std::string& out) noexcept;
	void TranslateErrorCode(const HMODULE moduleToReadFrom, const DWORD errorCode, std::string& out) noexcept;
}
