#include "pch.h"

import boring32;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Time
{
	TEST_CLASS(DateTime)
	{
		TEST_METHOD(TestAddSeconds)
		{
			const SYSTEMTIME st{
				.wYear = 2022,
				.wMonth = 8,
				.wDayOfWeek = 4,
				.wDay = 4,
				.wHour = 13,
				.wMinute = 25,
				.wSecond = 30,
				.wMilliseconds = 500
			};
			Boring32::Time::DateTime dt(st);
			dt.AddSeconds(1);
			Assert::IsTrue(dt.ToSystemTime().wSecond == 31);
		}

		TEST_METHOD(TestSubtractSeconds)
		{
			const SYSTEMTIME st{
				.wYear = 2022,
				.wMonth = 8,
				.wDayOfWeek = 4,
				.wDay = 4,
				.wHour = 13,
				.wMinute = 25,
				.wSecond = 30,
				.wMilliseconds = 500
			};
			Boring32::Time::DateTime dt(st);
			dt.AddSeconds(-1);
			Assert::IsTrue(dt.ToSystemTime().wSecond == 29);
		}
	};
}