module;

#include <vector>
#include <source_location>
#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <iptypes.h>

export module boring32.networking:api;
import boring32.error;

export namespace Boring32::Networking
{
	std::vector<std::byte> GetAdapters(const unsigned family, const unsigned flags)
	{
		ULONG bufferSizeBytes = 15000;
		std::vector<std::byte> buffer(bufferSizeBytes);
		ULONG status = ERROR_BUFFER_OVERFLOW;
		while (status != ERROR_SUCCESS)
		{
			buffer.resize(bufferSizeBytes);
			// https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
			// https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh
			status = GetAdaptersAddresses(
				family,
				flags,
				nullptr,
				reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data()),
				&bufferSizeBytes
			);
			// bufferSizeBytes will give the correct buffer size in this case,
			// and will be used to resize the buffer in the next iteration.
			if (status != ERROR_BUFFER_OVERFLOW && status != ERROR_SUCCESS)
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error("GetAdaptersAddresses() failed", lastError);
			}
		}
		if (!bufferSizeBytes)
			return {};
		// This is safe to do because the heap pointer is moved to the calling site,
		// so the linked-list next pointers are not modified and remain valid.
		return buffer;
	}
}