module;

#include <Windows.h>

export module boring32.util:guid;

export namespace Boring32::Util
{
	class GloballyUniqueID
	{
		public:
			virtual ~GloballyUniqueID();
			GloballyUniqueID();
			GloballyUniqueID(const GUID& guid);

		protected:
			GUID m_guid;
	};
}
