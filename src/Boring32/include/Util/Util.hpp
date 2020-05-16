#pragma once
#include <Windows.h>

namespace Boring32::Util
{
	HANDLE DuplicatePassedHandle(const HANDLE handle, const bool isInheritable);
}
