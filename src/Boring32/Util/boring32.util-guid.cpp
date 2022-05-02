module;

#include <Windows.h>

module boring32.util:guid;
import :functions;
import boring32.strings;

namespace Boring32::Util
{
	GloballyUniqueID::~GloballyUniqueID() {}
	GloballyUniqueID::GloballyUniqueID() 
		: m_guid(GenerateGUID())
	{}

	GloballyUniqueID::GloballyUniqueID(const GloballyUniqueID& other) = default;
	GloballyUniqueID::GloballyUniqueID(GloballyUniqueID&& other) noexcept = default;
	GloballyUniqueID& GloballyUniqueID::operator=(const GloballyUniqueID& other) = default;
	GloballyUniqueID& GloballyUniqueID::operator=(GloballyUniqueID&& other) noexcept = default;

	GloballyUniqueID::GloballyUniqueID(const GUID& guid)
		: m_guid(guid)
	{}

	void GloballyUniqueID::ToString(std::wstring& out)
	{
		out = GetGuidAsWString(m_guid);
	}

	void GloballyUniqueID::ToString(std::string& out)
	{
		out = Strings::ConvertString(GetGuidAsWString(m_guid));
	}
}
