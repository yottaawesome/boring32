export module boring32:util.guid;
import std;
import :win32;
import :error;
import :strings;
import :util.functions;

export namespace Boring32::Util
{
	struct EnhancedGuid : Win32::GUID 
	{
		constexpr EnhancedGuid() = default;
		constexpr EnhancedGuid(const Win32::GUID& other) : Win32::GUID{ other } {}
		constexpr bool operator==(const EnhancedGuid& other) const noexcept
		{ 
			return Data1 == other.Data1
				and Data2 == other.Data2
				and Data3 == other.Data3
				and std::ranges::equal(std::span{ Data4, 8 }, std::span{ other.Data4, 8 });
		}
	};
	static_assert(
		EnhancedGuid{} == Win32::GUID{ 0 },
		"Default constructed EnhancedGuid is expected to be the empty GUID."
	);

	struct GloballyUniqueID final
	{
		// GUID strings which aren't wrapped by {}
		struct BasicGuidString { std::wstring GuidString; };
		// GUID strings which are wrapped by {}
		struct WrappedGuidString { std::wstring GuidString; };

		GloballyUniqueID()
			: m_guid(GenerateGUID())
		{}

		explicit GloballyUniqueID(const WrappedGuidString& guidString)
		{
			if (guidString.GuidString.empty())
				throw Error::Boring32Error("GUID cannot be an empty string");
			// Accepts {} around the GUID
			// See https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iidfromstring
			Win32::HRESULT hr = Win32::IIDFromString(guidString.GuidString.c_str(), &m_guid);
			if (hr != Win32::S_Ok)
				throw Error::COMError(hr, "IIDFromString() failed");
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
			Win32::RPC_STATUS status = Win32::UuidFromStringW(cString, &m_guid);
			// https://docs.microsoft.com/en-us/windows/win32/rpc/rpc-return-values
			// Not sure if this works, as RPC_STATUS is a long, not an unsigned long
			if (status != Win32::_RPC_S_OK)
				throw Error::Win32Error(status, "UuidFromStringW() failed");
		}

		constexpr GloballyUniqueID(const Win32::GUID& guid)
			: m_guid(guid)
		{}

		constexpr GloballyUniqueID& operator=(const GUID& other) noexcept
		{
			m_guid = other;
			return *this;
		}

		constexpr bool operator==(const GloballyUniqueID& other) const noexcept
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/guiddef/nf-guiddef-isequalguid
			//return Win32::IsEqualGUID(m_guid, other.m_guid);
			return m_guid == other;
		}

		constexpr bool operator==(const Win32::GUID& other) const noexcept
		{
			return m_guid == EnhancedGuid{ other };
			//return Win32::IsEqualGUID(m_guid, other);
		}

		void ToString(std::wstring& out) const
		{
			out = GetGuidAsWString(m_guid);
		}

		void ToString(std::string& out) const
		{
			out = Strings::ConvertString(GetGuidAsWString(m_guid));
		}

		constexpr const GUID& Get() const noexcept
		{
			return m_guid;
		}

		constexpr bool IsNil() const noexcept
		{
			//Win32::RPC_STATUS out;
			//int status = Win32::UuidIsNil(const_cast<Win32::GUID*>(&m_guid), &out);
			return m_guid == EnhancedGuid{};
		}

		private:
		EnhancedGuid m_guid{};
	};
	static_assert(
		[]{
			Win32::GUID guid{ 1,2,3,{ 0,1,2,3,4,5,6,7 } };
			GloballyUniqueID nilGuid(guid);
			return nilGuid == guid;
		}(),
		"Equality check failed."
	);
}
