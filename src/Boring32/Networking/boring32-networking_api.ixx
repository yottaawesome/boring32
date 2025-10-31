export module boring32:networking_api;
import std;
import boring32.win32;
import :error;

export namespace Boring32::Networking
{
	auto GetAdapters(unsigned family, unsigned flags) -> std::vector<std::byte>
	{
		unsigned long bufferSizeBytes = 0;
		std::vector<std::byte> buffer(bufferSizeBytes);
		while (true)
		{
			bufferSizeBytes += 15000;
			buffer.resize(bufferSizeBytes);
			// https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
			// https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh
			auto status = Win32::GetAdaptersAddresses(
				family,
				flags,
				nullptr,
				reinterpret_cast<Win32::IP_ADAPTER_ADDRESSES*>(buffer.data()),
				&bufferSizeBytes
			);
			if (status == Win32::ErrorCodes::Success)
			{
				buffer.resize(bufferSizeBytes);
				// This is safe to do because the heap pointer is moved to the calling site,
				// so the linked-list next pointers are not modified and remain valid.
				return buffer;
			}

			// bufferSizeBytes will give the correct buffer size in this case,
			// and will be used to resize the buffer in the next iteration.
			if (status != Win32::ErrorCodes::BufferOverflow)
			{
				auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "GetAdaptersAddresses() failed");
			}
		}
	}

	auto IsConnectedToInternet() -> bool
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/netlistmgr/nn-netlistmgr-inetworklistmanager
		Win32::ComPtr<Win32::INetworkListManager> networkListManager;
		// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
		// https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-an-object-in-com
		Win32::HRESULT result = Win32::CoCreateInstance(
			Win32::CLSID_NetworkListManager,
			nullptr,
			Win32::CLSCTX::CLSCTX_INPROC_SERVER,
			Win32::IID_INetworkListManager,
			reinterpret_cast<void**>(networkListManager.GetAddressOf())
		);
		if (Win32::HrFailed(result))
			throw Error::COMError(result, "CoCreateGuid() failed");

		// https://docs.microsoft.com/en-us/windows/win32/api/netlistmgr/ne-netlistmgr-nlm_connectivity
		Win32::NLM_CONNECTIVITY connectivity;
		result = networkListManager->GetConnectivity(&connectivity);
		if (Win32::HrFailed(result))
			throw Error::COMError(result, "GetConnectivity() failed");

		if (connectivity & Win32::NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV4_INTERNET)
			return true;
		if (connectivity & Win32::NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV6_INTERNET)
			return true;
		return false;
	}
}