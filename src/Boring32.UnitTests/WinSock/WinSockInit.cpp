#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winsock;

namespace WinSock
{
	TEST_CLASS(WinSockInit)
	{
		public:
			TEST_METHOD(TestInit)
			{
				Boring32::WinSock::WinSockInit init(2,2);
			}

			TEST_METHOD(TestBadInit)
			{
				Assert::ExpectException<Boring32::WinSock::WinSockError>(
					[]() { Boring32::WinSock::WinSockInit init(0, 0); }
				);
			}
	};
}