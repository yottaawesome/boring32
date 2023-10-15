export module boring32.networking:api;
import <vector>;
import boring32.error;
import boring32.win32;

export namespace Boring32::Networking
{
	std::vector<std::byte> GetAdapters(const unsigned family, const unsigned flags)
	{
		unsigned long bufferSizeBytes = 15000;
		std::vector<std::byte> buffer(bufferSizeBytes);
		unsigned status = Win32::ErrorCodes::BufferOverflow;
		while (status != Win32::ErrorCodes::Success)
		{
			buffer.resize(bufferSizeBytes);
			// https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
			// https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh
			status = Win32::GetAdaptersAddresses(
				family,
				flags,
				nullptr,
				reinterpret_cast<Win32::IP_ADAPTER_ADDRESSES*>(buffer.data()),
				&bufferSizeBytes
			);
			// bufferSizeBytes will give the correct buffer size in this case,
			// and will be used to resize the buffer in the next iteration.
			if (status != Win32::ErrorCodes::BufferOverflow && status != Win32::ErrorCodes::Success)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error("GetAdaptersAddresses() failed", lastError);
			}
		}
		buffer.resize(bufferSizeBytes);
		// This is safe to do because the heap pointer is moved to the calling site,
		// so the linked-list next pointers are not modified and remain valid.
		return buffer;
	}
}