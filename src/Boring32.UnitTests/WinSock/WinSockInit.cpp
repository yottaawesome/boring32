#include "pch.h"
#include <stdexcept>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winsock;
import boring32.winsock.winsockerror;

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