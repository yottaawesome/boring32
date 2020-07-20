#pragma once
#include <string>
#include <Windows.h>
#include "Win32Exception.hpp"

namespace Boring32::Error
{
	void GetErrorCodeString(const DWORD errorCode, std::string& stringToHoldMessage);
	void GetErrorCodeString(const DWORD errorCode, std::wstring& stringToHoldMessage);
	std::wstring CreateErrorStringFromCode(const std::wstring msg, const DWORD errorCode);
	std::string CreateErrorStringFromCode(const std::string msg, const DWORD errorCode);
}
