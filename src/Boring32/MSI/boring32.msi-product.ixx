export module boring32.msi:product;
import std;

import <win32.hpp>;
import boring32.error;

export namespace Boring32::MSI
{
	class Product
	{
		public:
			static bool IsProductElevated(const std::wstring& path)
			{
				BOOL isElevated;
				// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msiisproductelevatedw
				unsigned status = MsiIsProductElevatedW(
					path.c_str(),
					&isElevated
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiIsProductElevatedW() failed", status);
				return isElevated;
			}
	};
}