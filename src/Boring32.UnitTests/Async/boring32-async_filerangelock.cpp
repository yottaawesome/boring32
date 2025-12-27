#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import std;
import boring32;

namespace Async
{
	using namespace Boring32::Async;

	TEST_CLASS(FileRangeLockTests)
	{
		// https://learn.microsoft.com/en-us/visualstudio/test/microsoft-visualstudio-testtools-cppunittestframework-api-reference?view=visualstudio
		TEST_METHOD_CLEANUP(Cleanup)
		{
			std::error_code ec;
			std::filesystem::remove("existing.txt", ec);
			std::filesystem::remove("new.txt", ec);
		}

		TEST_METHOD(TestDefaultConstructor)
		{
			Async::FileRangeLock fileLock;
			Assert::IsFalse(fileLock.HandleIsValid());
			Assert::IsTrue(fileLock.GetPath().empty());
		}

		TEST_METHOD(TestLockCreate)
		{
			Async::FileRangeLock fileLock("new.txt", true);
		}

		TEST_METHOD(TestLockExisting)
		{
			std::ofstream ofs("existing.txt");
			ofs.close();
			Async::FileRangeLock fileLock("existing.txt", true);
		}

		TEST_METHOD(TestMoveAssignment)
		{
			Async::FileRangeLock fileLock1("new.txt", true);
			Async::FileRangeLock fileLock2 = std::move(fileLock1);
			Assert::IsFalse(fileLock1.HandleIsValid());
			Assert::IsTrue(fileLock2.HandleIsValid());
			Assert::AreEqual(std::filesystem::path("new.txt").string(), fileLock2.GetPath().string());
		}

		TEST_METHOD(TestMoveConstructor)
		{
			Async::FileRangeLock fileLock1("new.txt", true);
			Async::FileRangeLock fileLock2{ std::move(fileLock1) };
			Assert::IsFalse(fileLock1.HandleIsValid());
			Assert::IsTrue(fileLock2.HandleIsValid());
			Assert::AreEqual(std::filesystem::path("new.txt").string(), fileLock2.GetPath().string());
		}

		TEST_METHOD(TestLock)
		{
			Async::FileRangeLock fileLock("new.txt", true);
			try
			{
				std::filesystem::remove("new.txt");
				Assert::IsTrue(false);
			}
			catch (...)
			{
				Assert::IsTrue(true);
			}
		}

		TEST_METHOD(TestLockUnlock)
		{
			{
				Async::FileRangeLock fileLock("new.txt", false);
				fileLock.lock();
				fileLock.unlock();
				// unlocking should not generate an error if the file is not locked
				fileLock.unlock();
			}
			std::filesystem::remove("new.txt");
		}

		TEST_METHOD(TestTryLock)
		{
			Async::FileRangeLock fileLock1("new.txt", true);
			Async::FileRangeLock fileLock2("new.txt", false);
			Assert::IsFalse(fileLock2.try_lock());
		}
	};
}
