#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.msi;

namespace MSI
{
	constexpr std::wstring_view MsiPath = LR"(C:\Temp\test.msi)";

	TEST_CLASS(Database)
	{

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::MSI::Database database(std::wstring{ MsiPath });
			}

			TEST_METHOD(TestGetProductVersion)
			{
				Boring32::MSI::Database database(std::wstring{ MsiPath });
				std::wstring version = database.GetProductVersion();
				Assert::IsFalse(version.empty());
			}
	};
}
