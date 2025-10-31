#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace Networking
{
	TEST_CLASS(ApiTests)
	{
		TEST_METHOD(TestGetAdapters)
		{
			auto vector = Boring32::Networking::GetAdapters(
				Boring32::Win32::WinSock::AddressFamily::IPv4,
				0
			);
			Assert::IsFalse(vector.empty());
		}
	};
}
