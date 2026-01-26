#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace WinSock
{
	TEST_CLASS(Socket)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::WinSock::WinSockInit init(2,2);
				Boring32::WinSock::TCPSocket socket;
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::TCPSocket socket1(L"www.google.com", 80);
				socket1.Connect();
				Boring32::WinSock::TCPSocket socket2(std::move(socket1));
				Assert::IsTrue(socket1.GetHandle() == Boring32::WinSock::TCPSocket::InvalidSocket);
				Assert::IsTrue(socket2.GetHandle() > 0);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::TCPSocket socket1(L"www.google.com", 80);
				socket1.Connect();
				Boring32::WinSock::TCPSocket socket2;
				socket2 = std::move(socket1);
				Assert::IsTrue(socket1.GetHandle() == Boring32::WinSock::TCPSocket::InvalidSocket);
				Assert::IsTrue(socket2.GetHandle() > 0);
			}

			TEST_METHOD(TestBasicConstructor)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::TCPSocket socket(L"www.google.com", 80);
			}

			TEST_METHOD(TestConnect)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::TCPSocket socket(L"www.google.com", 80);
				socket.Connect();
			}

			TEST_METHOD(TestSendPacket)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::TCPSocket socket(L"www.google.com", 80);
				socket.Connect();
				socket.Send({ std::byte(0x5) });
			}

			TEST_METHOD(TestClose)
			{
				Boring32::WinSock::WinSockInit init(2, 2);
				Boring32::WinSock::TCPSocket socket(L"www.google.com", 80);
				socket.Open();
				socket.Connect();
				socket.Close();
				Assert::IsTrue(socket.GetHandle() == Boring32::WinSock::TCPSocket::InvalidSocket);
			}
	};
}