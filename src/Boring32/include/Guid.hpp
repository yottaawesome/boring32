#pragma once
#include <string>
#include <Windows.h>

namespace Boring32::Guid
{
	std::wstring GetGuidAsWString(const GUID& guid);
}
