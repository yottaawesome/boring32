module;

#include <string>
#include <Windows.h>

export module boring32.guid;

export namespace Boring32::Guid
{
	std::wstring GetGuidAsWString(const GUID& guid);
	std::wstring GetGuidAsWString();
}
