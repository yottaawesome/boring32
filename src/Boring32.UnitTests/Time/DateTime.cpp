#include "pch.h"
#include <vector>
#include <string>
#include "CppUnitTest.h"

import boring32.time;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Time
{
	TEST_CLASS(DateTime)
	{
		public:
			TEST_METHOD(TestAddSeconds)
			{
				SYSTEMTIME st{
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
				st = dt.ToSystemTime();
				Assert::IsTrue(st.wSecond == 31);
			}
	};
}