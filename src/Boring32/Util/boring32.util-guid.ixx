module;

#include <string>
#include <Windows.h>

export module boring32.util:guid;

export namespace Boring32::Util
{
	class GloballyUniqueID
	{
		public:
			virtual ~GloballyUniqueID();
			GloballyUniqueID();
			GloballyUniqueID(const GloballyUniqueID& other);
			GloballyUniqueID(GloballyUniqueID&& other) noexcept;

			GloballyUniqueID(const GUID& guid);

		public:
			virtual GloballyUniqueID& operator=(const GloballyUniqueID& other);
			virtual GloballyUniqueID& operator=(GloballyUniqueID&& other) noexcept;

		public:
			virtual void ToString(std::wstring& out);
			virtual void ToString(std::string& out);

		protected:
			GUID m_guid;
	};
}
