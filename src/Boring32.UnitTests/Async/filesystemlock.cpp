#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import std;
import boring32;

namespace Async
{
	using namespace Boring32::Async;

	TEST_CLASS(FileSystemLockTests)
	{
		TEST_METHOD(TestDefaultConstructor)
		{
			Async::FileSystemLock<> fileLock;
			Assert::IsFalse(fileLock.IsLocked());
		}

		TEST_METHOD(TestLockCreate)
		{
			Async::FileSystemLock<> fileLock;
		}

		TEST_METHOD(TestMoveAssignment)
		{
			Async::FileSystemLock<> fileLock1(true);
			Async::FileSystemLock<> fileLock2 = std::move(fileLock1);
			Assert::IsFalse(fileLock1.IsLocked());
			Assert::IsTrue(fileLock2.IsLocked());
		}

		TEST_METHOD(TestMoveConstructor)
		{
			Async::FileSystemLock<> fileLock1(true);
			Async::FileSystemLock<> fileLock2{ std::move(fileLock1) };
			Assert::IsFalse(fileLock1.IsLocked());
			Assert::IsTrue(fileLock2.IsLocked());
		}

		TEST_METHOD(TestLock)
		{
			Async::FileSystemLock<L"new.txt"> fileLock(true);
			Assert::IsTrue(std::filesystem::exists("new.txt"));
		}

		TEST_METHOD(TestLockUnlock)
		{
			{
				Async::FileSystemLock<L"new.txt"> fileLock;
				fileLock.lock();
				Assert::IsTrue(std::filesystem::exists("new.txt"));
				fileLock.unlock();
				// unlocking should not generate an error if the file is not locked
				fileLock.unlock();
			}
			Assert::IsFalse(std::filesystem::exists("new.txt"));
		}

		TEST_METHOD(TestTryLock)
		{
			Async::FileSystemLock<L"new.txt"> fileLock1(true);
			Async::FileSystemLock<L"new.txt"> fileLock2(false);
			Assert::IsFalse(fileLock2.try_lock());
		}
	};
}
