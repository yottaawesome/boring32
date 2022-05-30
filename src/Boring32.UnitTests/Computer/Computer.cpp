#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.computer;

namespace Computer
{
	TEST_CLASS(Functions)
	{
		TEST_METHOD(TestGetFormattedHostName)
		{
			Assert::IsFalse(Boring32::Computer::GetFormattedHostName(ComputerNameDnsFullyQualified).empty());
		}

		TEST_METHOD(TestGetTotalMemoryKB)
		{
			Assert::IsTrue(Boring32::Computer::GetTotalMemoryKB() > 0);
		}

		TEST_METHOD(TestGetMemoryStatus)
		{
			Assert::IsTrue(Boring32::Computer::GetMemoryStatus().ullAvailPhys > 0);
		}

		TEST_METHOD(TestGetMillisecondsSinceSystemBooted)
		{
			Assert::IsTrue(Boring32::Computer::GetMillisecondsSinceSystemBooted() > 0);
		}

		TEST_METHOD(TestGetSystemTimeAdjustmentInfo)
		{
			// Not sure how to reliably test this; just test it for now by making
			// sure no exceptions are raised.
			Boring32::Computer::GetSystemTimeAdjustmentInfo();
		}
	};
}