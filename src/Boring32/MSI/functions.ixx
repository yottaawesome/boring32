export module boring32:msi.functions;
import std;
import :win32;
import :error;

export namespace Boring32::MSI
{
	// See https://learn.microsoft.com/en-us/windows/win32/msi/installer-function-reference
	struct InstalledProduct
	{
		std::wstring ProductCode;
	};

	auto GetInstalledProducts() -> std::vector<InstalledProduct>
	{
		auto returnValue = std::vector<InstalledProduct>{};

		auto index = Win32::DWORD{};
		while (true)
		{
			auto productCode = std::wstring(38, '\0');
			// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msienumproductsexw
			auto status = 
				Win32::MsiEnumProductsExW(
					nullptr,
					nullptr,
					Win32::MsiInstallContextMachine,
					index,
					productCode.data(),
					nullptr,
					nullptr,
					nullptr
				);
			if (status == Win32::ErrorCodes::NoMoreItems)
				return returnValue;
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiEnumProductsExW() failed"};

			returnValue.emplace_back(std::move(productCode));
			index++;
		}

		return returnValue;
	}

	auto GetMsiProperty(
		const std::wstring& productCode,
		const std::wstring& propertyName,
		bool* propertyFound = nullptr
	) -> std::wstring
	{
		if (propertyFound)
			*propertyFound = false;

		// See https://learn.microsoft.com/en-us/windows/win32/msi/required-properties
		// and https://learn.microsoft.com/en-us/windows/win32/msi/properties
		auto characters = Win32::DWORD{};
		// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msigetproductinfow
		// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msigetproductinfoexw
		auto status = 
			Win32::MsiGetProductInfoExW(
				productCode.c_str(),
				nullptr,
				Win32::MsiInstallContextMachine,
				propertyName.c_str(),
				nullptr,
				&characters
			);
		if (status == Win32::ErrorCodes::UnknownProperty)
			return {};
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error{status, "MsiGetProductInfoExW() failed [1]"};

		// The returned character count excludes the null terminator,
		// but this is required, so we bump the value.
		characters++;
		auto returnValue = std::wstring(characters, '\0');
		status = 
			Win32::MsiGetProductInfoExW(
				productCode.data(),
				nullptr,
				Win32::MsiInstallContextMachine,
				propertyName.c_str(),
				returnValue.data(),
				&characters
			);
		if (status == Win32::ErrorCodes::UnknownProperty)
			return {};
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error{status, "MsiGetProductInfoExW() failed [2]"};

		returnValue.resize(characters);
		if (propertyFound)
			*propertyFound = true;

		return returnValue;
	}

	struct InstalledProductInfo
	{
		std::wstring ProductCode;
		std::wstring Name;
		std::wstring OriginalMsiPackage;
	};
	
	auto GetProductInfo(const InstalledProduct& product) -> InstalledProductInfo
	{
		if (product.ProductCode.empty())
			return {};
		return {
			.ProductCode = product.ProductCode,
			.Name = GetMsiProperty(product.ProductCode, Win32::InstallProperty_ProductName),
			.OriginalMsiPackage = GetMsiProperty(product.ProductCode, Win32::InstallProperty_PackageName)
		};
	}

	auto FindProductCodeByName(const std::wstring& productName) -> InstalledProductInfo
	{
		auto productCodes = std::vector{ GetInstalledProducts() };

		for (const InstalledProduct& code : productCodes)
		{
			auto name = std::wstring{ GetMsiProperty(code.ProductCode, Win32::InstallProperty_ProductName) };
			if (name != productName)
				continue;
			return {
				.ProductCode = code.ProductCode,
				.Name = std::move(productName),
				.OriginalMsiPackage = GetMsiProperty(code.ProductCode, Win32::InstallProperty_PackageName)
			};
		}
		return {};
	}
}
