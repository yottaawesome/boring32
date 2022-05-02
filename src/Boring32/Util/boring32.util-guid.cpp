module;

#include <Windows.h>

module boring32.util:guid;
import :functions;

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
}
