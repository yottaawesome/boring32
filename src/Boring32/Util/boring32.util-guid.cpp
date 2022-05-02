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
	GloballyUniqueID::GloballyUniqueID(const GUID& guid)
		: m_guid(guid)
	{}
}
