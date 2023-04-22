module boring32.util:guid;
import :functions;
import boring32.error;
import boring32.strings;

namespace Boring32::Util
{
	GloballyUniqueID::GloballyUniqueID() 
		: m_guid(GenerateGUID())
	{}

	GloballyUniqueID::GloballyUniqueID(const std::wstring& guidString)
		: m_guid{0}
	{
		// CLSID, UUID, GUID seem to be equivalent
		// See https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-iidfromstring
		// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-stringfromguid2
		// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-clsidfromstring
		// https://docs.microsoft.com/en-us/windows/win32/api/rpcdce/nf-rpcdce-uuidfromstringw
		// https://docs.microsoft.com/en-us/windows/win32/shell/guidfromstring
		const RPC_WSTR cString = RPC_WSTR(guidString.c_str());
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
	
	GloballyUniqueID& GloballyUniqueID::operator=(const GUID& other) noexcept
	{
		m_guid = other;
		return *this;
	}

	GloballyUniqueID::GloballyUniqueID(const GUID& guid)
		: m_guid(guid)
	{}

	void GloballyUniqueID::ToString(std::wstring& out) const
	{
		out = GetGuidAsWString(m_guid);
	}

	void GloballyUniqueID::ToString(std::string& out) const
	{
		out = Strings::ConvertString(GetGuidAsWString(m_guid));
	}

	const GUID& GloballyUniqueID::Get() const noexcept
	{
		return m_guid;
	}

	bool GloballyUniqueID::operator==(const GloballyUniqueID& other) const noexcept
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/guiddef/nf-guiddef-isequalguid
		return IsEqualGUID(m_guid, other.m_guid);
	}

	bool GloballyUniqueID::operator==(const GUID& other) const noexcept
	{
		return IsEqualGUID(m_guid, other);
	}
}
