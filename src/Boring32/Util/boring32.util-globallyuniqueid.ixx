module;

#include <source_location>;

export module boring32.util:guid;
import <string>;
import <win32.hpp>;
import :functions;
import boring32.error;
import boring32.strings;

export namespace Boring32::Util
{
	class GloballyUniqueID final
	{
		public:
			struct BasicGuidString { std::wstring GuidString; };
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
			explicit GloballyUniqueID(const WrappedGuidString& guid)
			{
				const HRESULT hr = IIDFromString(guid.GuidString.c_str(), &m_guid);
				if (hr != S_OK)
					throw Error::COMError("IIDFromString() failed", hr);
			}

			explicit GloballyUniqueID(const BasicGuidString& guidString)
			{
				// CLSID, UUID, GUID seem to be equivalent
				// See https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iidfromstring
				// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-stringfromguid2
				// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-clsidfromstring
				// https://docs.microsoft.com/en-us/windows/win32/api/rpcdce/nf-rpcdce-uuidfromstringw
				// https://docs.microsoft.com/en-us/windows/win32/shell/guidfromstring
				const RPC_WSTR cString = RPC_WSTR(guidString.GuidString.c_str());
				// Does not accept {} around the GUID
				const RPC_STATUS status = UuidFromStringW(
					cString,
					&m_guid
				);
				// https://docs.microsoft.com/en-us/windows/win32/rpc/rpc-return-values
				// Not sure if this works, as RPC_STATUS is a long, not an unsigned long
				if (status != RPC_S_OK)
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
				return IsEqualGUID(m_guid, other.m_guid);
			}

			bool operator==(const GUID& other) const noexcept
			{
				return IsEqualGUID(m_guid, other);
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
				RPC_STATUS out;
				const int status = UuidIsNil(
					const_cast<GUID*>(&m_guid),
					&out
				);
				return status;
			}

		private:
			GUID m_guid = { 0 };
	};
}
