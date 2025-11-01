export module boring32:networking_api;
import std;
import boring32.win32;
import :error;

export namespace Boring32::Networking
{
	struct Adapters
	{
		template<typename T>
		struct Iterator
		{
			T* Ptr = nullptr;

			explicit Iterator(T* ptr) 
				: Ptr{ ptr } 
			{}

			auto operator*(this auto&& self)
			{ 
				return self.Ptr; 
			}

			auto operator->(this auto&& self)
			{
				return self.Ptr;
			}

			auto operator++(this auto&& self)
			{
				self.Ptr = self.Ptr->Next;
				return std::forward_like<decltype(self)>(self);
			}

			auto operator!=(this auto&& self, const Iterator& other) -> bool
			{
				return self.Ptr != other.Ptr;
			}
		};

		using BasicIterator = Iterator<Win32::IP_ADAPTER_ADDRESSES>;
		using ConstIterator = Iterator<const Win32::IP_ADAPTER_ADDRESSES>;

		static constexpr unsigned ExpansionIncrement = 15000;

		Adapters(size_t initialSize) noexcept
			: Buffer{ std::vector<std::byte>(initialSize) }
		{ }

		Adapters() noexcept
			: Buffer{ std::vector<std::byte>(ExpansionIncrement) }
		{ }

		auto Expand(this auto&& self) -> size_t
		{
			self.Buffer.resize(self.Buffer.size() + ExpansionIncrement);
			return self.Buffer.size();
		}

		auto Resize(this auto&& self, size_t size) -> size_t
		{
			self.Buffer.resize(size);
			return size;
		}

		auto Size(this auto&& self) -> size_t
		{
			return self.Buffer.size();
		}

		auto begin(this auto&& self)
		{
			// Also works
			/*Win32::IP_ADAPTER_ADDRESSES* ptr = nullptr;
			auto x = std::forward_like<decltype(self)>(ptr);
			return Iterator{ reinterpret_cast<decltype(x)>(self.Buffer.data()) };*/
			if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
				return ConstIterator{ reinterpret_cast<const Win32::IP_ADAPTER_ADDRESSES*>(self.Buffer.data()) };
			else
				return BasicIterator{ reinterpret_cast<Win32::IP_ADAPTER_ADDRESSES*>(self.Buffer.data()) };
		}

		auto end(this auto&& self)
		{ 
			if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
				return ConstIterator{ nullptr };
			else
				return BasicIterator{ nullptr };
		}

		std::vector<std::byte> Buffer;
	};

	auto GetAdapters(unsigned family, unsigned flags) -> Adapters
	{
		unsigned long bufferSizeBytes = 15000;
		Adapters result{ 15000 };
		while (true)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
			// https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh
			auto status = Win32::GetAdaptersAddresses(
				family,
				flags,
				nullptr,
				reinterpret_cast<Win32::IP_ADAPTER_ADDRESSES*>(result.Buffer.data()),
				&bufferSizeBytes
			);
			if (status == Win32::ErrorCodes::Success)
			{
				result.Resize(bufferSizeBytes);
				// This is safe to do because the heap pointer is moved to the calling site,
				// so the linked-list next pointers are not modified and remain valid.
				return result;
			}

			// bufferSizeBytes will give the correct result size in this case,
			// and will be used to resize the result in the next iteration.
			if (status != Win32::ErrorCodes::BufferOverflow)
			{
				auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "GetAdaptersAddresses() failed");
			}

			bufferSizeBytes = static_cast<unsigned long>(result.Expand());
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