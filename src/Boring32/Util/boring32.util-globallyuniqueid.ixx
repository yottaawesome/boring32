export module boring32.util:guid;
import <string>;
import boring32.win32;
import boring32.error;
import boring32.strings;
import :functions;

export namespace Boring32::Util
{
	class GloballyUniqueID final
	{
		public:
			// GUID strings which aren't wrapped by {}
			struct BasicGuidString { std::wstring GuidString; };
			// GUID strings which are wrapped by {}
			struct WrappedGuidString { std::wstring GuidString; };

		// The Six
		public:
			~GloballyUniqueID() = default;

			GloballyUniqueID()
				: m_guid(GenerateGUID())
			{}

			GloballyUniqueID(const GloballyUniqueID&) = default;

			GloballyUniqueID& operator=(const GloballyUniqueID&) = default;

			GloballyUniqueID(GloballyUniqueID&&) noexcept = default;

			GloballyUniqueID& operator=(GloballyUniqueID&&) noexcept = default;
			
		public:
			explicit GloballyUniqueID(const WrappedGuidString& guidString)
			{
				if (guidString.GuidString.empty())
					throw Error::Boring32Error("GUID cannot be an empty string");
				// Accepts {} around the GUID
				// See https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iidfromstring
				const Win32::HRESULT hr = Win32::IIDFromString(
					guidString.GuidString.c_str(), 
					&m_guid
				);
				if (hr != Win32::S_Ok)
					throw Error::COMError("IIDFromString() failed", hr);
			}

			explicit GloballyUniqueID(const BasicGuidString& guidString)
			{
				if (guidString.GuidString.empty())
					throw Error::Boring32Error("GUID cannot be an empty string");
				// CLSID, UUID, GUID seem to be equivalent
				// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-stringfromguid2
				// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-clsidfromstring
				// https://docs.microsoft.com/en-us/windows/win32/api/rpcdce/nf-rpcdce-uuidfromstringw
				// https://docs.microsoft.com/en-us/windows/win32/shell/guidfromstring
				const Win32::RPC_WSTR cString = Win32::RPC_WSTR(guidString.GuidString.c_str());
				// Does not accept {} around the GUID
				const Win32::RPC_STATUS status = Win32::UuidFromStringW(
					cString,
					&m_guid
				);
				// https://docs.microsoft.com/en-us/windows/win32/rpc/rpc-return-values
				// Not sure if this works, as RPC_STATUS is a long, not an unsigned long
				if (status != Win32::_RPC_S_OK)
					throw Error::Win32Error("UuidFromStringW() failed", status);
			}

			GloballyUniqueID(const GUID& guid)
				: m_guid(guid)
			{}

		public:
			GloballyUniqueID& operator=(const GUID& other) noexcept
			{
				m_guid = other;
				return *this;
			}

			bool operator==(const GloballyUniqueID& other) const noexcept
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/guiddef/nf-guiddef-isequalguid
				return Win32::IsEqualGUID(m_guid, other.m_guid);
			}

			bool operator==(const GUID& other) const noexcept
			{
				return Win32::IsEqualGUID(m_guid, other);
			}

		public:
			void ToString(std::wstring& out) const
			{
				out = GetGuidAsWString(m_guid);
			}

			void ToString(std::string& out) const
			{
				out = Strings::ConvertString(GetGuidAsWString(m_guid));
			}

			const GUID& Get() const noexcept
			{
				return m_guid;
			}

			bool IsNil() const noexcept
			{
				Win32::RPC_STATUS out;
				const int status = Win32::UuidIsNil(
					const_cast<Win32::GUID*>(&m_guid),
					&out
				);
				return status;
			}

		private:
			Win32::GUID m_guid = { 0 };
	};
}
