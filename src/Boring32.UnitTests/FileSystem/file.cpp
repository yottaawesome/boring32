#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace FileSystem
{
	TEST_CLASS(File)
	{
		TEST_METHOD(TestConstructor)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
		}

		TEST_METHOD(TestGetHandle)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto handle = file.GetHandle();
			Assert::IsTrue(handle != nullptr);
		}

		TEST_METHOD(TestGetFileType)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto type = file.GetType();
			Assert::IsTrue(type == Boring32::Win32::FileType::Disk);
		}

		TEST_METHOD(TestGetSize)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto size = file.GetSize();
			Assert::IsTrue(size == 0);
		}

		TEST_METHOD(TestClose)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			file.Close();
			Assert::IsTrue(file.GetHandle() == nullptr);
		}

		TEST_METHOD(TestGetFilePath)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto path = file.GetFilePath();
			Assert::IsTrue(path.ends_with(L"testfile.txt"));
		}

		TEST_METHOD(TestReadWriteClearFile)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt",  Boring32::Win32::GenericWrite | Boring32::Win32::GenericRead };
			auto data = std::vector<std::byte>{ static_cast<std::byte>('H'), static_cast<std::byte>('e'), static_cast<std::byte>('l'), static_cast<std::byte>('l'), static_cast<std::byte>('o') };
			file.WriteFile(data);
			file.Flush();

			file.SetFilePointer(0, Boring32::Win32::FilePointerMoveMethod::Begin);
			auto size = file.GetSize();
			auto readData = file.ReadFile(size);

			auto string = readData | std::ranges::views::transform([](std::byte b) { return static_cast<char>(b); }) | std::ranges::to<std::string>();

			file.Clear();
			Assert::IsTrue(string == "Hello");
			Assert::IsTrue(size == data.size());
			Assert::IsTrue(file.GetSize() == 0);
		}
	};
}