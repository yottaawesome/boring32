module;

#include <source_location>

export module boring32.msi:functions;
import <vector>;
import <string>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::MSI
{
	// See https://learn.microsoft.com/en-us/windows/win32/msi/installer-function-reference
	struct InstalledProducts
	{
		std::wstring ProductCode;
	};

	std::vector<InstalledProducts> GetInstalledProducts()
	{
		std::vector<InstalledProducts> returnValue;

		std::wstring productCode(38, '\0');
		DWORD index = 0;
		while (true)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msienumproductsexw
			const unsigned status = MsiEnumProductsExW(
				nullptr,
				nullptr,
				MSIINSTALLCONTEXT_MACHINE,
				index,
				productCode.data(),
				nullptr,
				nullptr,
				nullptr
			);
			if (status == ERROR_NO_MORE_ITEMS)
				return returnValue;
			if (status != ERROR_SUCCESS)
				throw Error::Win32Error("MsiEnumProductsExW() failed", status);

			returnValue.emplace_back(std::move(productCode));
			index++;
		}

		return returnValue;
	}
}
