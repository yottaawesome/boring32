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
				std::wstring value = database.GetProductVersion();
				Assert::IsFalse(value.empty());
			}

			TEST_METHOD(TestGetUpgradeCode)
			{
				Boring32::MSI::Database database(std::wstring{ MsiPath });
				std::wstring value = database.GetUpgradeCode();
				Assert::IsFalse(value.empty());
			}

			TEST_METHOD(TestGetProductCode)
			{
				Boring32::MSI::Database database(std::wstring{ MsiPath });
				std::wstring value = database.GetProductCode();
				Assert::IsFalse(value.empty());
			}

			TEST_METHOD(TestGetProductName)
			{
				Boring32::MSI::Database database(std::wstring{ MsiPath });
				std::wstring value = database.GetProductName();
				Assert::IsFalse(value.empty());
			}

			TEST_METHOD(TestGetProductLanguage)
			{
				Boring32::MSI::Database database(std::wstring{ MsiPath });
				std::wstring value = database.GetProductLanguage();
				Assert::IsFalse(value.empty());
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::MSI::Database database1(std::wstring{ MsiPath });
				Boring32::MSI::Database database2(database1);
				Assert::IsTrue(database1);
				Assert::IsTrue(database2);
				Assert::IsTrue(database1.GetPath() == database2.GetPath());
				Assert::IsTrue(database1.GetMode() == database2.GetMode());
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::MSI::Database database1(std::wstring{ MsiPath });
				Boring32::MSI::Database database2(std::wstring{ MsiPath });
				database2 = database1;
				Assert::IsTrue(database1);
				Assert::IsTrue(database2);
				Assert::IsTrue(database1.GetPath() == database2.GetPath());
				Assert::IsTrue(database1.GetMode() == database2.GetMode());
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::MSI::Database database1(std::wstring{ MsiPath });
				Boring32::MSI::Database database2(std::move(database1));
				Assert::IsFalse(database1);
				Assert::IsTrue(database2);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::MSI::Database database1(std::wstring{ MsiPath });
				Boring32::MSI::Database database2(std::wstring{ MsiPath });
				database2 = std::move(database1);
				Assert::IsFalse(database1);
				Assert::IsTrue(database2);
			}

			TEST_METHOD(TestGetPath)
			{
				std::wstring path{ MsiPath };
				Boring32::MSI::Database database(path);
				Assert::IsTrue(database.GetPath() == path);
			}

			TEST_METHOD(TestGetMode)
			{
				Boring32::MSI::Database database(
					std::wstring{ MsiPath },
					Boring32::MSI::Mode::ReadOnly
				);
				Assert::IsTrue(database.GetMode() == Boring32::MSI::Mode::ReadOnly);
			}
	};
}
