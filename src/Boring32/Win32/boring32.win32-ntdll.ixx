export module boring32.win32:ntdll;
import :defs;

export namespace Boring32::Win32
{
	class NTDLL final
	{
		public:
			NTDLL();
			NTDLL(const NTDLL&) = delete;
			NTDLL(NTDLL&&) noexcept = delete;

		public:
			NTDLL& operator=(const NTDLL&) = delete;
			NTDLL& operator=(NTDLL&&) noexcept = delete;

		private:
			void Map();

		private:
			MapViewOfSection m_mapViewOfSection;
			QuerySystemInformation m_querySystemInformation;
			DuplicateObject m_duplicateObject;
			QueryObject m_queryObject;
	};
}
