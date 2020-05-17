#pragma once
#include <string>
#include <Windows.h>
#include "Win32Exception.hpp"

namespace Boring32::Error
{
	std::wstring GetErrorCodeWString(const DWORD errorCode);
}
