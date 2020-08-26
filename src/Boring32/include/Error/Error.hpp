#pragma once
#include <string>
#include <Windows.h>
#include "Win32Error.hpp"
#include "ComError.hpp"

namespace Boring32::Error
{
	void GetErrorCodeString(const DWORD errorCode, std::string& stringToHoldMessage);
	void GetErrorCodeString(const DWORD errorCode, std::wstring& stringToHoldMessage);
	std::wstring CreateErrorStringFromCode(const std::wstring msg, const DWORD errorCode);
	std::string CreateErrorStringFromCode(const std::string msg, const DWORD errorCode);
	std::wstring GetErrorFromHResult(const std::wstring& msg, const HRESULT hr);
	std::string GetErrorFromHResult(const std::string& msg, const HRESULT hr);
}
