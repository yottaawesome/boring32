#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import std;
import boring32;
import boring32.win32;

namespace Async
{
	using namespace Boring32::Async;
	using LockParams = FileRangeLock::LockParams;
	namespace Win32 = Boring32::Win32;

	constexpr static std::string_view FileNameA = "new.txt";
	constexpr static std::wstring_view FileNameW = L"new.txt";

	auto CreateOrOpenLockFile() -> Boring32::RAII::HandleUniquePtr
	{
		if (not std::filesystem::exists(FileNameW))
		{
			std::ofstream ofs(FileNameA.data());
			if (not ofs.good())
				throw Boring32::Error::Boring32Error("Failed to create lock file.");
		}
		Win32::HANDLE file = Win32::CreateFileW(
			FileNameW.data(),
			Win32::GenericRead | Win32::GenericWrite,
			Win32::FileShareMode::Read | Win32::FileShareMode::Write,
			nullptr,
			static_cast<Win32::DWORD>(Win32::CreateFileDisposition::OpenAlways),
			Win32::FileAttributes::Normal | Win32::FileFlags::DeleteOnClose,
			nullptr
		);
		if (not file or file == Win32::InvalidHandleValue)
		{
			const auto lastError = Win32::GetLastError();
			throw Boring32::Error::Win32Error(lastError, "Failed to create lock file handle.");
		}
		return Boring32::RAII::HandleUniquePtr(file);
	}

	TEST_CLASS(FileRangeLockTests)
	{
		// https://learn.microsoft.com/en-us/visualstudio/test/microsoft-visualstudio-testtools-cppunittestframework-api-reference?view=visualstudio

		TEST_METHOD(TestDefaultConstructor)
		{
			Async::FileRangeLock fileLock;
		}

		TEST_METHOD(TestLock)
		{
			auto file = CreateOrOpenLockFile();
			Async::FileRangeLock fileLock(file.get(), LockParams{true});
		}

		TEST_METHOD(TestMoveAssignment)
		{
			auto file = CreateOrOpenLockFile();
			Async::FileRangeLock fileLock1(file.get(), LockParams{ true });
			Async::FileRangeLock fileLock2 = std::move(fileLock1);
			Assert::IsFalse(fileLock1.HandleIsValid());
			Assert::IsTrue(fileLock2.HandleIsValid());
		}

		TEST_METHOD(TestMoveConstructor)
		{
			auto file = CreateOrOpenLockFile();
			Async::FileRangeLock fileLock1(file.get(), LockParams{ true });
			Async::FileRangeLock fileLock2{ std::move(fileLock1) };
			Assert::IsFalse(fileLock1.HandleIsValid());
			Assert::IsTrue(fileLock2.HandleIsValid());
		}

		TEST_METHOD(TestLockActuallyLocks)
		{
			auto file = CreateOrOpenLockFile();
			Async::FileRangeLock fileLock(file.get(), LockParams{ true });
			try
			{
				std::filesystem::remove(FileNameA);
				Assert::IsTrue(false);
			}
			catch (...)
			{
				Assert::IsTrue(true);
			}
		}

		TEST_METHOD(TestLockUnlock)
		{
			auto file = CreateOrOpenLockFile();
			Async::FileRangeLock fileLock(file.get(), LockParams{ false });
			fileLock.lock();
			fileLock.unlock();
			// unlocking should not generate an error if the file is not locked
			fileLock.unlock();
		}

		TEST_METHOD(TestTryLock)
		{
			auto file = CreateOrOpenLockFile();
			Async::FileRangeLock fileLock1(file.get(), LockParams{ true });
			Async::FileRangeLock fileLock2(file.get(), LockParams{ false });
			Assert::IsFalse(fileLock2.try_lock());
		}
	};
}
