#include "pch.h"

import boring32;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Services
{
	TEST_CLASS(Functions)
	{
		TEST_METHOD(TestServiceExistsSuccess)
		{
			Assert::IsTrue(Boring32::Services::ServiceExists(L"CoreMessagingRegistrar"));
		}

		TEST_METHOD(TestServiceExistsFailure)
		{
			Assert::IsFalse(Boring32::Services::ServiceExists(L"Boo"));
		}

		TEST_METHOD(TestServiceExistsEmptyName)
		{
			Assert::IsFalse(Boring32::Services::ServiceExists(L""));
		}
	};
}