#include "pch.h"
#include <stdexcept>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winsock;
import boring32.winsock.winsockerror;

namespace WinSock
{
	TEST_CLASS(Socket)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::WinSock::WinSockInit init(2,2);
				Boring32::WinSock::Socket socket;
			}

			TEST_METHOD(TestBasicConstructor)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::Socket socket(L"www.google.com", 80);
			}
	};
}