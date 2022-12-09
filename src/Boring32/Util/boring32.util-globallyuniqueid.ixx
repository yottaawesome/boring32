export module boring32.util:guid;
import <string>;
import <Windows.h>;

export namespace Boring32::Util
{
	class GloballyUniqueID
	{
		public:
			virtual ~GloballyUniqueID();
			GloballyUniqueID();
			GloballyUniqueID(const std::wstring& guidString);
			GloballyUniqueID(const GloballyUniqueID& other);
			GloballyUniqueID(GloballyUniqueID&& other) noexcept;

			GloballyUniqueID(const GUID& guid);

		public:
			virtual GloballyUniqueID& operator=(const GUID& other) noexcept;
			virtual GloballyUniqueID& operator=(const GloballyUniqueID& other);
			virtual GloballyUniqueID& operator=(GloballyUniqueID&& other) noexcept;
			virtual bool operator==(const GloballyUniqueID& other) const noexcept;
			virtual bool operator==(const GUID& other) const noexcept;

		public:
			virtual void ToString(std::wstring& out) const;
			virtual void ToString(std::string& out) const;
			virtual const GUID& Get() const noexcept;

		protected:
			GUID m_guid;
	};
}
