#include "pch.hpp"
#include "Objbase.h"
#include "include/Error/Error.hpp"
#include "include/Guid/Guid.hpp"

namespace Boring32::Guid
{
	// Adapted from https://stackoverflow.com/a/19941516/7448661
	std::wstring GetGuidAsWString(const GUID& guid)
	{
		wchar_t rawGuid[64] = { 0 };
		HRESULT result = StringFromGUID2(guid, rawGuid, 64);
		if (StringFromGUID2(guid, rawGuid, 64) == 0)
			throw std::runtime_error("GetGuidAsWString(): StringFromGUID2() failed");
		return rawGuid;
	}

	std::wstring GetGuidAsWString()
	{
		GUID guidReference;
		HRESULT result = CoCreateGuid(&guidReference);
		if (FAILED(result))
			throw Error::ComError("GetGuidAsWString(): CoCreateGuid() failed", result);
		return GetGuidAsWString(guidReference);
	}
}
