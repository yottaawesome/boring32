#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import std;
import boring32;

namespace MSI
{
	TEST_CLASS(Functions)
	{
		TEST_METHOD(TestGetInstalledProducts)
		{
			std::vector products
				= Boring32::MSI::GetInstalledProducts();
			Assert::IsFalse(products.empty());
		}

		TEST_METHOD(TestGetProductInfo)
		{
			std::vector<Boring32::MSI::InstalledProduct> products
				= Boring32::MSI::GetInstalledProducts();
			Assert::IsFalse(products.empty());

			Boring32::MSI::InstalledProductInfo info = GetProductInfo(
				products.back()
			);

			Assert::IsFalse(info.ProductCode.empty());
			Assert::IsFalse(info.Name.empty());
			Assert::IsFalse(info.OriginalMsiPackage.empty());
		}
	};
}
