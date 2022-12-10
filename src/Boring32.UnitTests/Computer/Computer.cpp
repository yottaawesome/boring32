#include "pch.h"

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
			auto x = Boring32::Computer::GetSystemTimeAdjustmentInfo();
		}

		TEST_METHOD(TestGetSystemInfo)
		{
			// This function can't really fail, and we don't to test against the
			// results which are hardware dependent, so just leave it as it.
			auto x = Boring32::Computer::GetSystemInfo();
		}

		TEST_METHOD(TestGetLogicalProcessorInfo)
		{
			const auto result = 
				Boring32::Computer::GetLogicalProcessorInfo(LOGICAL_PROCESSOR_RELATIONSHIP::RelationAll);
		}
	};
}