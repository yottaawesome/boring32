#include "pch.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.util;

namespace Util
{
	TEST_CLASS(Util)
	{
		public:
			TEST_METHOD(TestToByteVector)
			{
				std::vector bytes = Boring32::Util::ToByteVector(0x0, 0x1, 0x2, 0x3);
				Assert::IsTrue(bytes.size() == 4);
				for(int i = 0; i < bytes.size(); i++)
					Assert::IsTrue(bytes[i] == (std::byte)i);
			}

			TEST_METHOD(TestGetCurrentExecutableDirectory)
			{
				std::wstring path = Boring32::Util::GetCurrentExecutableDirectory();
				Assert::IsFalse(path.empty());
			}

			TEST_METHOD(TestToByteVectorRoundtrip1)
			{
				std::string string = "hello, world";
				std::vector<std::byte> byteVector = Boring32::Util::StringToByteVector(string);
				std::string result = Boring32::Util::ByteVectorToString<std::string>(byteVector);
				Assert::IsTrue(string == result);
			}

			TEST_METHOD(TestToByteVectorRoundtrip2)
			{
				std::wstring string = L"hello, world";
				std::vector<std::byte> byteVector = Boring32::Util::StringToByteVector(string);
				std::wstring result = Boring32::Util::ByteVectorToString<std::wstring>(byteVector);
				Assert::IsTrue(string == result);
			}
	};
}