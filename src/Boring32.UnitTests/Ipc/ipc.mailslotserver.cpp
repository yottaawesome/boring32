#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace IPC
{
	TEST_CLASS(MailslotServer)
	{
		TEST_METHOD(TestConstructor)
		{
			auto server = Boring32::IPC::MailslotServer{L"\\\\.\\mailslot\\test", 1024, 5000};
			Assert::IsTrue(server.IsValid());
		}

		TEST_METHOD(TestConstructorEmptyName)
		{
			try
			{
				auto server = Boring32::IPC::MailslotServer{L"", 1024, 5000};
				Assert::Fail(L"Expected exception was not thrown.");
			}
			catch (const Boring32::Error::Boring32Error&)
			{
			}
			catch (...)
			{
				Assert::Fail(L"Unexpected exception type was thrown.");
			}
		}

		TEST_METHOD(TestClose)
		{
			auto server = Boring32::IPC::MailslotServer{L"\\\\.\\mailslot\\test", 1024, 5000};
			Assert::IsTrue(server.IsValid());
			server.Close();
			Assert::IsFalse(server.IsValid());
		}

		TEST_METHOD(TestMoveConstructor)
		{
			auto server1 = Boring32::IPC::MailslotServer{L"\\\\.\\mailslot\\test", 1024, 5000};
			auto server2 = std::move(server1);
			Assert::IsFalse(server1.IsValid());
			Assert::IsTrue(server2.IsValid());
		}

		TEST_METHOD(TestMoveAssignment)
		{
			auto server1 = Boring32::IPC::MailslotServer{L"\\\\.\\mailslot\\test", 1024, 5000};
			auto server2 = Boring32::IPC::MailslotServer{L"\\\\.\\mailslot\\test2", 2048, 10000};
			server2 = std::move(server1);
			Assert::IsFalse(server1.IsValid());
			Assert::IsTrue(server2.IsValid());
		}
		TEST_METHOD(TestGetters)
		{
			auto server = Boring32::IPC::MailslotServer{L"\\\\.\\mailslot\\test", 1024, 5000};
			Assert::AreEqual(std::wstring{L"\\\\.\\mailslot\\test"}, server.GetName());
			Assert::AreEqual(static_cast<Boring32::Win32::DWORD>(1024), server.GetMaxMessageSize());
			Assert::AreEqual(static_cast<Boring32::Win32::DWORD>(5000), server.GetReadTimeoutMs());
		}
	};
}
