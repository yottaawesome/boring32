export module boring32:util.functions;
import std;
import :win32;
import :error;
import :concepts;

export namespace Boring32::Util
{
	// based on https://stackoverflow.com/questions/45172052/correct-way-to-initialize-a-container-of-stdbyte
	template<typename... Ts>
	auto ToByteVector(Ts&&... args) noexcept -> std::vector<std::byte>
	{
		return{ std::byte(std::forward<Ts>(args))... };
	}

	template<typename T> requires Concepts::WideOrNarrowString<T>
	auto ByteVectorToString(const std::vector<std::byte>& vector) -> T
	{
		return { 
			reinterpret_cast<T::const_pointer>(&vector[0]), 
			vector.size() / sizeof(T::value_type) 
		};
	}

	template<typename T> requires Concepts::WideOrNarrowString<T>
	auto StringToByteVector(const T& str) -> std::vector<std::byte>
	{
		return {
			reinterpret_cast<const std::byte*>(&str[0]),
			reinterpret_cast<const std::byte*>(&str[0]) + str.size() * sizeof(T::value_type)
		};
	}

	template<typename T, typename S>
	auto ReinterpretVector(const std::vector<S>& in) -> std::vector<T>
	{
		return {
			reinterpret_cast<const T*>(&in[0]),
			reinterpret_cast<const T*>(&in[0]) + (in.size() * sizeof(S)) / sizeof(T)
		};
	}

	auto GetCurrentExecutablePath() -> std::filesystem::path
	{
		constexpr size_t blockSize = 2048;
		std::wstring filePath(L"\0", 0);
		Win32::DWORD status = Win32::ErrorCodes::InsufficientBuffer;
		Win32::DWORD count = 0;
		while (status == Win32::ErrorCodes::InsufficientBuffer)
		{
			filePath.resize(filePath.size() + blockSize);
			count = Win32::GetModuleFileNameW(
				nullptr, 
				filePath.data(), 
				static_cast<Win32::DWORD>(filePath.size())
			);
			if (not count)
				throw Error::Win32Error(Win32::GetLastError(), "GetModuleFileNameW() failed");
			status = Win32::GetLastError();
		}
		filePath.resize(count);
		return filePath;
	}

	auto GetCurrentExecutableDirectory() -> std::filesystem::path
	{
		// Don't actually need to do this, can just use parent_path() on the path object.
		// This is just done like this to show how to use PathCchRemoveFileSpec.
		auto filePath = GetCurrentExecutablePath().wstring();
		Win32::HRESULT result = Win32::PathCchRemoveFileSpec(&filePath[0], filePath.size());
		if (result != Win32::S_Ok && result != Win32::_S_FALSE)
			throw Error::COMError(result, "PathCchRemoveFileSpec() failed");
		filePath = filePath.c_str();
		return filePath;
	}

	auto LargeIntegerTimeToSystemTime(const Win32::LARGE_INTEGER& li) -> Win32::SYSTEMTIME
	{
		Win32::SYSTEMTIME st{ 0 };
		Win32::FILETIME ft{
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<Win32::DWORD>(li.HighPart)
		};
		if (not Win32::FileTimeToSystemTime(&ft, &st))
			throw Error::Win32Error(Win32::GetLastError(), "FileTimeToSystemTime() failed");
		return st;
	}

	auto GetUnixTime() noexcept -> size_t
	{
		namespace ch = std::chrono;
		const auto rightNow = ch::system_clock::now();
		return ch::duration_cast<ch::seconds>(rightNow.time_since_epoch()).count();
	}

	auto GetMillisToMinuteBoundary(
		const Win32::SYSTEMTIME& time, 
		size_t minuteBoundary
	) noexcept -> size_t
	{
		size_t minutesToMillis = static_cast<size_t>(time.wMinute) * 60 * 1000;
		minutesToMillis += static_cast<size_t>(time.wSecond) * 1000;
		minutesToMillis += time.wMilliseconds;
		size_t boundaryToMillis = minuteBoundary * 60 * 1000;
		return boundaryToMillis - (minutesToMillis % boundaryToMillis);
	}

	auto GetMillisToSecondBoundary(
		const Win32::SYSTEMTIME& time,
		size_t secondBoundary
	) noexcept -> size_t
	{
		size_t currentSecondMillis = static_cast<size_t>(time.wSecond) * 1000;
		currentSecondMillis += time.wMilliseconds;
		size_t boundaryMillis = secondBoundary * 1000;
		return boundaryMillis - (currentSecondMillis % boundaryMillis);
	}

	// Adapted from https://stackoverflow.com/a/19941516/7448661
	auto GetGuidAsWString(const Win32::GUID& guid) -> std::wstring
	{
		std::wstring rawGuid(64, '\0');
		int numberOfChars = Win32::StringFromGUID2(guid, &rawGuid[0], 64);
		if (numberOfChars == 0)
			throw Error::Boring32Error("StringFromGUID2() failed");
		rawGuid.resize(numberOfChars - 1); // remove null terminator
		return rawGuid;
	}

	auto GetGuidAsWString() -> std::wstring
	{
		Win32::GUID guidReference;
		Win32::HRESULT result = Win32::CoCreateGuid(&guidReference);
		if (Win32::HrFailed(result))
			throw Error::COMError(result, "CoCreateGuid() failed");
		return GetGuidAsWString(guidReference);
	}

	auto GenerateGUID() -> GUID
	{
		Win32::GUID guid;
		HRESULT result = Win32::CoCreateGuid(&guid);
		if (Win32::HrFailed(result))
			throw Error::COMError(result, "CoCreateGuid() failed");
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

	constexpr auto Decompose(std::uint64_t value) noexcept -> std::pair<Win32::DWORD, Win32::DWORD>
	{
		const Win32::DWORD
			low = static_cast<Win32::DWORD>(value & 0xFFFFFFFF),
			high = static_cast<Win32::DWORD>((value >> 32) & 0xFFFFFFFF);
		return { low, high };
	}
}