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
	struct InstalledProduct
	{
		std::wstring ProductCode;
	};

	std::vector<InstalledProduct> GetInstalledProducts()
	{
		std::vector<InstalledProduct> returnValue;

		DWORD index = 0;
		while (true)
		{
			std::wstring productCode(38, '\0');
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

	std::wstring GetMsiProperty(
		const std::wstring& productCode,
		const std::wstring& propertyName
	)
	{
		DWORD characters = 0;
		unsigned status = MsiGetProductInfoExW(
			productCode.c_str(),
			nullptr,
			MSIINSTALLCONTEXT_MACHINE,
			propertyName.c_str(),
			nullptr,
			&characters
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error("MsiGetProductInfoExW() failed [1]", status);

		characters++;
		std::wstring returnValue(characters, '\0');
		status = MsiGetProductInfoExW(
			productCode.data(),
			nullptr,
			MSIINSTALLCONTEXT_MACHINE,
			propertyName.c_str(),
			returnValue.data(),
			&characters
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error("MsiGetProductInfoExW() failed [2]", status);
		returnValue.resize(characters);

		return returnValue;
	}

	struct InstalledProductInfo
	{
		std::wstring ProductCode;
		std::wstring Name;
		std::wstring OriginalMsiPackage;
	};
	
	InstalledProductInfo GetProductInfo(const InstalledProduct& product)
	{
		if (product.ProductCode.empty())
			return {};
		return {
			.ProductCode = product.ProductCode,
			.Name = GetMsiProperty(product.ProductCode, INSTALLPROPERTY_PRODUCTNAME),
			.OriginalMsiPackage = GetMsiProperty(product.ProductCode, INSTALLPROPERTY_PACKAGENAME)
		};
	}
}
