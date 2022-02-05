module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include "Objbase.h"

module boring32.guid;
import boring32.error.comerror;

namespace Boring32::Guid
{
	// Adapted from https://stackoverflow.com/a/19941516/7448661
	std::wstring GetGuidAsWString(const GUID& guid)
	{
		std::wstring rawGuid(64, '\0');
		int numberOfChars = StringFromGUID2(guid, &rawGuid[0], 64);
		if (numberOfChars == 0)
			throw std::runtime_error(__FUNCSIG__": StringFromGUID2() failed");
		rawGuid.resize(numberOfChars-1); // remove null terminator
		return rawGuid;
	}

	std::wstring GetGuidAsWString()
	{
		GUID guidReference;
		HRESULT result = CoCreateGuid(&guidReference);
		if (FAILED(result))
			throw Error::ComError(std::source_location::current(), "CoCreateGuid() failed", result);
		return GetGuidAsWString(guidReference);
	}
}
