#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.msi;

namespace MSI
{
	TEST_CLASS(Functions)
	{
		public:
			TEST_METHOD(TestGetInstalledProducts)
			{
				std::vector<Boring32::MSI::InstalledProduct> products
					= Boring32::MSI::GetInstalledProducts();
				Assert::IsFalse(products.empty());
			}
	};
}
