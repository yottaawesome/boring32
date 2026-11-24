export module Boring32:MSI.Product;
import std;
import :Win32;
import :Error;

export namespace Boring32::MSI
{
	struct Product
	{
		static auto IsProductElevated(const std::wstring& path) -> bool
		{
			auto isElevated = Win32::BOOL{};
			// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msiisproductelevatedw
			auto status = Win32::MsiIsProductElevatedW(path.c_str(), &isElevated);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiIsProductElevatedW() failed"};
			return isElevated;
		}
	};
}
