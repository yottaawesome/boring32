#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.util;
import boring32.error;

namespace Util
{
	// {BA6D5A7F-6777-4D86-876B-8E0D0D79D0E3}
	constexpr GUID TestGUID =
		{ 0xba6d5a7f, 0x6777, 0x4d86, { 0x87, 0x6b, 0x8e, 0xd, 0xd, 0x79, 0xd0, 0xe3 } };
	constexpr GUID NilGUID =
		{ 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

	TEST_CLASS(UtilGloballyUniqueID)
	{
		public:
			TEST_METHOD(TestGUIDConstructor)
			{
				Boring32::Util::GloballyUniqueID id(TestGUID);
				Assert::IsTrue(id == TestGUID);
			}

			TEST_METHOD(TestToString1)
			{
				Boring32::Util::GloballyUniqueID id(TestGUID);
				std::wstring guidString;
				id.ToString(guidString);
				Assert::IsTrue(guidString == L"{BA6D5A7F-6777-4D86-876B-8E0D0D79D0E3}");
			}

			TEST_METHOD(TestToString2)
			{
				Boring32::Util::GloballyUniqueID id(TestGUID);
				std::string guidString;
				id.ToString(guidString);
				Assert::IsTrue(guidString == "{BA6D5A7F-6777-4D86-876B-8E0D0D79D0E3}");
			}

			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::Util::GloballyUniqueID id;
			}

			TEST_METHOD(TestStringConstructor)
			{
				Boring32::Util::GloballyUniqueID id(Boring32::Util::GloballyUniqueID::BasicGuidString{L"BA6D5A7F-6777-4D86-876B-8E0D0D79D0E3"});
				Assert::IsTrue(id == TestGUID);
			}

			TEST_METHOD(TestInvalidStringConstructor)
			{
				Assert::ExpectException<Boring32::Error::Win32Error>(
					[]() 
					{ 
						Boring32::Util::GloballyUniqueID id(Boring32::Util::GloballyUniqueID::BasicGuidString{L"AAAAAAA"}); 
					});
			}

			TEST_METHOD(TestComparisonGUID)
			{
				Boring32::Util::GloballyUniqueID id(TestGUID);
				Assert::IsTrue(id == TestGUID);
			}

			TEST_METHOD(TestComparisonOwnType)
			{
				Boring32::Util::GloballyUniqueID id1(TestGUID);
				Boring32::Util::GloballyUniqueID id2(TestGUID);
				Assert::IsTrue(id1 == id2);
			}

			TEST_METHOD(TestCopyAssignment1)
			{
				Boring32::Util::GloballyUniqueID id1;
				id1 = TestGUID;
				Assert::IsTrue(id1 == TestGUID);
			}

			TEST_METHOD(TestCopyAssignment2)
			{
				Boring32::Util::GloballyUniqueID id1 = TestGUID;
				Boring32::Util::GloballyUniqueID id2 = id1;
				Assert::IsTrue(id2 == TestGUID);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Util::GloballyUniqueID id = Boring32::Util::GloballyUniqueID(TestGUID);
				Assert::IsTrue(id == TestGUID);
			}

			TEST_METHOD(TestGet)
			{
				Boring32::Util::GloballyUniqueID id = Boring32::Util::GloballyUniqueID(TestGUID);
				Assert::IsTrue(id == TestGUID);
			}

			TEST_METHOD(TestIsNil)
			{
				Boring32::Util::GloballyUniqueID id = Boring32::Util::GloballyUniqueID(NilGUID);
				Assert::IsTrue(id.IsNil());
			}

			TEST_METHOD(TestIsNotNil)
			{
				Boring32::Util::GloballyUniqueID id = Boring32::Util::GloballyUniqueID(TestGUID);
				Assert::IsFalse(id.IsNil());
			}
	};
}