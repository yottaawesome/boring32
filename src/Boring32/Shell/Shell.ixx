export module Boring32:Shell;
import std;
import :Win32;
import :Error;
import :Raii;

namespace Boring32::Shell
{
	using CoTaskMemFreeDeleteUniquePtr = RAII::UniquePtr<void, Win32::CoTaskMemFree>;
}

export namespace Boring32::Shell
{
	auto GetKnownFolderPath(const Win32::Shell::WellKnownFolder& folderId) -> std::filesystem::path
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
		auto out = (wchar_t*)nullptr;
		auto hr = Win32::Shell::SHGetKnownFolderPath(
			folderId,
			0,
			nullptr,
			&out
		);
		auto ptr = CoTaskMemFreeDeleteUniquePtr{ out };
		if (Win32::HrFailed(hr))
			throw Error::COMError{hr, "SHGetKnownFolderPath() failed"};
		
		return out 
			? std::filesystem::path{ std::wstring{ out } } 
			: std::filesystem::path{};
	}
}