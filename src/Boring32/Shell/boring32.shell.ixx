export module boring32.shell;
import std;
import boring32.win32;
import boring32.error;

namespace Boring32::Shell
{
	struct CoTaskMemFreeDelete
	{
		void operator()(void* ptr)
		{
			Win32::CoTaskMemFree(ptr);
		}
	};
	using CoTaskMemFreeDeleteUniquePtr = std::unique_ptr<void, CoTaskMemFreeDelete>;
}

export namespace Boring32::Shell
{
	std::filesystem::path GetKnownFolderPath(
		const Win32::Shell::WellKnownFolder& folderId
	)
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
		wchar_t* out = nullptr;
		const Win32::HRESULT hr = Win32::Shell::SHGetKnownFolderPath(
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