export module boring32.util:functions;
import boring32.shared;
import boring32.error;
import boring32.concepts;

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

	template<typename T>
	concept IsString = 
		std::is_same_v<std::wstring, T> || std::is_same_v<std::string, T>;

	template<typename T> requires IsString<T>
	inline T ByteVectorToString(const std::vector<std::byte>& vector)
	{
		return { 
			reinterpret_cast<T::const_pointer>(&vector[0]), 
			vector.size() / sizeof(T::value_type) 
		};
	}

	template<typename T> requires IsString<T>
	inline std::vector<std::byte> StringToByteVector(const T& str)
	{
		return {
			reinterpret_cast<const std::byte*>(&str[0]),
			reinterpret_cast<const std::byte*>(&str[0]) + str.size() * sizeof(T::value_type)
		};
	}

	template<typename T, typename S>
	inline std::vector<T> ReinterpretVector(const std::vector<S>& in)
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
		Win32::DWORD status = Win32::ErrorCodes::InsufficientBuffer;
		while (status == Win32::ErrorCodes::InsufficientBuffer)
		{
			filePath.resize(filePath.size() + blockSize);
			status = Win32::GetModuleFileNameW(nullptr, &filePath[0], static_cast<Win32::DWORD>(filePath.size()));
			if (!status)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error("GetModuleFileNameW() failed", lastError);
			}
		}

		const Win32::HRESULT result = Win32::PathCchRemoveFileSpec(&filePath[0], filePath.size());
		if (result != Win32::S_Ok && result != Win32::_S_FALSE)
			throw Error::COMError("PathCchRemoveFileSpec() failed", result);
		filePath = filePath.c_str();

		return filePath;
	}

	Win32::SYSTEMTIME LargeIntegerTimeToSystemTime(const Win32::LARGE_INTEGER& li)
	{
		Win32::SYSTEMTIME st{ 0 };
		Win32::FILETIME ft{
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<Win32::DWORD>(li.HighPart)
		};
		if (!Win32::FileTimeToSystemTime(&ft, &st))
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("FileTimeToSystemTime() failed", lastError);
		}
		return st;
	}

	size_t GetUnixTime() noexcept
	{
		namespace ch = std::chrono;
		const auto rightNow = ch::system_clock::now();
		return ch::duration_cast<ch::seconds>(rightNow.time_since_epoch()).count();
	}

	size_t GetMillisToMinuteBoundary(const Win32::SYSTEMTIME& time, const size_t minuteBoundary) noexcept
	{
		size_t minutesToMillis = static_cast<size_t>(time.wMinute) * 60 * 1000;
		minutesToMillis += static_cast<size_t>(time.wSecond) * 1000;
		minutesToMillis += time.wMilliseconds;
		size_t boundaryToMillis = minuteBoundary * 60 * 1000;
		return boundaryToMillis - (minutesToMillis % boundaryToMillis);
	}

	size_t GetMillisToSecondBoundary(
		const Win32::SYSTEMTIME& time,
		const size_t secondBoundary
	) noexcept
	{
		size_t currentSecondMillis = static_cast<size_t>(time.wSecond) * 1000;
		currentSecondMillis += time.wMilliseconds;
		size_t boundaryMillis = secondBoundary * 1000;
		return boundaryMillis - (currentSecondMillis % boundaryMillis);
	}

	// Adapted from https://stackoverflow.com/a/19941516/7448661
	std::wstring GetGuidAsWString(const Win32::GUID& guid)
	{
		std::wstring rawGuid(64, '\0');
		int numberOfChars = Win32::StringFromGUID2(guid, &rawGuid[0], 64);
		if (numberOfChars == 0)
			throw Error::Boring32Error("StringFromGUID2() failed");
		rawGuid.resize(numberOfChars - 1); // remove null terminator
		return rawGuid;
	}

	std::wstring GetGuidAsWString()
	{
		Win32::GUID guidReference;
		Win32::HRESULT result = Win32::CoCreateGuid(&guidReference);
		if (Win32::HrFailed(result))
			throw Error::COMError("CoCreateGuid() failed", result);
		return GetGuidAsWString(guidReference);
	}

	GUID GenerateGUID()
	{
		Win32::GUID guid;
		HRESULT result = Win32::CoCreateGuid(&guid);
		if (Win32::HrFailed(result))
			throw Error::COMError("CoCreateGuid() failed", result);
		return guid;
	}

	template <Concepts::Variant TVariant, size_t N = 0>
	void RuntimeSet(TVariant& tup, size_t idx)
	{
		if (N == idx)
			tup = std::variant_alternative_t<N, TVariant>{};
		if constexpr (N + 1 < std::variant_size_v<TVariant>)
			RuntimeSet<TVariant, N + 1>(tup, idx);
	}
}