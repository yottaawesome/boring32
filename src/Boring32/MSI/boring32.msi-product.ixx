export module boring32.msi:product;
import std;
import std.compat;
import boring32.win32;
import boring32.error;

export namespace Boring32::MSI
{
	class Product
	{
		public:
			static bool IsProductElevated(const std::wstring& path)
			{
				Win32::BOOL isElevated;
				// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msiisproductelevatedw
				unsigned status = Win32::MsiIsProductElevatedW(
					path.c_str(),
					&isElevated
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiIsProductElevatedW() failed", status);
				return isElevated;
			}
	};
}