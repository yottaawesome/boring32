module;

#include <source_location>;

export module boring32.util:functions;
import <string>;
import <stdexcept>;
import <chrono>;
import <vector>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Util
{
	//std::vector<std::byte> StringToByteVector(const std::wstring_view str);
	//std::vector<std::byte> StringToByteVector(const std::string_view str);

	// based on https://stackoverflow.com/questions/45172052/correct-way-to-initialize-a-container-of-stdbyte
	template<typename... Ts>
	std::vector<std::byte> ToByteVector(Ts&&... args) noexcept
	{
		return{ std::byte(std::forward<Ts>(args))... };
	}

	template<typename T> requires std::is_same<std::wstring, T>::value || std::is_same<std::string, T>::value
		T ByteVectorToString(const std::vector<std::byte>&vector)
	{
		return { reinterpret_cast<T::const_pointer>(&vector[0]), vector.size() / sizeof(T::value_type) };
	}

	template<typename T> requires std::is_same<std::wstring, T>::value || std::is_same<std::string, T>::value
		std::vector<std::byte> StringToByteVector(const T & str)
	{
		return {
			reinterpret_cast<const std::byte*>(&str[0]),
			reinterpret_cast<const std::byte*>(&str[0]) + str.size() * sizeof(T::value_type)
		};
	}

	template<typename T, typename S>
	std::vector<T> ReinterpretVector(const std::vector<S>& in)
	{
		return {
			reinterpret_cast<const T*>(&in[0]),
			reinterpret_cast<const T*>(&in[0]) + (in.size() * sizeof(S)) / sizeof(T)
		};
	}

	typedef std::string(*blah)(const std::vector<std::byte>& vector);// = ByteVectorToString<std::string>;
	//static blah m = ByteVectorToString<std::string>;

	using x = std::string(*)(const std::vector<std::byte>& vector);

	std::wstring GetCurrentExecutableDirectory()
	{
		constexpr size_t blockSize = 2048;
		std::wstring filePath(L"\0", 0);
		DWORD status = ERROR_INSUFFICIENT_BUFFER;
		while (status == ERROR_INSUFFICIENT_BUFFER)
		{
			filePath.resize(filePath.size() + blockSize);
			status = GetModuleFileNameW(nullptr, &filePath[0], (DWORD)filePath.size());
			if (!status)
				throw Error::Win32Error("GetModuleFileNameW() failed", GetLastError());
		}

		const HRESULT result = PathCchRemoveFileSpec(&filePath[0], filePath.size());
		if (result != S_OK && result != S_FALSE)
			throw Error::COMError("PathCchRemoveFileSpec() failed", result);
		filePath = filePath.c_str();

		return filePath;
	}

	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li)
	{
		FILETIME ft{ 0 };
		ft.dwLowDateTime = li.LowPart;
		ft.dwHighDateTime = li.HighPart;
		SYSTEMTIME st{ 0 };
		if (FileTimeToSystemTime(&ft, &st) == false)
			throw Error::Win32Error("FileTimeToSystemTime() failed", GetLastError());
		return st;
	}

	size_t GetUnixTime() noexcept
	{
		const auto rightNow = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::seconds>(rightNow.time_since_epoch()).count();
	}

	size_t GetMillisToMinuteBoundary(const SYSTEMTIME& time, const size_t minuteBoundary) noexcept
	{
		size_t minutesToMillis = static_cast<size_t>(time.wMinute) * 60 * 1000;
		minutesToMillis += static_cast<size_t>(time.wSecond) * 1000;
		minutesToMillis += time.wMilliseconds;
		size_t boundaryToMillis = minuteBoundary * 60 * 1000;
		return boundaryToMillis - (minutesToMillis % boundaryToMillis);
	}

	size_t GetMillisToSecondBoundary(
		const SYSTEMTIME& time, 
		const size_t secondBoundary
	) noexcept
	{
		size_t currentSecondMillis = static_cast<size_t>(time.wSecond) * 1000;
		currentSecondMillis += time.wMilliseconds;
		size_t boundaryMillis = secondBoundary * 1000;
		return boundaryMillis - (currentSecondMillis % boundaryMillis);
	}

	// Adapted from https://stackoverflow.com/a/19941516/7448661
	std::wstring GetGuidAsWString(const GUID& guid)
	{
		std::wstring rawGuid(64, '\0');
		int numberOfChars = StringFromGUID2(guid, &rawGuid[0], 64);
		if (numberOfChars == 0)
			throw Error::Boring32Error("StringFromGUID2() failed");
		rawGuid.resize(numberOfChars - 1); // remove null terminator
		return rawGuid;
	}

	std::wstring GetGuidAsWString()
	{
		GUID guidReference;
		HRESULT result = CoCreateGuid(&guidReference);
		if (FAILED(result))
			throw Error::COMError("CoCreateGuid() failed", result);
		return GetGuidAsWString(guidReference);
	}

	GUID GenerateGUID()
	{
		GUID guid;
		HRESULT result = CoCreateGuid(&guid);
		if (FAILED(result))
			throw Error::COMError("CoCreateGuid() failed", result);
		return guid;
	}

	bool IsConnectedToInternet()
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/netlistmgr/nn-netlistmgr-inetworklistmanager
		Microsoft::WRL::ComPtr<INetworkListManager> networkListManager;
		// https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
		// https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-an-object-in-com
		HRESULT result = CoCreateInstance(
			CLSID_NetworkListManager,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_INetworkListManager,
			reinterpret_cast<void**>(networkListManager.GetAddressOf())
		);
		if (FAILED(result))
			throw Error::COMError("CoCreateGuid() failed", result);

		// https://docs.microsoft.com/en-us/windows/win32/api/netlistmgr/ne-netlistmgr-nlm_connectivity
		NLM_CONNECTIVITY connectivity;
		result = networkListManager->GetConnectivity(&connectivity);
		if (FAILED(result))
			throw Error::COMError("GetConnectivity() failed", result);

		if (connectivity & NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV4_INTERNET)
			return true;
		if (connectivity & NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV6_INTERNET)
			return true;
		return false;
	}
}