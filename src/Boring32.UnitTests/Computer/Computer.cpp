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
	};
}