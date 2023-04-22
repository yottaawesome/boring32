export module boring32.util:guid;
import <string>;
import <win32.hpp>;

export namespace Boring32::Util
{
	class GloballyUniqueID
	{
		public:
			virtual ~GloballyUniqueID() = default;
			GloballyUniqueID();
			GloballyUniqueID(const std::wstring& guidString);
			GloballyUniqueID(const GloballyUniqueID& other) = default;
			GloballyUniqueID(GloballyUniqueID&& other) noexcept = default;

			GloballyUniqueID(const GUID& guid);

		public:
			virtual GloballyUniqueID& operator=(const GUID& other) noexcept;
			virtual GloballyUniqueID& operator=(const GloballyUniqueID& other) = default;
			virtual GloballyUniqueID& operator=(GloballyUniqueID&& other) noexcept = default;
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
