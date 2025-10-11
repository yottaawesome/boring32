export module boring32:shell;
import std;
import boring32.win32;
import :error;
import :raii;

namespace Boring32::Shell
{
	using CoTaskMemFreeDeleteUniquePtr = RAII::UniquePtr<void, Win32::CoTaskMemFree>;
}

export namespace Boring32::Shell
{
	auto GetKnownFolderPath(const Win32::Shell::WellKnownFolder& folderId) -> std::filesystem::path
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
		wchar_t* out = nullptr;
		Win32::HRESULT hr = Win32::Shell::SHGetKnownFolderPath(
			folderId,
			0,
			nullptr,
			&out
		);
		CoTaskMemFreeDeleteUniquePtr ptr(out);
		if (Win32::HrFailed(hr))
			throw Error::COMError(hr, "SHGetKnownFolderPath() failed");
		
		return out 
			? std::filesystem::path{ std::wstring{ out } } 
			: std::filesystem::path{};
	}
}