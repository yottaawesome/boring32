export module boring32:shell;
import boring32.shared;
import :error;
import :raii;

namespace Boring32::Shell
{
	using CoTaskMemFreeDeleteUniquePtr = RAII::UniquePtr<void, Win32::CoTaskMemFree>;
}

export namespace Boring32::Shell
{
	std::filesystem::path GetKnownFolderPath(const Win32::Shell::WellKnownFolder& folderId)
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
			throw Error::COMError("SHGetKnownFolderPath() failed", hr);
		
		return out 
			? std::filesystem::path{ std::wstring{ out } } 
			: std::filesystem::path{};
	}
}