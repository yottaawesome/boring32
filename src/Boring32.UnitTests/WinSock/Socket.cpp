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

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::Socket socket1(L"www.google.com", 80);
				socket1.Connect();
				Boring32::WinSock::Socket socket2(std::move(socket1));
				Assert::IsTrue(socket1.GetHandle() == 0);
				Assert::IsTrue(socket2.GetHandle() > 0);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::Socket socket1(L"www.google.com", 80);
				socket1.Connect();
				Boring32::WinSock::Socket socket2;
				socket2 = std::move(socket1);
				Assert::IsTrue(socket1.GetHandle() == 0);
				Assert::IsTrue(socket2.GetHandle() > 0);
			}

			TEST_METHOD(TestBasicConstructor)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::Socket socket(L"www.google.com", 80);
			}

			TEST_METHOD(TestConnect)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::Socket socket(L"www.google.com", 80);
				socket.Connect();
			}

			TEST_METHOD(TestSendPacket)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::Socket socket(L"www.google.com", 80);
				socket.Connect();
				socket.Send({ std::byte(0x5) });
			}
	};
}